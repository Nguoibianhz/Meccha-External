#ifndef INTERNAL_BRIDGE_HPP
#define INTERNAL_BRIDGE_HPP

#include "../Memory/Memory.hpp"
#include "../Injector/Injector.hpp"
#include "../../../../InternalExploits/HookProtocol.hpp"

#include <cstdint>

class InternalBridge {
public:
	static constexpr wchar_t kDllName[] = L"InternalExploits.dll";
	static constexpr char kSharedPtrExport[] = "Meccha_SharedBlockPtr";
	static constexpr char kPumpExport[] = "Meccha_PumpCommands";

	bool attach(Memory& memory, const wchar_t* dllPath);
	void detach(Memory& memory);

	[[nodiscard]] bool isAttached() const { return attached; }
	[[nodiscard]] bool isReady() const;
	[[nodiscard]] uintptr_t sharedBlockAddress() const { return sharedRemote; }

	bool refreshShared();
	bool writeShared(const MecchaSharedBlock& block);
	bool writeSlotPayload(uint32_t slotId, const MecchaSlotPayload& payload);
	bool queueExecute(uint32_t slotId);
	bool setInvokeNative(uintptr_t address);

	bool enqueueCommand(const MecchaCommand& command);
	bool pumpCommands();

	bool createHook(uint32_t slotId, uintptr_t target, MecchaHookSignature signature);
	bool enableHook(uint32_t slotId);
	bool disableHook(uint32_t slotId);
	bool removeHook(uint32_t slotId);

	[[nodiscard]] const MecchaSharedBlock& cachedShared() const { return sharedCache; }

private:
	Injector injector{};
	MecchaSharedBlock sharedCache{};
	uintptr_t dllBase = 0;
	uintptr_t sharedRemote = 0;
	uintptr_t sharedPtrRemote = 0;
	uintptr_t pumpRemote = 0;
	uint32_t processId = 0;
	HANDLE processHandle = nullptr;
	bool attached = false;
	Memory* memory = nullptr;

	bool resolveRemoteShared();
	bool readRemoteShared();
	bool writeRemoteField(uintptr_t offset, const void* data, size_t size);
};

#endif // INTERNAL_BRIDGE_HPP
