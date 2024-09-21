#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "MinHook/MinHook.h"
#if _WIN64 
#pragma comment(lib, "MinHook/libMinHook.x64.lib")
#else
#pragma comment(lib, "MinHook/libMinHook.x86.lib")
#endif

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include <d3d11.h>
#include "visuals.h"  
#include "aimbot.h"
#pragma comment(lib, "d3d11.lib")
#include "beepbob.h"
#include "movment.h" 


#include "memory.h"
#include <iostream>
#include "sniper.h" 
#include "freecam.h"

HINSTANCE dll_handle;
typedef long(__stdcall* present)(IDXGISwapChain*, UINT, UINT);
present p_present;
present p_present_target;

uint8_t local_team = 0;
bool aimbot_active = false;

bool get_present_pointer() {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = GetForegroundWindow();
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* swap_chain;
    ID3D11Device* device;

    const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        feature_levels,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &swap_chain,
        &device,
        nullptr,
        nullptr) == S_OK) {
        void** p_vtable = *reinterpret_cast<void***>(swap_chain);
        swap_chain->Release();
        device->Release();
        p_present_target = (present)p_vtable[8];
        return true;
    }
    return false;
}

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HWND window = NULL;
ID3D11Device* p_device = NULL;
ID3D11DeviceContext* p_context = NULL;
ID3D11RenderTargetView* mainRenderTargetView = NULL;

void RenderMenu() {
    ImGui::Begin("LLI_K_O_JI_b_H_U_K");
    if (ImGui::BeginTabBar("##tabs")) {
   
        if (ImGui::BeginTabItem("Render")) {
            Visuals1::RenderSettingsMenu();
            ImGui::EndTabItem();
        }

       
        if (ImGui::BeginTabItem("Aimbot")) {
            Aimbot::RenderAimbotSettingsMenu();
            BebopHook::RenderBebopSettingsMenu();
            SniperAutoAim::RenderSniperSettingsMenu();
            ImGui::EndTabItem();
        }

     
        if (ImGui::BeginTabItem("Movement")) {
            render_movement_settings();  
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Extra")) {
            freecam::RenderFreeCamMenu();
            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }
    ImGui::End();
}


static long __stdcall detour_present(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags) {
    if (!init) {
        if (SUCCEEDED(p_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&p_device))) {
            p_device->GetImmediateContext(&p_context);
            DXGI_SWAP_CHAIN_DESC sd;
            p_swap_chain->GetDesc(&sd);
            window = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            p_device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
            ImGui_ImplWin32_Init(window);
            ImGui_ImplDX11_Init(p_device, p_context);
            init = true;
        }
        else
            return p_present(p_swap_chain, sync_interval, flags);
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    RenderMenu(); 
    process_movement_logic(); 


    uintptr_t local_entity = find_local_player(local_team);
    ViewMatrix vm = get_view_matrix();  


    uintptr_t cammanagerAddress = memory::memRead<uintptr_t>(memory::baseAddress + (offsets::CCitadelCameraManager + 0x28));
    Vector3 cammanager_pos = memory::memRead<Vector3>(cammanagerAddress+0x38);



    ViewMatrix view_matrix = memory::memRead<ViewMatrix>(memory::baseAddress + offsets::dwViewMatrix);


    /*
    // Вывод ViewMatrix в ImGui
    ImGui::Text("ViewMatrix:");
    for (int i = 0; i < 4; ++i) {
        ImGui::Text("[%.2f, %.2f, %.2f, %.2f]",
            view_matrix.matrix[i][0],
            view_matrix.matrix[i][1],
            view_matrix.matrix[i][2],
            view_matrix.matrix[i][3]);
    }
    */

    uintptr_t local_pawn = memory::memRead<uintptr_t>(memory::baseAddress + 0x1F44280);
    /*
    char address_str[32];
    sprintf_s(address_str, sizeof(address_str), "0x%08llX", local_pawn);
    ImGui::Text("Local Pawn Address: %s", address_str);


    ULONG_PTR entity_list = get_entity_list();
    int max_entities = get_max_entities();

    for (int i = 1; i <= max_entities; ++i) {
        uintptr_t entity = get_base_entity_from_index(i, entity_list);
        if (!entity) continue;

        uint32_t health = memory::memRead<uint32_t>(entity + 0x34c);
        Vector3 entity_pos = memory::memRead<Vector3>(entity + offsets::m_vOldOrigin);


        char address_str[32];
        sprintf_s(address_str, sizeof(address_str), "0x%08llX", entity);


        std::string designer_name = get_designer_name(entity);



        ImGui::Text("%s: %s | Position: X: %.2f, Y: %.2f, Z: %.2f | Health: %d",
            designer_name.c_str(), address_str,
            entity_pos.X, entity_pos.Y, entity_pos.Z, health);

    }
      */

    Vector3 local_player_pos = memory::memRead<Vector3>(local_pawn + offsets::m_vOldOrigin);


    if (Visuals1::esp_box_enabled || Visuals1::esp_line_enabled || Visuals1::esp_health_enabled) {
        Visuals1::PlayerEsp(local_team);
    }

    freecam::NavigateEnemies(local_team);
    freecam::UpdateCameraPosition();
    

    if (Aimbot::settings.enabled) {  
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
            Aimbot::settings.active = true;
        }
        else {
            Aimbot::settings.active = false;
            Aimbot::settings.targetLocked = false;
        }
        Aimbot::AimbotLogic(local_entity, local_team, cammanager_pos, vm);  
        Aimbot::DrawFOVCircle(Aimbot::settings.fov);
    }

    if (BebopHook::hook_active) {
        BebopHook::BebopAutoHookLogic(local_entity, local_team, cammanager_pos, vm);
    }

    if (SniperAutoAim::aim_active) {
        SniperAutoAim::SniperAutoAimLogic(local_entity, local_team, cammanager_pos, vm);  
    }

    ImGui::EndFrame();
    ImGui::Render();

    p_context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return p_present(p_swap_chain, sync_interval, flags);
}

DWORD __stdcall EjectThread(LPVOID lpParameter) {
    Sleep(100);
    FreeLibraryAndExitThread(dll_handle, 0);
    Sleep(100);
    return 0;
}

int WINAPI main() {

    const wchar_t* processName = L"project8.exe";
    const wchar_t* moduleName = L"client.dll";


    DWORD processId = memory::GetProcess(processName);
    if (!processId) {
        std::wcerr << L"Процесс не найден: " << processName << std::endl;
        return 1;
    }


    memory::processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    if (!memory::processHandle) {
        std::cerr << "Не удалось открыть процесс." << std::endl;
        return 1;
    }


    if (!memory::GetModuleInfo(processId, moduleName)) {
        std::wcerr << L"Модуль не найден: " << moduleName << std::endl;
        CloseHandle(memory::processHandle);
        return 1;
    }


    offsets::initializeOffsets();


    Visuals1::UpdateEntityCache();

    if (!get_present_pointer()) {
        return 1;
    }

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK) {
        return 1;
    }

    if (MH_CreateHook(reinterpret_cast<void**>(p_present_target), &detour_present, reinterpret_cast<void**>(&p_present)) != MH_OK) {
        return 1;
    }

    if (MH_EnableHook(p_present_target) != MH_OK) {
        return 1;
    }

    while (true) {
        Sleep(50);

        if (GetAsyncKeyState(VK_NUMPAD0) & 1) {

        }

        if (GetAsyncKeyState(VK_DELETE)) {
            break;
        }
    }

    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
        return 1;
    }
    if (MH_Uninitialize() != MH_OK) {
        return 1;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = NULL; }
    if (p_context) { p_context->Release(); p_context = NULL; }
    if (p_device) { p_device->Release(); p_device = NULL; }
    SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));


    CloseHandle(memory::processHandle);

    CreateThread(0, 0, EjectThread, 0, 0, 0);

    return 0;
}

BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        dll_handle = hModule;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, NULL, 0, NULL);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {

    }
    return TRUE;
}
