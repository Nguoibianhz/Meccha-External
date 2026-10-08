#include "InternalBridge.hpp"

#include <cstring>

bool InternalBridge::attach(Memory& mem, const wchar_t* dllPath) {
	detach(mem);

	if (!mem.processHandle || !dllPath || !dllPath[0]) return false;

	memory = &mem;
	processHandle = mem.processHandle;
	processId = mem.processId;

	if (!injector.inject(processHandle, dllPath)) return false;

	for (int attempt = 0; attempt < 50; ++attempt) {
		dllBase = injector.getModuleBase(processId, kDllName);
		if (dllBase) break;

		Sleep(20);
	}

	if (!dllBase) return false;

	sharedPtrRemote = injector.getExportAddress(processId, dllBase, kSharedPtrExport);
	pumpRemote = injector.getExportAddress(processId, dllBase, kPumpExport);
	if (!sharedPtrRemote || !pumpRemote) return false;

	if (!resolveRemoteShared()) return false;

	attached = true;
	return isReady();
}

void InternalBridge::detach(Memory& mem) {
	(void)mem;

	if (attached && processHandle) injector.eject(processHandle, processId, kDllName);

	attached = false;
	memory = nullptr;
	processHandle = nullptr;
	processId = 0;
	dllBase = 0;
	sharedRemote = 0;
	sharedPtrRemote = 0;
	pumpRemote = 0;
	sharedCache = {};
}

bool InternalBridge::isReady() const {
	return attached && sharedRemote != 0 && sharedCache.ready != 0 && sharedCache.magic == MECCHA_HOOK_MAGIC;
}

bool InternalBridge::resolveRemoteShared() {
	if (!memory) return false;

	for (int attempt = 0; attempt < 100; ++attempt) {
		const uintptr_t ptr = memory->readMemory<uintptr_t>(sharedPtrRemote);
		if (ptr) {
			sharedRemote = ptr;
			return readRemoteShared();
		}
		Sleep(10);
	}
	return false;
}

bool InternalBridge::readRemoteShared() {
	if (!memory || !sharedRemote) return false;

	return memory->readRawMemory(sharedRemote, &sharedCache, sizeof(MecchaSharedBlock));
}

bool InternalBridge::writeShared(const MecchaSharedBlock& block) {
	if (!memory || !sharedRemote) return false;

	sharedCache = block;
	return memory->writeMemory(sharedRemote, block);
}

bool InternalBridge::writeRemoteField(uintptr_t offset, const void* data, size_t size) {
	if (!processHandle || !sharedRemote || !data || size == 0) return false;

	return WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(sharedRemote + offset), data, size, nullptr);
}

bool InternalBridge::refreshShared() {
	return readRemoteShared();
}

bool InternalBridge::enqueueCommand(const MecchaCommand& command) {
	if (!memory || !sharedRemote) return false;

	if (!readRemoteShared()) return false;

	const uint32_t writeIndex = sharedCache.commandWrite % MECCHA_COMMAND_QUEUE_SIZE;
	sharedCache.commands[writeIndex] = command;
	sharedCache.commandWrite++;

	if (!writeRemoteField(offsetof(MecchaSharedBlock, commands) + writeIndex * sizeof(MecchaCommand), &command, sizeof(MecchaCommand))) return false;

	const uint32_t commandWrite = sharedCache.commandWrite;
	return writeRemoteField(offsetof(MecchaSharedBlock, commandWrite), &commandWrite, sizeof(commandWrite));
}

bool InternalBridge::pumpCommands() {
	if (!pumpRemote) return false;

	return injector.remoteCall(processHandle, pumpRemote, 0);
}

bool InternalBridge::createHook(uint32_t slotId, uintptr_t target, MecchaHookSignature signature) {
	MecchaCommand command{};
	command.op = MECCHA_CMD_CREATE_HOOK;
	command.slotId = slotId;
	command.arg0 = target;
	command.arg1 = static_cast<uintptr_t>(signature);
	return enqueueCommand(command) && pumpCommands();
}

bool InternalBridge::enableHook(uint32_t slotId) {
	MecchaCommand command{};
	command.op = MECCHA_CMD_ENABLE;
	command.slotId = slotId;
	return enqueueCommand(command) && pumpCommands();
}

bool InternalBridge::disableHook(uint32_t slotId) {
	MecchaCommand command{};
	command.op = MECCHA_CMD_DISABLE;
	command.slotId = slotId;
	return enqueueCommand(command) && pumpCommands();
}

bool InternalBridge::removeHook(uint32_t slotId) {
	MecchaCommand command{};
	command.op = MECCHA_CMD_REMOVE_HOOK;
	command.slotId = slotId;
	return enqueueCommand(command) && pumpCommands();
}

bool InternalBridge::setInvokeNative(uintptr_t address) {
	if (!writeRemoteField(offsetof(MecchaSharedBlock, invokeNative), &address, sizeof(address))) return false;

	sharedCache.invokeNative = address;
	return true;
}

bool InternalBridge::writeSlotPayload(uint32_t slotId, const MecchaSlotPayload& payload) {
	if (slotId >= MECCHA_MAX_SLOTS || !sharedRemote) return false;

	const uintptr_t offset = offsetof(MecchaSharedBlock, slots) + slotId * sizeof(MecchaSlotConfig) + offsetof(MecchaSlotConfig, payload);
	if (!writeRemoteField(offset, &payload, sizeof(payload))) return false;

	sharedCache.slots[slotId].payload = payload;
	return true;
}

bool InternalBridge::queueExecute(uint32_t slotId) {
	if (slotId >= MECCHA_MAX_SLOTS || !sharedRemote) return false;

	const uint32_t pending = 1;
	const uintptr_t offset = offsetof(MecchaSharedBlock, slots) + slotId * sizeof(MecchaSlotConfig) + offsetof(MecchaSlotConfig, executePending);
	if (!writeRemoteField(offset, &pending, sizeof(pending))) return false;

	sharedCache.slots[slotId].executePending = pending;
	return true;
}
