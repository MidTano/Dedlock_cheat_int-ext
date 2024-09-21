#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "offsets.h"

#define WIN32_LEAN_AND_MEAN

namespace memory {

    extern uintptr_t baseAddress;
    extern size_t moduleSize;
    extern HANDLE processHandle;

    DWORD GetProcess(const wchar_t* Target);

    bool GetModuleInfo(DWORD processId, const wchar_t* moduleName);

    void parsePattern(const char* patternString, std::vector<BYTE>& patternBytes, std::string& mask);

    uintptr_t patternScan(const std::vector<BYTE>& patternBytes, const std::string& mask);

    template <typename type>
    type memRead(uintptr_t address) {
        type value = { };
        ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(type), NULL);
        return value;
    }

    template <typename type>
    bool memWrite(uintptr_t address, type value) {
        return WriteProcessMemory(processHandle, (LPVOID)address, &value, sizeof(type), NULL);
    }

    // Объявление новой функции безопасной записи в память
    bool memSafeWrite(uintptr_t address, const void* buffer, SIZE_T size);

}
