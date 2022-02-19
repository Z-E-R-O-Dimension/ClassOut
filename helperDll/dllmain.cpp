// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "../include/Messager.h"

struct injectThreadMessage {
    bool exec, succ, isDpiAware;
};
BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    static msgChannel channel(L"ClassOutHelper");
    static injectThreadMessage msg{ 0 };
    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        channel.open();
        msg.exec = msg.succ = true;
        msg.isDpiAware = IsProcessDPIAware();
        channel.send(&msg, sizeof(msg));
        channel.close();
        if(msg.isDpiAware) {
            MessageBoxW(NULL, L"dpi aware", L"tip", 0);
        }
        else {
            MessageBoxW(NULL, L"dpi unaware", L"tip", 0);
        }
        break;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

