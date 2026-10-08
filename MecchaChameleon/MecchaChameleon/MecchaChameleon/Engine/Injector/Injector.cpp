#include "Injector.hpp"

#include <TlHelp32.h>
#include <cstring>
#include <vector>

bool Injector::inject(HANDLE processHandle, const wchar_t* dllPath) {
	if (!processHandle || !dllPath || !dllPath[0]) return false;

	const size_t pathBytes = (wcslen(dllPath) + 1) * sizeof(wchar_t);
	void* remotePath = VirtualAllocEx(processHandle, nullptr, pathBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!remotePath) return false;

	if (!WriteProcessMemory(processHandle, remotePath, dllPath, pathBytes, nullptr)) {
		VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
		return false;
	}

	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (!kernel32) return false;

	auto loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(kernel32, "LoadLibraryW"));
	if (!loadLibrary) {
		VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
		return false;
	}

	HANDLE thread = CreateRemoteThread(processHandle, nullptr, 0, loadLibrary, remotePath, 0, nullptr);
	if (!thread) {
		VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
		return false;
	}

	WaitForSingleObject(thread, 5000);
	CloseHandle(thread);
	VirtualFreeEx(processHandle, remotePath, 0, MEM_RELEASE);
	return true;
}

bool Injector::eject(HANDLE processHandle, uint32_t processId, const wchar_t* moduleName) {
	if (!processHandle || !moduleName || !moduleName[0]) return false;

	const uintptr_t moduleBase = getModuleBase(processId, moduleName);
	if (!moduleBase) return false;

	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (kernel32 == 0) return false;

	auto freeLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(kernel32, "FreeLibrary"));
	if (!freeLibrary) return false;

	HANDLE thread = CreateRemoteThread(processHandle, nullptr, 0, freeLibrary, reinterpret_cast<LPVOID>(moduleBase), 0, nullptr);

	if (!thread) return false;

	WaitForSingleObject(thread, 5000);
	CloseHandle(thread);
	return true;
}

uintptr_t Injector::getModuleBase(uint32_t processId, const wchar_t* moduleName) const {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
	if (snapshot == INVALID_HANDLE_VALUE) return 0;

	MODULEENTRY32W entry{};
	entry.dwSize = sizeof(entry);

	uintptr_t base = 0;
	if (Module32FirstW(snapshot, &entry)) {
		do {
			if (_wcsicmp(entry.szModule, moduleName) == 0) {
				base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
				break;
			}
		} while (Module32NextW(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return base;
}

uintptr_t Injector::getExportAddress(uint32_t processId, uintptr_t moduleBase, const char* exportName) const {
	if (!processId || !moduleBase || !exportName) return 0;

	HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
	if (!process) return 0;

	IMAGE_DOS_HEADER dos{};
	if (!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(moduleBase), &dos, sizeof(dos), nullptr) || dos.e_magic != IMAGE_DOS_SIGNATURE) {
		CloseHandle(process);
		return 0;
	}

	IMAGE_NT_HEADERS64 nt{};
	if (!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(moduleBase + dos.e_lfanew), &nt, sizeof(nt), nullptr) ||
		nt.Signature != IMAGE_NT_SIGNATURE) {
		CloseHandle(process);
		return 0;
	}

	const auto& exportDirEntry = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (!exportDirEntry.VirtualAddress || !exportDirEntry.Size) {
		CloseHandle(process);
		return 0;
	}

	IMAGE_EXPORT_DIRECTORY exports{};
	if (!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(moduleBase + exportDirEntry.VirtualAddress), &exports, sizeof(exports), nullptr)) {
		CloseHandle(process);
		return 0;
	}

	const DWORD count = exports.NumberOfNames;
	std::vector<DWORD> nameRvas(count);
	std::vector<WORD> ordinals(count);
	std::vector<DWORD> functionRvas(exports.NumberOfFunctions);

	if (!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(moduleBase + exports.AddressOfNames), nameRvas.data(), count * sizeof(DWORD), nullptr) || !ReadProcessMemory(process, reinterpret_cast<LPCVOID>(moduleBase + exports.AddressOfNameOrdinals), ordinals.data(), count * sizeof(WORD), nullptr) || !ReadProcessMemory(process, reinterpret_cast<LPCVOID>(moduleBase + exports.AddressOfFunctions), functionRvas.data(), functionRvas.size() * sizeof(DWORD), nullptr)) {
		CloseHandle(process);
		return 0;
	}

	uintptr_t result = 0;
	for (DWORD i = 0; i < count; ++i) {
		char nameBuffer[256]{};
		if (!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(moduleBase + nameRvas[i]), nameBuffer, sizeof(nameBuffer) - 1, nullptr)) continue;

		if (strcmp(nameBuffer, exportName) != 0) continue;

		const WORD ordinal = ordinals[i];
		if (ordinal >= functionRvas.size()) break;

		result = moduleBase + functionRvas[ordinal];
		break;
	}

	CloseHandle(process);
	return result;
}

bool Injector::remoteCall(HANDLE processHandle, uintptr_t functionAddress, uintptr_t parameter, uint32_t timeoutMs) const {
	if (!processHandle || !functionAddress) return false;

	HANDLE thread = CreateRemoteThread(processHandle, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(functionAddress), reinterpret_cast<LPVOID>(parameter), 0, nullptr);

	if (!thread) return false;

	const DWORD waitResult = WaitForSingleObject(thread, timeoutMs);
	CloseHandle(thread);
	return waitResult == WAIT_OBJECT_0;
}
