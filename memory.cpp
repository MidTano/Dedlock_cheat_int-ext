#include "memory.h"
#include <windows.h>
#include <iostream>

namespace memory {

    uintptr_t baseAddress = 0;
    size_t moduleSize = 0;
    HANDLE processHandle = nullptr;

    DWORD GetProcess(const wchar_t* Target) {
        HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshotHandle == INVALID_HANDLE_VALUE) {
            return NULL;
        }

        PROCESSENTRY32W processEntry = { };
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshotHandle, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, Target) == 0) {
                    CloseHandle(snapshotHandle);
                    return processEntry.th32ProcessID;
                }
            } while (Process32NextW(snapshotHandle, &processEntry));
        }

        CloseHandle(snapshotHandle);
        return NULL;
    }

    bool GetModuleInfo(DWORD processId, const wchar_t* moduleName) {
        HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

        if (snapshotHandle == INVALID_HANDLE_VALUE) {
            return false;
        }

        MODULEENTRY32W moduleEntry = { };
        moduleEntry.dwSize = sizeof(MODULEENTRY32W);

        if (Module32FirstW(snapshotHandle, &moduleEntry)) {
            do {
                if (_wcsicmp(moduleEntry.szModule, moduleName) == 0) {
                    baseAddress = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                    moduleSize = static_cast<size_t>(moduleEntry.modBaseSize);
                    CloseHandle(snapshotHandle);
                    return true;
                }
            } while (Module32NextW(snapshotHandle, &moduleEntry));
        }

        CloseHandle(snapshotHandle);
        return false;
    }

    void parsePattern(const char* patternString, std::vector<BYTE>& patternBytes, std::string& mask) {
        std::istringstream iss(patternString);
        std::string s;
        while (iss >> s) {
            if (s == "?") {
                patternBytes.push_back(0x00);
                mask += '?';
            }
            else {
                patternBytes.push_back(static_cast<BYTE>(strtoul(s.c_str(), nullptr, 16)));
                mask += 'x';
            }
        }
    }

    uintptr_t patternScan(const std::vector<BYTE>& patternBytes, const std::string& mask) {
        BYTE* data = new BYTE[moduleSize];
        SIZE_T bytesRead;

        if (!ReadProcessMemory(processHandle, (LPCVOID)baseAddress, data, moduleSize, &bytesRead)) {
            delete[] data;
            return NULL;
        }

        for (size_t i = 0; i <= bytesRead - patternBytes.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < patternBytes.size(); j++) {
                if (mask[j] != '?' && data[i + j] != patternBytes[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                delete[] data;
                return baseAddress + i;
            }
        }

        delete[] data;
        return NULL;
    }

    // Новая функция записи в память с защитой
    bool memSafeWrite(uintptr_t address, const void* buffer, SIZE_T size) {
        // Изменение защиты памяти
        DWORD oldProtect;
        if (!VirtualProtectEx(processHandle, (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            std::cout << "Failed to change memory protection!" << std::endl;
            return false;
        }

        // Запись в память
        SIZE_T written;
        if (!WriteProcessMemory(processHandle, (LPVOID)address, buffer, size, &written) || written != size) {
            std::cout << "Failed to write to memory!" << std::endl;
            VirtualProtectEx(processHandle, (LPVOID)address, size, oldProtect, &oldProtect); // Восстановление защиты
            return false;
        }

        // Восстановление старой защиты памяти
        if (!VirtualProtectEx(processHandle, (LPVOID)address, size, oldProtect, &oldProtect)) {
            std::cout << "Failed to restore memory protection!" << std::endl;
            return false;
        }

        return true;
    }

}
