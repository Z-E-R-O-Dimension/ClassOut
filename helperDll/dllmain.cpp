// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <stdio.h>
#include <iostream>
#include <detours/detours.h>
#include <conio.h>
#include <assert.h>
#include <chrono>
#include <Shlwapi.h>
#include <thread>
#include "../include/win32Obj.h"
#include "../include/notifyStruct.h"
#include "../include/hookHelper.h"
#pragma comment(lib,"Version.lib")
#pragma comment(lib,"Shlwapi.lib")
using namespace std::chrono_literals;

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) {
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		std::cout << "closing...\n";
		FreeConsole();
		break;
	default:
		break;
	}
	return false;
}

#pragma region Qt_Declearations
#define Q_ASSERT assert
typedef int QBasicAtomicInt;
typedef double qreal;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef ptrdiff_t qptrdiff;
namespace QtPrivate {
	class RefCount
	{
	public:
		QBasicAtomicInt atomic;
	};
}
struct QArrayData
{
public:
	QtPrivate::RefCount ref;
	int size;
	uint alloc : 31;
	uint capacityReserved : 1;
	qptrdiff offset; // in bytes from beginning of header
	void* data()
	{
		Q_ASSERT(size == 0
			|| offset < 0 || size_t(offset) >= sizeof(QArrayData));
		return reinterpret_cast<char*>(this) + offset;
	}
	const void* data() const
	{
		Q_ASSERT(size == 0
			|| offset < 0 || size_t(offset) >= sizeof(QArrayData));
		return reinterpret_cast<const char*>(this) + offset;
	}

};
template <class T>
struct QTypedArrayData
	: QArrayData {
public:

};
class QRect {
public:
	int x1;
	int y1;
	int x2;
	int y2;
};
class QRectF {
public:
	qreal xp;
	qreal yp;
	qreal w;
	qreal h;
};
class QPoint {
public:
	int xp;
	int yp;
};
class QPointF {
public:
	qreal xp;
	qreal yp;
};
class QPainter {
public:
	__declspec(dllimport)
		void drawText(class QPointF const&, class QString const&, int, int);
};
class QBasicAtomicInt0 {
public:
	volatile int _q_value;
};
class QString {
public:
	int size() const { return d->size; }
	typedef QTypedArrayData<ushort> Data;
	Data* d;
};
class QByteArray {
public:
	int size() const { return d->size; }
	typedef QTypedArrayData<char> Data;
	Data* d;
};
#pragma endregion
HMODULE hQt5GuiDll = NULL, hQt5CoreDll = NULL, hCommEngineDll = NULL;

// help function
// Qt5Core.dll
// 7762 ?toWCharArray@QString@@QBEHPA_W@Z
// 7762 public: int __thiscall QString::toWCharArray(wchar_t *)const 
int(__fastcall* QString_toWCharArray)(const QString*, int, wchar_t*);
// detour function
// Qt5Gui.dll
// 3975 ?drawText@QPainter@@QAEXABVQPoint@@ABVQString@@@Z                
// 3976 ?drawText@QPainter@@QAEXABVQPointF@@ABVQString@@@Z               
// 3977 ?drawText@QPainter@@QAEXABVQPointF@@ABVQString@@HH@Z             
// 3978 ?drawText@QPainter@@QAEXABVQRect@@HABVQString@@PAV2@@Z           
// 3979 ?drawText@QPainter@@QAEXABVQRectF@@ABVQString@@ABVQTextOption@@@Z
// 3980 ?drawText@QPainter@@QAEXABVQRectF@@HABVQString@@PAV2@@Z          
// 3981 ?drawText@QPainter@@QAEXHHABVQString@@@Z                         
// 3982 ?drawText@QPainter@@QAEXHHHHHABVQString@@PAVQRect@@@Z            
/*
void __fastcall Detour_QPainter_drawText_21(void*, int, QPoint const&, class QString const&);
void __fastcall Detour_QPainter_drawText_22(void*, int, QPointF const&, class QString const&);
void __fastcall Detour_QPainter_drawText_41(void*, int, QPointF const&, class QString const&, int, int);
void __fastcall Detour_QPainter_drawText_42(void*, int, QRect const&, int, class QString const&, class QRect*);
void __fastcall Detour_QPainter_drawText_31(void*, int, QRectF const&, class QString const&, class QTextOption const&);
void __fastcall Detour_QPainter_drawText_43(void*, int, QRectF const&, int, class QString const&, class QRectF*);
void __fastcall Detour_QPainter_drawText_32(void*, int, int, int, class QString const&);
void __fastcall Detour_QPainter_drawText_71(void*, int, int, int, int, int, int, class QString const&, class QRect*);
decltype(Detour_QPainter_drawText_21)* Trampoline_QPainter_drawText_21 = nullptr;
decltype(Detour_QPainter_drawText_22)* Trampoline_QPainter_drawText_22 = nullptr;
decltype(Detour_QPainter_drawText_41)* Trampoline_QPainter_drawText_41 = nullptr;
decltype(Detour_QPainter_drawText_42)* Trampoline_QPainter_drawText_42 = nullptr;
decltype(Detour_QPainter_drawText_31)* Trampoline_QPainter_drawText_31 = nullptr;
decltype(Detour_QPainter_drawText_43)* Trampoline_QPainter_drawText_43 = nullptr;
decltype(Detour_QPainter_drawText_32)* Trampoline_QPainter_drawText_32 = nullptr;
decltype(Detour_QPainter_drawText_71)* Trampoline_QPainter_drawText_71 = nullptr;
*/
// CommEngine.dll
// 519 ?getOnStageShortOfNumber@CommandUserSettings@@QAEHXZ
// 519 public: int __thiscall CommandUserSettings::getOnStageShortOfNumber(void)
// 520 ?getOnStageUpperLimite@CommandClassroomState@@QAEHXZ
// 520 public: int __thiscall CommandClassroomState::getOnStageUpperLimite(void)
/*
int __fastcall Detour_CommandUserSettings_getOnStageShortOfNumber(void*, int);
int __fastcall Detour_CommandClassroomState_getOnStageUpperLimite(void*, int);
decltype(Detour_CommandUserSettings_getOnStageShortOfNumber)* Trampoline_CommandUserSettings_getOnStageShortOfNumber = nullptr;
decltype(Detour_CommandClassroomState_getOnStageUpperLimite)* Trampoline_CommandClassroomState_getOnStageUpperLimite = nullptr;
*/
// User32.dll
// 2313 328 0002B120 SetCursor
/*
HCURSOR __stdcall Detour_SetCursor(HCURSOR hCursor);
decltype(Detour_SetCursor)* Trampoline_SetCursor = nullptr;
*/

void drawText_impl(QString const& str) {
	wchar_t wstr[2048]{ 0 };
	int len = QString_toWCharArray(&str, 0, wstr);
	wprintf(L"[%d]%s\n", len, wstr);
	if (wcscmp(wstr, L"你下台了，暂时无法与大家互动") == 0) {
		//wcscpy(pNotify->string, wstr);
		//evtNotify.set();
	}
}

HH_FUNC_GLOBAL(HCURSOR, WINAPI, SetCursor, HCURSOR hCursor) {
	printf("in setcursor:%p\n", hCursor);
	return HH_TRAMPL_FUNC(SetCursor)(hCursor);
}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_21, void* pThis, int edx, QPoint const& p, class QString const& s) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_21)(pThis, edx, p, s);
}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_22, void* pThis, int edx, QPointF const& p, class QString const& s) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_22)(pThis, edx, p, s);
}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_41, void* pThis, int edx, QPointF const& p, class QString const& s, int x, int y) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_41)(pThis, edx, p, s, x, y);
}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_42, void* pThis, int edx, QRect const& r, int i, class QString const& s, class QRect* p) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_42)(pThis, edx, r, i, s, p);
}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_31, void* pThis, int edx, QRectF const& r, class QString const& s, class QTextOption const& t) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_31)(pThis, edx, r, s, t);
}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_43, void* pThis, int edx, QRectF const& r, int i, class QString const& s, class QRectF* f) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_43)(pThis, edx, r, i, s, f);

}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_32, void* pThis, int edx, int i, int n, class QString const& s) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_32)(pThis, edx, i, n, s);
}
HH_FUNC_DLL(void, __fastcall, QPainter_drawText_71, void* pThis, int edx, int i, int n, int t, int j, int l, class QString const& s, class QRect* p) {
	drawText_impl(s);
	HH_TRAMPL_FUNC(QPainter_drawText_71)(pThis, edx, i, n, t, j, l, s, p);
}
HH_FUNC_DLL(int, __fastcall, CommandUserSettings_getOnStageShortOfNumber, void* pThis, int edx) {
	int ret = HH_TRAMPL_FUNC(CommandUserSettings_getOnStageShortOfNumber)(pThis, edx);
	printf("in Detour_CommandUserSettings_getOnStageShortOfNumber: ret=%d", ret);
	return ret;
}
HH_FUNC_DLL(int, __fastcall, CommandClassroomState_getOnStageUpperLimite, void* pThis, int edx) {
	int ret = HH_TRAMPL_FUNC(CommandClassroomState_getOnStageUpperLimite)(pThis, edx);
	printf("in Detour_CommandClassroomState_getOnStageUpperLimite: ret=%d", ret);
	return ret;
}

HookInfo info[] = {
	HH_INFO_GLOBAL(SetCursor),
	HH_INFO_DLL(QPainter_drawText_21,hQt5GuiDll,"?drawText@QPainter@@QAEXABVQPoint@@ABVQString@@@Z"),
	HH_INFO_DLL(QPainter_drawText_22,hQt5GuiDll,"?drawText@QPainter@@QAEXABVQPointF@@ABVQString@@@Z"),
	HH_INFO_DLL(QPainter_drawText_41,hQt5GuiDll,"?drawText@QPainter@@QAEXABVQPointF@@ABVQString@@HH@Z"),
	HH_INFO_DLL(QPainter_drawText_42,hQt5GuiDll,"?drawText@QPainter@@QAEXABVQRect@@HABVQString@@PAV2@@Z"),
	HH_INFO_DLL(QPainter_drawText_31,hQt5GuiDll,"?drawText@QPainter@@QAEXABVQRectF@@ABVQString@@ABVQTextOption@@@Z"),
	HH_INFO_DLL(QPainter_drawText_43,hQt5GuiDll,"?drawText@QPainter@@QAEXABVQRectF@@HABVQString@@PAV2@@Z"),
	HH_INFO_DLL(QPainter_drawText_32,hQt5GuiDll,"?drawText@QPainter@@QAEXHHABVQString@@@Z"),
	HH_INFO_DLL(QPainter_drawText_71,hQt5GuiDll,"?drawText@QPainter@@QAEXHHHHHABVQString@@PAVQRect@@@Z"),

	HH_INFO_DLL(CommandUserSettings_getOnStageShortOfNumber,hCommEngineDll,"?getOnStageShortOfNumber@CommandUserSettings@@QAEHXZ"),
	HH_INFO_DLL(CommandClassroomState_getOnStageUpperLimite,hCommEngineDll,"?getOnStageUpperLimite@CommandClassroomState@@QAEHXZ"),

};

template<typename dst_type, typename src_type>
dst_type pointer_cast(src_type src) {
	return *static_cast<dst_type*>(static_cast<void*>(&src));
}
template<typename T>
void getProcAddr(HMODULE hModule, LPCSTR lpProcName, T& pFun) {
	pFun = pointer_cast<T>(GetProcAddress(hModule, lpProcName));
}

void printQString(const QString& qstr) {
	wchar_t wstr[2048]{ 0 };
	int ret = QString_toWCharArray(&qstr, 0, wstr);
	printf("[%d]%ls\n", ret, wstr);
}

void getVersion(LPCWSTR szVersionFile) {
	DWORD  verHandle = 0;
	UINT   size = 0;
	LPBYTE lpBuffer = NULL;
	DWORD  verSize = GetFileVersionInfoSizeW(szVersionFile, &verHandle);
	if (verSize != NULL)
	{
		LPSTR verData = new char[verSize];
		if (GetFileVersionInfoW(szVersionFile, verHandle, verSize, verData))
		{
			if (VerQueryValueW(verData, L"\\", (LPVOID*)&lpBuffer, &size))
			{
				if (size)
				{
					VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
					if (verInfo->dwSignature == 0xfeef04bd)
					{
						// Doesn't matter if you are on 32 bit or 64 bit,
						// DWORD is always 32 bits, so first two revision numbers
						// come from dwFileVersionMS, last two come from dwFileVersionLS
						printf("%ls Version: %d.%d.%d.%d\n",
							szVersionFile,
							(verInfo->dwFileVersionMS >> 16) & 0xffff,
							(verInfo->dwFileVersionMS >> 0) & 0xffff,
							(verInfo->dwFileVersionLS >> 16) & 0xffff,
							(verInfo->dwFileVersionLS >> 0) & 0xffff
						);
					}
				}
			}
		}
		delete[] verData;
	}
}

__declspec(dllexport)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	wchar_t moduleName[260]{ 0 };
	LPCWSTR fileName = NULL;
	LONG code = 0;
	switch (ul_reason_for_call)
	{
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_ATTACH:
		AllocConsole();
		SetConsoleCtrlHandler(HandlerRoutine, TRUE);
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		setlocale(LC_ALL, "");

		hQt5CoreDll = LoadLibraryW(L"Qt5Core.dll");
		hQt5GuiDll = LoadLibraryW(L"Qt5Gui.dll");
		hCommEngineDll = LoadLibraryW(L"CommEngine.dll");

		GetProcAddrMulti(info);
		if (code = DetourAttachAllInOne(info) != NO_ERROR) {
			printf("failed%d\n", code);
		}
		else {
			printf("succeed\n");
		}
		QString_toWCharArray = (decltype(QString_toWCharArray))GetProcAddress(hQt5CoreDll, "?toWCharArray@QString@@QBEHPA_W@Z");
		break;
	case DLL_PROCESS_DETACH:
		if (DetourDetachAllInOne(info) != NO_ERROR) {
			printf("failed\n");
		}
		else {
			printf("succeed\n");
		}
		FreeLibrary(hQt5CoreDll);
		FreeLibrary(hQt5GuiDll);
		FreeLibrary(hCommEngineDll);
		break;
	}
	return TRUE;
}
/*
HCURSOR __stdcall Detour_SetCursor(HCURSOR hCursor)
{
	HCURSOR ret;
	ret = Trampoline_SetCursor(hCursor);
	switch ((uintptr_t)hCursor) {
		//00010003 -> arrow
		//0001001F -> hand
		//00010005 -> ibeam
	case 0x00010003:
		PostMessageW(g_notify.classoutWnd, NOTIFY_CURSOR_CHANGED, (WPARAM)IDC_ARROW, NULL);
		break;
	case 0x0001001F:
		PostMessageW(g_notify.classoutWnd, NOTIFY_CURSOR_CHANGED, (WPARAM)IDC_HAND, NULL);
		break;
	case 0x00010005:
		PostMessageW(g_notify.classoutWnd, NOTIFY_CURSOR_CHANGED, (WPARAM)IDC_IBEAM, NULL);
		break;
	default:
		break;
	}
	printf("\n");
	return ret;
}
*/