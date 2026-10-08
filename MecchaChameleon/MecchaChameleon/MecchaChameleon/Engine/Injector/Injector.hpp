#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#include <Windows.h>
#include <cstdint>

class Injector {
public:
	bool inject(HANDLE processHandle, const wchar_t* dllPath);
	bool eject(HANDLE processHandle, uint32_t processId, const wchar_t* moduleName);

	uintptr_t getModuleBase(uint32_t processId, const wchar_t* moduleName) const;
	uintptr_t getExportAddress(uint32_t processId, uintptr_t moduleBase, const char* exportName) const;

	bool remoteCall(HANDLE processHandle, uintptr_t functionAddress, uintptr_t parameter, uint32_t timeoutMs = 5000) const;
};

#endif // INJECTOR_HPP
