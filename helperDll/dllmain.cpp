// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <stdio.h>
#include <iostream>
#include <detours/detours.h>
#include <conio.h>
#include <assert.h>
#include "../include/win32Obj.h"
#include "../include/notifyStruct.h"
#pragma comment(lib,"Version.lib")

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

#pragma region Detour_Functions
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

// CommEngine.dll
// 519 ?getOnStageShortOfNumber@CommandUserSettings@@QAEHXZ
// 519 public: int __thiscall CommandUserSettings::getOnStageShortOfNumber(void)
// 520 ?getOnStageUpperLimite@CommandClassroomState@@QAEHXZ
// 520 public: int __thiscall CommandClassroomState::getOnStageUpperLimite(void)
int __fastcall Detour_CommandUserSettings_getOnStageShortOfNumber(void*, int);
int __fastcall Detour_CommandClassroomState_getOnStageUpperLimite(void*, int);
decltype(Detour_CommandUserSettings_getOnStageShortOfNumber)* Trampoline_CommandUserSettings_getOnStageShortOfNumber = nullptr;
decltype(Detour_CommandClassroomState_getOnStageUpperLimite)* Trampoline_CommandClassroomState_getOnStageUpperLimite = nullptr;

#pragma endregion

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

HMODULE hQt5GuiDll = NULL, hQt5CoreDll = NULL, hCommEngineDll = NULL;
#define EXTRACTINFO(hModule,funName,decName) { &hModule, decName, &(PVOID&)Trampoline_##funName, (void*)Detour_##funName }
struct FuncInfo {
	HMODULE* hModule;
	LPCSTR decoratedName;
	PVOID* ppTrampoline;
	PVOID pDetour;
}info[] = {
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_21, "?drawText@QPainter@@QAEXABVQPoint@@ABVQString@@@Z"),
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_22, "?drawText@QPainter@@QAEXABVQPointF@@ABVQString@@@Z"),
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_41, "?drawText@QPainter@@QAEXABVQPointF@@ABVQString@@HH@Z"),
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_42, "?drawText@QPainter@@QAEXABVQRect@@HABVQString@@PAV2@@Z"),
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_31, "?drawText@QPainter@@QAEXABVQRectF@@ABVQString@@ABVQTextOption@@@Z"),
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_43, "?drawText@QPainter@@QAEXABVQRectF@@HABVQString@@PAV2@@Z"),
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_32, "?drawText@QPainter@@QAEXHHABVQString@@@Z"),
	EXTRACTINFO(hQt5GuiDll, QPainter_drawText_71, "?drawText@QPainter@@QAEXHHHHHABVQString@@PAVQRect@@@Z"),
	//EXTRACTINFO(hCommEngineDll, CommandUserSettings_getOnStageShortOfNumber, "?getOnStageShortOfNumber@CommandUserSettings@@QAEHXZ"),
	//EXTRACTINFO(hCommEngineDll, CommandClassroomState_getOnStageUpperLimite, "?getOnStageUpperLimite@CommandClassroomState@@QAEHXZ"),
};
void GetProcAddrMulti() {
	for (auto& obj : info) {
		getProcAddr(*obj.hModule, obj.decoratedName, *obj.ppTrampoline);
	}
}
void DetourAttachMulti() {
	for (auto& obj : info) {
		DetourAttach(obj.ppTrampoline, obj.pDetour);
	}
}
void DetourDetachMulti() {
	for (auto& obj : info) {
		DetourDetach(obj.ppTrampoline, obj.pDetour);
	}
}

// globals
win32Event evtNotify(L"classOutEvent");
win32SharedMemory smNotify(L"classOutSM");
notifyStruct* pNotify = NULL;

void drawText_impl(QString const& str) {
	wchar_t wstr[2048]{ 0 };
	int len = QString_toWCharArray(&str, 0, wstr);
	if (wcscmp(wstr, L"你下台了，暂时无法与大家互动") == 0) {
		wcscpy(pNotify->string, wstr);
		evtNotify.set();
	}
}
__declspec(dllexport)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call)
	{
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_ATTACH:
		/*
		AllocConsole();
		SetConsoleCtrlHandler(HandlerRoutine, TRUE);
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		setlocale(LC_ALL, "");
		*/
		if (!(evtNotify.create() && smNotify.create(sizeof(notifyStruct)))) {
			MessageBoxW(NULL, L"There is another instance running.", L"Error", MB_ICONERROR | MB_OK);
			ExitProcess(1);
		}
		hQt5GuiDll = LoadLibraryW(L"Qt5Gui.dll");
		hQt5CoreDll = LoadLibraryW(L"Qt5Core.dll");
		hCommEngineDll = LoadLibraryW(L"CommEngine.dll");
		getVersion(L"Qt5Gui.dll");
		getVersion(L"Qt5Core.dll");
		GetProcAddrMulti();
		getProcAddr(hQt5CoreDll, "?toWCharArray@QString@@QBEHPA_W@Z", QString_toWCharArray);
		DetourRestoreAfterWith();
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttachMulti();
		if (DetourTransactionCommit() != NO_ERROR) {
			printf("detourAttach failed\n");
		}
		break;
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetachMulti();
		if (DetourTransactionCommit() != NO_ERROR) {
			printf("detourDetach failed\n");
		}
		break;
	}
	return TRUE;
}

void __fastcall Detour_QPainter_drawText_21(void* pThis, int edx, QPoint const& p, class QString const& s) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_21(pThis, edx, p, s);
}
void __fastcall Detour_QPainter_drawText_22(void* pThis, int edx, QPointF const& p, class QString const& s) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_22(pThis, edx, p, s);
}
void __fastcall Detour_QPainter_drawText_41(void* pThis, int edx, QPointF const& p, class QString const& s, int x, int y) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_41(pThis, edx, p, s, x, y);
}
void __fastcall Detour_QPainter_drawText_42(void* pThis, int edx, QRect const& r, int i, class QString const& s, class QRect* p) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_42(pThis, edx, r, i, s, p);
}
void __fastcall Detour_QPainter_drawText_31(void* pThis, int edx, QRectF const& r, class QString const& s, class QTextOption const& t) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_31(pThis, edx, r, s, t);
}
void __fastcall Detour_QPainter_drawText_43(void* pThis, int edx, QRectF const& r, int i, class QString const& s, class QRectF* f) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_43(pThis, edx, r, i, s, f);
}
void __fastcall Detour_QPainter_drawText_32(void* pThis, int edx, int i, int n, class QString const& s) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_32(pThis, edx, i, n, s);
}
void __fastcall Detour_QPainter_drawText_71(void* pThis, int edx, int i, int n, int t, int j, int l, class QString const& s, class QRect* p) {
	drawText_impl(s);
	Trampoline_QPainter_drawText_71(pThis, edx, i, n, t, j, l, s, p);
}
int __fastcall Detour_CommandUserSettings_getOnStageShortOfNumber(void* pthis, int edx) {
	int ret = Trampoline_CommandUserSettings_getOnStageShortOfNumber(pthis, edx);
	printf("in Detour_CommandUserSettings_getOnStageShortOfNumber: ret=%d", ret);
	return ret;
}
int __fastcall Detour_CommandClassroomState_getOnStageUpperLimite(void* pthis, int edx) {
	int ret = Trampoline_CommandClassroomState_getOnStageUpperLimite(pthis, edx);
	printf("in Detour_CommandClassroomState_getOnStageUpperLimite: ret=%d", ret);
	return ret;
}
