#define _CRT_SECURE_NO_WARNINGS
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <tlhelp32.h>
#include <inttypes.h>
#include <exception>
#include <assert.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#include <detours/detours.h>
#include "resource.h"
#include "../include/win32Obj.h"
#include "../include/notifyStruct.h"
#pragma comment(lib,"Shlwapi.lib")

#define CheckReleaseDC(_hwnd,_hdc) do{ if((_hdc) != NULL) { ReleaseDC(_hwnd,_hdc); _hdc = NULL; } } while(0)
#define CheckDeleteDC(_hdc) do{ if((_hdc) != NULL) { DeleteDC(_hdc); _hdc = NULL; } } while(0)
#define CheckDeleteObject(_ho) do{ if((_ho) != NULL) { DeleteObject(_ho); _ho = NULL; } } while(0)

class RectWrapper :public RECT {
public:
	RectWrapper() :RECT{ 0,0,0,0 } {}
	LONG& x() {
		return this->left;
	}
	LONG& y() {
		return this->top;
	}
	LONG& x(LONG _x) {
		x() = _x;
		return this->left;
	}
	LONG& y(LONG _y) {
		y() = _y;
		return this->top;
	}
	LONG w() {
		return this->right - this->left;
	}
	LONG h() {
		return this->bottom - this->top;
	}
	void xy(LONG _x, LONG _y) {
		x(_x), y(_y);
	}
	void wh(LONG _cx, LONG _cy) {
		w(_cx), h(_cy);
	}
	void w(LONG _cx) {
		this->right = this->left + _cx;
	}
	void h(LONG _cy) {
		this->bottom = this->top + _cy;
	}
	POINT pto() {
		return POINT{ this->left,this->top };
	}
	POINT ptd() {
		return POINT{ this->right,this->bottom };
	}
};
class PointWrapper :public POINT {
public:
	PointWrapper() :POINT{ 0,0 } {}
	PointWrapper(LONG x, LONG y) :POINT{ x,y } {}
	void xy(LONG _x, LONG _y) { x = _x, y = _y; }
	POINT operator+(const POINT& other) {
		return POINT{ x + other.x,y + other.y };
	}
	POINT operator-(const POINT& other) {
		return POINT{ x - other.x,y - other.y };
	}
};
void trace(LPCWSTR format, ...) {
	TCHAR buffer[1000];
	va_list argptr;
	va_start(argptr, format);
	wvsprintfW(buffer, format, argptr);
	va_end(argptr);
	OutputDebugStringW(buffer);
}
class ClassOutApp {
public:
	ClassOutApp(HINSTANCE _hInstance) :hInstance(_hInstance) {
		if (!pInstance)
			pInstance = this;
		else
			throw std::runtime_error("multiple initializations");
		init();
	}
	~ClassOutApp() {
		detachClassroom();
	}
	int exec() {
		MSG msg;
		while (GetMessageW(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return (int)msg.wParam;
	}

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void init() {
		CoInitialize(NULL);
		systemScale = getZoom();
		SetProcessDPIAware();
		exportDll();
		evtNotify.reg(L"classOutEvent");
		smNotify.reg(L"classOutSM");

		// register window class
		WNDCLASSEXW wcex{ 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON1));
		wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszClassName = L"ClassOutMain";
		wcex.lpszMenuName = NULL;
		wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON1));
		RegisterClassExW(&wcex);

		// create menu
		viewWindow.hMenuRoot = CreateMenu();
		viewWindow.hMenuClassOut = CreatePopupMenu();
		viewWindow.hMenuClassIn = CreatePopupMenu();
		viewWindow.hMenuConnect = CreatePopupMenu();
		AppendMenuW(viewWindow.hMenuRoot, MF_POPUP, (UINT_PTR)viewWindow.hMenuClassOut, L"ClassOut");
		AppendMenuW(viewWindow.hMenuRoot, MF_POPUP, (UINT_PTR)viewWindow.hMenuClassIn, L"ClassIn");
		AppendMenuW(viewWindow.hMenuRoot, MF_POPUP, (UINT_PTR)viewWindow.hMenuConnect, L"连接");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu, L"刷新");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 1, L"隐藏");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 2, L"置底");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 3, L"关闭");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 4, L"专注学习模式");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 5, L"100%");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 6, L"125%");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 7, L"150%");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 8, L"175%");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 9, L"200%");
		AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 10, L"225%");
		AppendMenuW(viewWindow.hMenuClassOut, MF_STRING, idMenu + 100, L"置顶");
		AppendMenuW(viewWindow.hMenuConnect, MF_STRING, idMenu + 201, L"Start ClassIn With Dll");
		AppendMenuW(viewWindow.hMenuConnect, MF_STRING, idMenu + 202, L"Connect");
		// create window
		viewWindow.hWnd = CreateWindowW(
			L"ClassOutMain", L"ClassOut", WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
			CW_USEDEFAULT, 0, 800 * systemScale, 600 * systemScale, NULL, viewWindow.hMenuRoot, hInstance, NULL
		);

		ShowWindow(viewWindow.hWnd, SW_SHOW);
		UpdateWindow(viewWindow.hWnd);
	}

	LRESULT onCreate(HWND hWnd, LPCREATESTRUCTW pCS) {
		LoadLibraryW(L"user32.dll");
		// ctrl + F1
		RegisterHotKey(hWnd, idHotkey, MOD_CONTROL, VK_F1);

		if (GetLastError() == ERROR_HOTKEY_ALREADY_REGISTERED) {

		}

		// font msyh
		hFontMSYH32 = CreateFontW(
			32 * systemScale, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑"
		);
		// setup ui
		/*
		hButton[0] = CreateWindowExW(
			0, L"BUTTON", L"BUTTON1",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			93 * systemScale, 131 * systemScale, 186 * systemScale, 252 * systemScale, hWnd, NULL, hInstance, NULL
		);
		hButton[1] = CreateWindowExW(
			0, L"BUTTON", L"BUTTON2",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			285 * systemScale, 131 * systemScale, 186 * systemScale, 252 * systemScale, hWnd, NULL, hInstance, NULL
		);
		hButton[2] = CreateWindowExW(
			0, L"BUTTON", L"BUTTON3",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			477 * systemScale, 131 * systemScale, 186 * systemScale, 252 * systemScale, hWnd, NULL, hInstance, NULL
		);
		for (auto btn : hButton)
			SendMessageW(btn, WM_SETFONT, (WPARAM)hFontMSYH32, TRUE);
		*/
		//setup standard scroll bars
		updateScrolls();
		//refresh at 60Hz
		SetTimer(hWnd, idTimer, 1000 / 60, NULL);
		//viewWindow.hWndDC = GetDC(hWnd);
		//viewWindow.hMemDC = CreateCompatibleDC(viewWindow.hWndDC);
		return 0;
	}
	LRESULT onPaint(HWND hWnd) {
		//return DefWindowProcW(hWnd, WM_PAINT, 0, 0);
		PAINTSTRUCT ps{ 0 };
		HDC hdcPaint;
		hdcPaint = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		if (classinWindow.attached) {
			//ClassInWindow valid
			updateView();
		}
		else {
			RectWrapper rc;
			GetClientRect(hWnd, &rc);
			SelectObject(hdcPaint, hFontMSYH32);
			DrawTextW(hdcPaint, L"释放：等待ClassIn教室窗口", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		EndPaint(hWnd, &ps);
		return 0;
	}
	LRESULT onSizing(HWND hWnd, WPARAM edge, LPRECT pRect) {
		RectWrapper* prc = (RectWrapper*)pRect;
		RectWrapper rc;
		static int cx = GetSystemMetrics(SM_CXVSCROLL), cy = GetSystemMetrics(SM_CYHSCROLL);
		if (classinWindow.attached) {
			GetWindowRect(classinWindow.hWnd, &rc);
			if (rc.w() > 300 && rc.h() > 300) {
				limit(prc->right, 0, rc.w() / classinWindow.scale * viewWindow.viewScale + cx);
				limit(prc->bottom, 0, rc.h() / classinWindow.scale * viewWindow.viewScale + cy);
			}
			// rc->w() > classinWindow.w() * viewScale
		}
		return 0;
	}
	LRESULT onSize(HWND hWnd, UINT nWidth, UINT nHeight) {
		RectWrapper rc;
		//viewWindow.viewRect.wh(nWidth, nHeight);
		/*
		if(viewWindow.viewRect.x() > classinWindow.changedRect.w() - viewWindow.viewRect.w()) {
			viewWindow.viewRect.x() = classinWindow.changedRect.w() - viewWindow.viewRect.w();
		}
		if(viewWindow.viewRect.y() > classinWindow.changedRect.h() - viewWindow.viewRect.h()) {
			viewWindow.viewRect.y() = classinWindow.changedRect.h() - viewWindow.viewRect.h();
		}
		*/
		updateScrolls();
		updateView();
		//nViewPosX = 0, nViewPosY = 0;
		GetClientRect(hWnd, &rc);
		InvalidateRect(hWnd, &rc, false);
		return -1;
		return 0;
	}
	LRESULT onExitSizeMove(HWND hWnd) {
		RectWrapper rc;
		if (classinWindow.relevantSizing) {
			GetClientRect(hWnd, &rc);
			SetWindowPos(classinWindow.hWnd, NULL, 0, 0, rc.w(), rc.h(), SWP_NOMOVE | SWP_NOZORDER);
		}
		return 0;
	}
	LRESULT onHotKey(HWND hWnd, int id, UINT fsModifiers) {
		POINT pt;
		RectWrapper rc;
		if (id == idHotkey) {
			viewWindow.topmost = !viewWindow.topmost;
			if (viewWindow.topmost) {
				CheckMenuItem(viewWindow.hMenuClassOut, idMenu + 100, MF_BYCOMMAND | MF_CHECKED);
				GetCursorPos(&pt);
				GetWindowRect(hWnd, &rc);
				SetWindowPos(hWnd, HWND_TOPMOST, pt.x - rc.w() / 2, pt.y - 15, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
			}
			else {
				CheckMenuItem(viewWindow.hMenuClassOut, idMenu + 100, MF_BYCOMMAND | MF_UNCHECKED);
				SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
			}
		}
		return 0;
	}
	LRESULT onClose(HWND hWnd, WPARAM wParam, LPARAM lParam) {
		UnregisterHotKey(hWnd, idHotkey);
		return DefWindowProcW(hWnd, WM_CLOSE, wParam, lParam);
	}
	LRESULT onMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (classinWindow.attached) {
			PostMessageW(classinWindow.hWnd, message, wParam, MAKELPARAM(
				GET_X_LPARAM(lParam) * classinWindow.scale / viewWindow.viewScale + viewWindow.viewPos.x * classinWindow.scale,
				GET_Y_LPARAM(lParam) * classinWindow.scale / viewWindow.viewScale + viewWindow.viewPos.y * classinWindow.scale
			));
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	LRESULT onMouseWheel(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		RectWrapper viewRC, classinRC;
		if (classinWindow.attached) {
			GetWindowRect(hWnd, &viewRC);
			GetWindowRect(classinWindow.hWnd, &classinRC);
			return SendMessageW(classinWindow.hWnd, message, wParam, MAKELPARAM(
				(GET_X_LPARAM(lParam) - viewRC.x()) * classinWindow.scale / viewWindow.viewScale + viewWindow.viewPos.x * classinWindow.scale + classinRC.x(),
				(GET_Y_LPARAM(lParam) - viewRC.y()) * classinWindow.scale / viewWindow.viewScale + viewWindow.viewPos.y * classinWindow.scale + classinRC.y()
			));
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);

	}
	LRESULT onKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (classinWindow.attached)
			SendMessageW(classinWindow.hWnd, message, wParam, lParam);
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	LRESULT onClipboardMsg(HWND hWnd, UINT message) {
		if (classinWindow.attached)
			SendMessageW(classinWindow.hWnd, message, 0, 0);
		return DefWindowProcW(hWnd, message, 0, 0);
	}
	LRESULT onTimer(HWND hWnd, UINT_PTR nIDEvent, TIMERPROC lpTimerFunc) {
		switch (nIDEvent) {
		case idTimer:
			if (!classinWindow.attached)
				break;
			invalidate();
			break;
		default:
			break;
		}
		return 0;
	}
	LRESULT onHScroll(HWND hWnd, WORD request, WORD position, HWND hScrollBar) {
		RectWrapper classinRC, viewRC;
		GetWindowRect(classinWindow.hWnd, &classinRC);
		GetClientRect(viewWindow.hWnd, &viewRC);
		switch (request) {
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			viewWindow.viewPos.x = position;
			SetScrollPos(hWnd, SB_HORZ, position, TRUE);
			invalidate();
			break;
		case SB_PAGELEFT:
		case SB_LINELEFT:
			viewWindow.viewPos.x -= 20;
			limit(viewWindow.viewPos.x, 0, classinRC.w() / viewWindow.viewScale - viewRC.w());
			SetScrollPos(hWnd, SB_HORZ, viewWindow.viewPos.x, TRUE);
			invalidate();
			break;
		case SB_PAGERIGHT:
		case SB_LINERIGHT:
			viewWindow.viewPos.x += 20;
			limit(viewWindow.viewPos.x, 0, classinRC.w() / viewWindow.viewScale - viewRC.w());
			SetScrollPos(hWnd, SB_HORZ, viewWindow.viewPos.x, TRUE);
			invalidate();
			break;
		default:
			break;
		}
		return 0;
	}
	LRESULT onVScroll(HWND hWnd, WORD request, WORD position, HWND hScrollBar) {
		RectWrapper classinRC, viewRC;
		GetWindowRect(classinWindow.hWnd, &classinRC);
		GetClientRect(viewWindow.hWnd, &viewRC);
		switch (request) {
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			viewWindow.viewPos.y = position;
			SetScrollPos(hWnd, SB_VERT, position, TRUE);
			invalidate();
			break;
		case SB_PAGELEFT:
		case SB_LINELEFT:
			viewWindow.viewPos.y -= 20;
			limit(viewWindow.viewPos.y, 0, classinRC.h() / viewWindow.viewScale - viewRC.h());
			SetScrollPos(hWnd, SB_VERT, viewWindow.viewPos.y, TRUE);
			invalidate();
			break;
		case SB_PAGERIGHT:
		case SB_LINERIGHT:
			viewWindow.viewPos.y += 20;
			limit(viewWindow.viewPos.y, 0, classinRC.h() / viewWindow.viewScale - viewRC.h());
			SetScrollPos(hWnd, SB_VERT, viewWindow.viewPos.y, TRUE);
			invalidate();
			break;
		default:
			break;
		}
		return 0;
	}
	LRESULT onCommand(HWND hWnd, WORD nCtlNotifyCode, WORD nCtlID, HWND hControl) {
		RectWrapper rc;
		switch (nCtlID) {
		case idMenu://menu refresh
		{
			//refresh HWND's & Rect
			attachClassroom(getClassroomHWND());
			//attachClassroom(getClassroomHWND());
			// create DC & BMP
			CheckReleaseDC(hWnd, viewWindow.hWndDC);
			CheckDeleteDC(viewWindow.hMemDC);
			CheckDeleteObject(viewWindow.hBMP);
			viewWindow.hWndDC = GetDC(hWnd);
			viewWindow.hMemDC = CreateCompatibleDC(viewWindow.hWndDC);
			viewWindow.hBMP = CreateCompatibleBitmap(viewWindow.hWndDC, classinWindow.originalRect.w(), classinWindow.originalRect.h());
			SelectObject(viewWindow.hMemDC, viewWindow.hBMP);
			updateView();
			updateScrolls();
		}
		break;
		case idMenu + 1://menu hide
			setHide(!classinWindow.hide);
			break;
		case idMenu + 2://menu Bottom most
			setBottomMost(!classinWindow.bottomMost);
			break;
		case idMenu + 3://menu close
			SendMessageW(classinWindow.hWnd, WM_CLOSE, NULL, NULL);
			detachClassroom();
			invalidate();
			break;
		case idMenu + 4://menu relevant sizing
			setFocusMode(false);
			//setRelevantSizing(!classinWindow.relevantSizing);
			break;
		case idMenu + 5://menu 100%
			setViewScale(SCALE_100);
			break;
		case idMenu + 6://menu 125%
			setViewScale(SCALE_125);
			break;
		case idMenu + 7://menu 150%
			setViewScale(SCALE_150);
			break;
		case idMenu + 8://menu 175%
			setViewScale(SCALE_175);
			break;
		case idMenu + 9://menu 200%
			setViewScale(SCALE_200);
			break;
		case idMenu + 10://menu 225%
			setViewScale(SCALE_225);
			break;
		case idMenu + 100://menu topmost
			viewWindow.topmost = !viewWindow.topmost;
			if (viewWindow.topmost) {
				CheckMenuItem(viewWindow.hMenuClassOut, idMenu + 100, MF_BYCOMMAND | MF_CHECKED);
				SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
			}
			else {
				CheckMenuItem(viewWindow.hMenuClassOut, idMenu + 100, MF_BYCOMMAND | MF_UNCHECKED);
				SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
			}
			break;
		case idMenu + 201://menu start classin
			startClassInWithDll();
			break;
		case idMenu + 202://menu connect
			if (!(evtNotify.open() && smNotify.open())) {
				MessageBoxW(viewWindow.hWnd, L"ClassIn not present.", L"Error", MB_ICONEXCLAMATION | MB_OK);
			}
			pNotify = (notifyStruct*)smNotify.map();
			// TODO: exception handling
			assert(pNotify->classinPresent);
			pNotify->classoutPresent = true;
			pNotify->classoutWnd = viewWindow.hWnd;
			SetWindowTextW(viewWindow.hWnd, L"ClassIn 释放器：ClassIn 已连接");
			break;
		default:
			return -1;
			break;
		}
		return 0;
	}
	LRESULT onChar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (classinWindow.attached)
			return SendMessageW(classinWindow.hWnd, message, wParam, lParam);
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	LRESULT onClassInCursorChanged(HWND hWnd, LPWSTR cursor) {
		trace(L"in cursor changed\n");
		SetCursor(LoadCursorW(hInstance, cursor));
		return TRUE;
	}

	void invalidate() {
		RectWrapper rc;
		GetClientRect(viewWindow.hWnd, &rc);
		InvalidateRect(viewWindow.hWnd, &rc, false);
	}
	void updateScrolls() {
		RectWrapper classinRC, viewRC;
		SCROLLINFO info{ 0 };
		info.cbSize = sizeof(SCROLLINFO);
		if (classinWindow.attached) {
			GetWindowRect(classinWindow.hWnd, &classinRC);
			GetClientRect(viewWindow.hWnd, &viewRC);
			info.fMask = SIF_ALL;
			info.nMin = 0;
			info.nMax = classinRC.w() / classinWindow.scale;
			info.nPage = viewRC.w() / viewWindow.viewScale;
			info.nPos = viewWindow.viewPos.x;
			info.nTrackPos = 0;
			SetScrollInfo(viewWindow.hWnd, SB_HORZ, &info, TRUE);
			info.fMask = SIF_ALL;
			info.nMin = 0;
			info.nMax = classinRC.h() / classinWindow.scale;
			info.nPage = viewRC.h() / viewWindow.viewScale;
			info.nPos = viewWindow.viewPos.y;
			info.nTrackPos = 0;
			SetScrollInfo(viewWindow.hWnd, SB_VERT, &info, TRUE);
		}
		else {
			// not attached: disable
			info.fMask = SIF_ALL;
			SetScrollInfo(viewWindow.hWnd, SB_HORZ, &info, TRUE);
			SetScrollInfo(viewWindow.hWnd, SB_VERT, &info, TRUE);
		}

	}
	void updateView() {
		bool success = true;
		if (classinWindow.attached) {
			RectWrapper classinRC, viewRC;
			GetWindowRect(classinWindow.hWnd, &classinRC);
			GetClientRect(viewWindow.hWnd, &viewRC);
			/*
			HWND hDesktopWnd = classinWindow.hWnd;
			HDC hDesktopDC = GetDC(hDesktopWnd);
			HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
			HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
				rc.w(), rc.h());

			SelectObject(hCaptureDC, hCaptureBitmap);
			BitBlt(hCaptureDC, 0, 0, rc.w(), rc.h(),
				hDesktopDC, 0, 0, SRCCOPY);
			BitBlt(
				viewWindow.hWndDC, 0, 0, viewWindow.viewRect.w(), viewWindow.viewRect.h(),
				hCaptureDC, viewWindow.viewRect.x(), viewWindow.viewRect.y(),
				SRCCOPY
			);

			ReleaseDC(hDesktopWnd, hDesktopDC);
			DeleteDC(hCaptureDC);
			DeleteObject(hCaptureBitmap);


			return;
			*/
			//ClassInWindow valid
			success &= (bool)PrintWindow(classinWindow.hWnd, viewWindow.hMemDC, 0);
			//SetStretchBltMode(viewWindow.hWndDC, HALFTONE);
			//SetBrushOrgEx(viewWindow.hWndDC, 0, 0, NULL);
			success &= (bool)StretchBlt(
				viewWindow.hWndDC, 0, 0, viewRC.w(), viewRC.h(),
				viewWindow.hMemDC, viewWindow.viewPos.x, viewWindow.viewPos.y, viewRC.w() / viewWindow.viewScale, viewRC.h() / viewWindow.viewScale,
				SRCCOPY
			);
			/*
			success &= (bool)BitBlt(
				viewWindow.hWndDC, 0, 0, viewWindow.viewRect.w(), viewWindow.viewRect.h(),
				viewWindow.hMemDC, viewWindow.viewRect.x(), viewWindow.viewRect.y(),
				SRCCOPY
			);
			*/
			if (!success && GetLastError() == ERROR_INVALID_HANDLE) {
				detachClassroom();
			}
		}
	}
	void attachClassroom(HWND hWnd) {
		if (classinWindow.attached && hWnd != classinWindow.hWnd) {
			detachClassroom();
		}
		if (hWnd) {
			classinWindow.hWnd = hWnd;
			classinWindow.attached = GetWindowRect(hWnd, &classinWindow.originalRect);
			classinWindow.hide = false, classinWindow.bottomMost = false, classinWindow.relevantSizing = false;
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 1, MF_BYCOMMAND | MF_UNCHECKED);
			SIZE sz{ 0 };
			GetRealWindowDimension(hWnd, &sz);

			// use average
			classinWindow.scale = (classinWindow.originalRect.w() * 1.0 / sz.cx + classinWindow.originalRect.h() * 1.0 / sz.cy) * 0.5;
			if (abs(classinWindow.scale - 1.00) < 0.01)classinWindow.scale = 1.00;
			else if (abs(classinWindow.scale - 1.25) < 0.01)classinWindow.scale = 1.25;
			else if (abs(classinWindow.scale - 1.50) < 0.01)classinWindow.scale = 1.50;
			else if (abs(classinWindow.scale - 1.75) < 0.01)classinWindow.scale = 1.75;
			else if (abs(classinWindow.scale - 2.00) < 0.01)classinWindow.scale = 2.00;
			else if (abs(classinWindow.scale - 2.25) < 0.01)classinWindow.scale = 2.25;
		}
	}
	void detachClassroom() {
		if (classinWindow.attached) {
			// restore to where it started
			SetWindowPos(
				classinWindow.hWnd, HWND_TOPMOST,
				classinWindow.originalRect.x(), classinWindow.originalRect.y(),
				classinWindow.originalRect.w(), classinWindow.originalRect.h(),
				//SWP_SHOWWINDOW
				SWP_SHOWWINDOW | SWP_NOZORDER
			);
			classinWindow.hWnd = NULL;
			classinWindow.hide = false, classinWindow.bottomMost = false, classinWindow.relevantSizing = false;
		}
	}
	void setRelevantSizing(bool enable) {
		static LONG prevW = 0, prevH = 0, prevX = 0, prevY = 0;
		RectWrapper rc;
		if (enable) {
			// enable
			GetWindowRect(classinWindow.hWnd, &rc);
			// store previous size
			prevW = rc.w(), prevH = rc.h();
			prevX = viewWindow.viewPos.x, prevY = viewWindow.viewPos.y;
			// resize
			GetClientRect(viewWindow.hWnd, &rc);
			classinWindow.relevantSizing = SetWindowPos(viewWindow.hWnd, NULL, 0, 0, rc.w(), rc.h(), SWP_NOMOVE | SWP_NOZORDER);
			viewWindow.viewPos.xy(0, 0);
		}
		else {
			// disable
			// restore size
			classinWindow.relevantSizing = !SetWindowPos(viewWindow.hWnd, NULL, prevW, prevH, rc.w(), rc.h(), SWP_NOMOVE | SWP_NOZORDER);
			GetWindowRect(classinWindow.hWnd, &rc);
			viewWindow.viewPos.xy(prevX, prevY);
		}
		if (classinWindow.relevantSizing) {
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 4, MF_BYCOMMAND | MF_CHECKED);
		}
		else {
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 4, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}
	void setHide(bool enable) {
		static LONG oriX = 0, oriY = 0;
		RectWrapper rc;
		// changes window pos
		if (classinWindow.attached) {
			if (enable) {// hide
				GetWindowRect(classinWindow.hWnd, &rc);
				oriX = rc.x(), oriY = rc.y();
				classinWindow.hide = SetWindowPos(
					classinWindow.hWnd, NULL,
					classinWindow.xHide, classinWindow.yHide, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW
				);
			}
			else { // unhide
				classinWindow.hide = !SetWindowPos(
					classinWindow.hWnd, NULL,
					oriX, oriY, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW
				);
			}
			if (classinWindow.hide)
				CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 1, MF_BYCOMMAND | MF_CHECKED);
			else
				CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 1, MF_BYCOMMAND | MF_UNCHECKED);
		}

	}
	void setBottomMost(bool enable) {
		// changes z-order
		if (classinWindow.attached) {
			if (enable)
				classinWindow.bottomMost = SetWindowPos(
					classinWindow.hWnd, HWND_BOTTOM, 0, 0, 0, 0,
					SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE
				);
			else
				classinWindow.bottomMost = !SetWindowPos(
					classinWindow.hWnd, HWND_TOP, 0, 0, 0, 0,
					SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW
				);
		}
		if (classinWindow.bottomMost)
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 2, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 2, MF_BYCOMMAND | MF_UNCHECKED);

	}
	enum viewScale {
		SCALE_100,
		SCALE_125,
		SCALE_150,
		SCALE_175,
		SCALE_200,
		SCALE_225,
		SCALE_AUTO
	};
	void setViewScale(viewScale newScale) {
		CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 5, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 6, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 7, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 8, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 9, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 10, MF_BYCOMMAND | MF_UNCHECKED);
		switch (newScale) {
		case ClassOutApp::SCALE_100:
			viewWindow.viewScale = 1.00;
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 5, MF_BYCOMMAND | MF_CHECKED);
			break;
		case ClassOutApp::SCALE_125:
			viewWindow.viewScale = 1.25;
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 6, MF_BYCOMMAND | MF_CHECKED);
			break;
		case ClassOutApp::SCALE_150:
			viewWindow.viewScale = 1.50;
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 7, MF_BYCOMMAND | MF_CHECKED);
			break;
		case ClassOutApp::SCALE_175:
			viewWindow.viewScale = 1.75;
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 8, MF_BYCOMMAND | MF_CHECKED);
			break;
		case ClassOutApp::SCALE_200:
			viewWindow.viewScale = 2.00;
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 9, MF_BYCOMMAND | MF_CHECKED);
			break;
		case ClassOutApp::SCALE_225:
			viewWindow.viewScale = 2.25;
			CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 10, MF_BYCOMMAND | MF_CHECKED);
			break;
		case ClassOutApp::SCALE_AUTO:
			break;
		default:
			break;
		}
		updateScrolls();
	}
	HWND getClassroomHWND() {
		// GetPidByName
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		HWND hCIWnd = NULL;
		DWORD checkProcessID = 0;
		WCHAR lpWindowCaption[16] = { 0 };
		if (INVALID_HANDLE_VALUE == hSnapshot) {
			return NULL;
		}
		PROCESSENTRY32W pe = { sizeof(pe) };
		for (BOOL ret = Process32FirstW(hSnapshot, &pe); ret; ret = Process32NextW(hSnapshot, &pe)) {
			// compare names
			if (wcscmp(pe.szExeFile, L"ClassIn.exe") == 0) {
				classinWindow.dwProcessID = pe.th32ProcessID;
				break;
			}
		}
		CloseHandle(hSnapshot);
		if (classinWindow.dwProcessID == 0)
			return NULL;

		//GetFirstWindowsFromProcessID
		do {
			hCIWnd = FindWindowExW(NULL, hCIWnd, NULL, NULL);
			GetWindowThreadProcessId(hCIWnd, &checkProcessID);
			if (checkProcessID == classinWindow.dwProcessID) {
				GetWindowTextW(hCIWnd, lpWindowCaption, 16);
				//if (wcscmp(lpWindowCaption, L"ClassIn") == 0) {
				if (wcsncmp(lpWindowCaption, L"Classroom_", wcslen(L"Classroom_")) == 0) {
					return hCIWnd;
				}
			}
		} while (hCIWnd != NULL);
		return NULL;
	}
	void exportDll() {
		char tempDir[MAX_PATH]{ 0 }, dir[MAX_PATH]{ 0 };
		DWORD len = GetTempPathA(MAX_PATH, tempDir);
		PathCombineA(dir, tempDir, "ClassOut");
		DWORD attr = GetFileAttributesA(dir);
		if (attr == INVALID_FILE_ATTRIBUTES) {
			CreateDirectoryA(dir, NULL);
		}
		PathCombineA(dir, tempDir, "ClassOut\\helperDll.dll");
		attr = GetFileAttributesA(dir);
		//if (attr == INVALID_FILE_ATTRIBUTES) {
		{
			HRSRC hRs = FindResourceW(hInstance, MAKEINTRESOURCEW(IDR_DLL1), L"DLL");
			HGLOBAL hGlo = LoadResource(hInstance, hRs);
			void* pData = LockResource(hGlo);
			DWORD size = SizeofResource(hInstance, hRs);
			HANDLE hFile = CreateFileA(dir, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile(hFile, pData, size, NULL, NULL);
			CloseHandle(hFile);
			FreeResource(hGlo);
		}
		strcpy(dllFile, dir);
		hHelperDll = LoadLibraryA(dllFile);
	}
	void startClassInWithDll() {
		STARTUPINFOW si;
		PROCESS_INFORMATION pi;
		si.cb = sizeof(si);
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		bool b = DetourCreateProcessWithDllW(
			L"C:\\Program Files (x86)\\ClassIn\\ClassIn.exe",
			NULL, NULL, NULL, FALSE, 0, NULL,
			L"C:\\Program Files (x86)\\ClassIn\\",
			&si, &pi, dllFile, NULL
		);
	}
	static BOOL WINAPI GetRealWindowDimension(HWND hWnd, LPSIZE lpSize) {
		//## regardless of system zoom scale ##
		BITMAP bm{ 0 };
		ShowWindow(hWnd, SW_NORMAL);
		HDC hDC = GetDC(hWnd);
		HGDIOBJ hBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
		int ret = GetObjectW(hBitmap, sizeof(BITMAP), &bm);
		ReleaseDC(hWnd, hDC);
		if (ret == 0)
			return FALSE;
		lpSize->cx = bm.bmWidth, lpSize->cy = bm.bmHeight;
		return TRUE;
	}
	static BOOL WINAPI GetRealDCDimension(HDC hDC, LPSIZE lpSize) {
		BITMAP bm{ 0 };
		HGDIOBJ hBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
		int ret = GetObjectW(hBitmap, sizeof(BITMAP), &bm);
		if (ret == 0)
			return FALSE;
		lpSize->cx = bm.bmWidth, lpSize->cy = bm.bmHeight;
		return TRUE;
	}
	void setFocusMode(bool enable) {
		RectWrapper rc;
		if (classinWindow.attached) {
			if (enable) {

			}
			else {
				ShowWindow(viewWindow.hWnd, SW_MAXIMIZE);
				GetWindowRect(viewWindow.hWnd, &rc);
				int diff = -rc.y();
				rc.left = rc.top = 0;
				rc.right -= diff, rc.bottom -= diff;
				SetWindowPos(classinWindow.hWnd, HWND_NOTOPMOST, rc.x(), rc.y(), rc.w(), rc.h(), SWP_SHOWWINDOW);
			}
		}
	}
	// limit val to [min, max) and returns true if val is changed
	template<typename T, typename Tx, typename Ty>
	static bool limit(T& val, Tx min, Ty max) {
		if (val < min) {
			val = min;
			return true;
		}
		if (val >= max) {
			val = max - 1;
			return true;
		}
		return false;
	}
	static double getZoom() {
		HWND hWnd = GetDesktopWindow();
		HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		// monitor logical width
		MONITORINFOEXW monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		GetMonitorInfo(hMonitor, &monitorInfo);
		int cxLogical = (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
		// monitor physical width
		DEVMODEW dm;
		dm.dmSize = sizeof(dm);
		dm.dmDriverExtra = 0;
		EnumDisplaySettingsW(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &dm);
		int cxPhysical = dm.dmPelsWidth;

		return cxPhysical * 1.0 / cxLogical;
	}
	static DWORD WINAPI classOutRoutine(LPVOID param) {
		while (pInstance->evtNotify.wait()) {

		}
	}

	struct ViewWrapper {
		HWND hWnd = NULL;
		HDC hWndDC = NULL, hMemDC = NULL;
		HBITMAP hBMP = NULL;
		//RectWrapper viewRect;// viewRect: relevant to classin window
		PointWrapper viewPos{ 0,0 };
		bool topmost = false;
		double viewScale = 1.;
		HMENU hMenuRoot = NULL, hMenuClassOut = NULL, hMenuClassIn = NULL, hMenuConnect = NULL;
	}viewWindow;
	struct ClassInWrapper {
		const LONG xHide = 6666, yHide = 6666;
		bool attached = false;
		HWND hWnd = NULL;
		DWORD dwProcessID = 0;
		RectWrapper originalRect;// where it all starts
		//RectWrapper changedRect;
		bool dpiAware = false;
		double scale = 1.;
		bool hide = false, bottomMost = false, relevantSizing = false;
		bool initialFocusMode = false;
	}classinWindow;

	win32Event evtNotify;
	win32SharedMemory smNotify;
	notifyStruct* pNotify = NULL;
	bool connected = false;
	HWND hButton[3];
	CONST static UINT_PTR idTimer = 2233;
	CONST static UINT_PTR idMenu = 4233;
	CONST static INT idHotkey = 5233;
	char dllFile[MAX_PATH]{ 0 };
	HINSTANCE hInstance;
	HMODULE hHelperDll;
	HFONT hFontMSYH32;
	double systemScale = 1.;
	static ClassOutApp* pInstance;
};
ClassOutApp* ClassOutApp::pInstance = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	ClassOutApp app(hInstance);
	return app.exec();
}

LRESULT ClassOutApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	wchar_t str[2]{ 0 };
	switch (message) {
	case WM_CREATE:
		return pInstance->onCreate(hWnd, (LPCREATESTRUCTW)lParam);
	case WM_PAINT:
		return pInstance->onPaint(hWnd);
	case WM_SIZING:
		return pInstance->onSizing(hWnd, wParam, (LPRECT)lParam);
	case WM_SIZE:
		return pInstance->onSize(hWnd, LOWORD(lParam), HIWORD(lParam));
	case WM_EXITSIZEMOVE:
		return pInstance->onExitSizeMove(hWnd);
	case WM_TIMER:
		return pInstance->onTimer(hWnd, (UINT_PTR)wParam, (TIMERPROC)lParam);
	case WM_HOTKEY:
		return pInstance->onHotKey(hWnd, (int)wParam, (UINT)lParam);
	case WM_COMMAND:
		if (pInstance->onCommand(hWnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam) == 0)
			return 0;
		return DefWindowProcW(hWnd, message, wParam, lParam);
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		//case WM_SETCURSOR:
		return pInstance->onMouseMsg(hWnd, message, wParam, lParam);
	case WM_CUT:
	case WM_COPY:
	case WM_PASTE:
	case WM_CLEAR:
	case WM_UNDO:
		return pInstance->onClipboardMsg(hWnd, message);
	case WM_MOUSEWHEEL:
		return pInstance->onMouseWheel(hWnd, message, wParam, lParam);
	case WM_HSCROLL:
		return pInstance->onHScroll(hWnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
	case WM_VSCROLL:
		return pInstance->onVScroll(hWnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
	case WM_CLOSE:
		return pInstance->onClose(hWnd, wParam, lParam);
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		return pInstance->onKeyboardMsg(hWnd, message, wParam, lParam);
	case WM_IME_CHAR:
	//case WM_CHAR:
		return pInstance->onChar(hWnd, message, wParam, lParam);
	case NOTIFY_CURSOR_CHANGED:
		return pInstance->onClassInCursorChanged(hWnd, (LPWSTR)wParam);
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}
