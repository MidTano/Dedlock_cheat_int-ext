#include "offsets.h"
#include "memory.h" 
#include <vector>
#include <string>
#include <iostream>

namespace offsets
{

    std::ptrdiff_t dwEntityList = 0;
    std::ptrdiff_t dwLocalPlayer = 0;
    std::ptrdiff_t CCitadelCameraManager = 0;
    std::ptrdiff_t dwViewMatrix = 0;

    void initializeOffsets()
    {


        // **dwEntityList**
        {
            const char* patternString = "48 8B 0D ? ? ? ? 8B C5 48 C1 E8";
            std::vector<BYTE> patternBytes;
            std::string mask;
            memory::parsePattern(patternString, patternBytes, mask);

            uintptr_t patternAddress = memory::patternScan(patternBytes, mask);

            if (patternAddress == NULL)
            {
                std::cerr << "Не удалось найти паттерн для dwEntityList." << std::endl;
            }
            else
            {
           
                uintptr_t nextInstruction = patternAddress + 7;
                int relativeOffset = memory::memRead<int>(patternAddress + 3);
                dwEntityList = nextInstruction + relativeOffset - memory::baseAddress; 
            }
        }

        // **dwLocalPlayer**
        {
            const char* patternString = "4C 8B 05 ? ? ? ? 8B D3 48 8B CD";
            std::vector<BYTE> patternBytes;
            std::string mask;
            memory::parsePattern(patternString, patternBytes, mask);

            uintptr_t patternAddress = memory::patternScan(patternBytes, mask);

            if (patternAddress == NULL)
            {
                std::cerr << "Не удалось найти паттерн для dwLocalPlayer." << std::endl;
            }
            else
            {
                uintptr_t nextInstruction = patternAddress + 7;
                int relativeOffset = memory::memRead<int>(patternAddress + 3);
                dwLocalPlayer = nextInstruction + relativeOffset - memory::baseAddress;
            }
        }

        // **CCitadelCameraManager**
        {
            const char* patternString = "48 8D 3D ? ? ? ? 8B D9";
            std::vector<BYTE> patternBytes;
            std::string mask;
            memory::parsePattern(patternString, patternBytes, mask);

            uintptr_t patternAddress = memory::patternScan(patternBytes, mask);

            if (patternAddress == NULL)
            {
                std::cerr << "Не удалось найти паттерн для CCitadelCameraManager." << std::endl;
            }
            else
            {
                uintptr_t nextInstruction = patternAddress + 7;
                int relativeOffset = memory::memRead<int>(patternAddress + 3);
                CCitadelCameraManager = nextInstruction + relativeOffset - memory::baseAddress;
            }
        }

        // **ViewMatrix**
        {
            const char* patternString = "48 8D 0D ? ? ? ? 48 C1 E0 06 48 03 C1 C3";
            std::vector<BYTE> patternBytes;
            std::string mask;
            memory::parsePattern(patternString, patternBytes, mask);

            uintptr_t patternAddress = memory::patternScan(patternBytes, mask);

            if (patternAddress == NULL)
            {
                std::cerr << "Не удалось найти паттерн для ViewMatrix." << std::endl;
            }
            else
            {
                uintptr_t nextInstruction = patternAddress + 7;
                int relativeOffset = memory::memRead<int>(patternAddress + 3);
                dwViewMatrix = nextInstruction + relativeOffset - memory::baseAddress;
            }
        }

    
    }
}
