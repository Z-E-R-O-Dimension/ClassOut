#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <inttypes.h>
#include <exception>
#include <assert.h>
#include "../include/Messager.h"
#include "resource.h"
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

class ClassOutApp {
public:
    ClassOutApp(HINSTANCE _hInstance) :hInstance(_hInstance), channel(L"ClassOutHelper") {
        if(!pInstance)
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
        while(GetMessageW(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return (int)msg.wParam;
    }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void init() {
        systemZoom = getZoom();
        SetProcessDPIAware();
        channel.create(sizeof(injectThreadMessage));
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
        AppendMenuW(viewWindow.hMenuRoot, MF_POPUP, (UINT_PTR)viewWindow.hMenuClassOut, L"ClassOut");
        AppendMenuW(viewWindow.hMenuRoot, MF_POPUP, (UINT_PTR)viewWindow.hMenuClassIn, L"ClassIn");
        AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu, L"Ë¢ÐÂ");
        AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 1, L"Òþ²Ø");
        AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 2, L"ÖÃµ×");
        AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 3, L"¹Ø±Õ");
        AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 4, L"Relevant Sizing");
        AppendMenuW(viewWindow.hMenuClassIn, MF_STRING, idMenu + 5, L"Dll Inject");
        AppendMenuW(viewWindow.hMenuClassOut, MF_STRING, idMenu + 100, L"ÖÃ¶¥");

        // create window
        viewWindow.hWnd = CreateWindowW(
            wcex.lpszClassName, L"ClassOut", WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
            CW_USEDEFAULT, 0, 800 * systemZoom, 600 * systemZoom, NULL, viewWindow.hMenuRoot, hInstance, NULL
        );
        ShowWindow(viewWindow.hWnd, SW_SHOW);
        UpdateWindow(viewWindow.hWnd);
    }

    LRESULT onCreate(HWND hWnd, LPCREATESTRUCTW pCS) {
        // ctrl + F1
        RegisterHotKey(hWnd, idHotkey, MOD_CONTROL, VK_F1);
        // font msyh
        hFontMSYH32 = CreateFontW(32 * systemZoom, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Î¢ÈíÑÅºÚ");
        //setup standard scroll bars
        updateScrolls();
        //refresh at 60Hz
        SetTimer(hWnd, idTimer, 1000 / 60, NULL);
        //viewWindow.hWndDC = GetDC(hWnd);
        //viewWindow.hMemDC = CreateCompatibleDC(viewWindow.hWndDC);
        return 0;
    }
    LRESULT onPaint(HWND hWnd) {
        PAINTSTRUCT ps{ 0 };
        HDC hdcPaint;
        hdcPaint = BeginPaint(hWnd, &ps);
        if(classinWindow.attached) {
            //ClassInWindow valid
            updateView();
        }
        else {
            RectWrapper rc;
            GetClientRect(hWnd, &rc);
            SelectObject(hdcPaint, hFontMSYH32);
            DrawTextW(hdcPaint, L"ÊÍ·Å£ºµÈ´ýClassIn½ÌÊÒ´°¿Ú", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    LRESULT onSize(HWND hWnd, UINT nWidth, UINT nHeight) {
        RectWrapper rc;
        viewWindow.viewRect.wh(nWidth, nHeight);
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
        return 0;
    }
    LRESULT onHotKey(HWND hWnd, int id, UINT fsModifiers) {
        POINT pt;
        RectWrapper rc;
        if(id == idHotkey) {
            viewWindow.topmost = !viewWindow.topmost;
            if(viewWindow.topmost) {
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
        if(classinWindow.attached)
            PostMessageW(classinWindow.hWnd, message, wParam, MAKELPARAM(
                LOWORD(lParam) + viewWindow.viewRect.x(),
                HIWORD(lParam) + viewWindow.viewRect.y()
            ));
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    LRESULT onMouseWheel(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        RectWrapper rc;
        if(classinWindow.attached) {
            GetWindowRect(hWnd, &rc);
            if(classinWindow.hide) {
                return SendMessageW(classinWindow.hWnd, message, wParam, MAKELPARAM(
                    LOWORD(lParam) + classinWindow.xHide - rc.x() + viewWindow.viewRect.x(),
                    LOWORD(lParam) + classinWindow.yHide - rc.y() + viewWindow.viewRect.y()
                ));
            }
            else {
                return SendMessageW(classinWindow.hWnd, message, wParam, MAKELPARAM(
                    LOWORD(lParam) + classinWindow.changedRect.x() - rc.x() + viewWindow.viewRect.x(),
                    HIWORD(lParam) + classinWindow.changedRect.y() - rc.y() + viewWindow.viewRect.y()
                ));
            }
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);

    }
    LRESULT onClipboardMsg(HWND hWnd, UINT message) {
        if(classinWindow.attached)
            SendMessageW(classinWindow.hWnd, message, 0, 0);
        return DefWindowProcW(hWnd, message, 0, 0);
    }
    LRESULT onTimer(HWND hWnd, UINT_PTR nIDEvent, TIMERPROC lpTimerFunc) {
        switch(nIDEvent) {
        case idTimer:
            if(!classinWindow.attached)
                break;
            invalidate();
            break;
        default:
            break;
        }
        return 0;
    }
    LRESULT onHScroll(HWND hWnd, WORD request, WORD position, HWND hScrollBar) {
        switch(request) {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            viewWindow.viewRect.x() = position;
            SetScrollPos(hWnd, SB_HORZ, position, TRUE);
            invalidate();
            break;
        case SB_PAGELEFT:
        case SB_LINELEFT:
            viewWindow.viewRect.x() -= 20;
            if(viewWindow.viewRect.x() < 0)
                viewWindow.viewRect.x() = 0;
            if(viewWindow.viewRect.x() > classinWindow.changedRect.w() - viewWindow.viewRect.w())
                viewWindow.viewRect.x() = classinWindow.changedRect.w() - viewWindow.viewRect.w();
            SetScrollPos(hWnd, SB_HORZ, viewWindow.viewRect.x(), TRUE);
            invalidate();
            break;
        case SB_PAGERIGHT:
        case SB_LINERIGHT:
            viewWindow.viewRect.x() += 20;
            if(viewWindow.viewRect.x() < 0)
                viewWindow.viewRect.x() = 0;
            if(viewWindow.viewRect.x() > classinWindow.changedRect.w() - viewWindow.viewRect.w())
                viewWindow.viewRect.x() = classinWindow.changedRect.w() - viewWindow.viewRect.w();
            SetScrollPos(hWnd, SB_HORZ, viewWindow.viewRect.x(), TRUE);
            invalidate();
            break;
        default:
            break;
        }
        return 0;
    }
    LRESULT onVScroll(HWND hWnd, WORD request, WORD position, HWND hScrollBar) {
        switch(request) {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            viewWindow.viewRect.y() = position;
            SetScrollPos(hWnd, SB_VERT, position, TRUE);
            invalidate();
            break;
        case SB_PAGELEFT:
        case SB_LINELEFT:
            viewWindow.viewRect.y() -= 20;
            if(viewWindow.viewRect.y() < 0)
                viewWindow.viewRect.y() = 0;
            if(viewWindow.viewRect.y() > classinWindow.changedRect.h() - viewWindow.viewRect.h())
                viewWindow.viewRect.y() = classinWindow.changedRect.h() - viewWindow.viewRect.h();
            SetScrollPos(hWnd, SB_VERT, viewWindow.viewRect.y(), TRUE);
            invalidate();
            break;
        case SB_PAGERIGHT:
        case SB_LINERIGHT:
            viewWindow.viewRect.y() += 20;
            if(viewWindow.viewRect.y() < 0)
                viewWindow.viewRect.y() = 0;
            if(viewWindow.viewRect.y() > classinWindow.changedRect.h() - viewWindow.viewRect.h())
                viewWindow.viewRect.y() = classinWindow.changedRect.h() - viewWindow.viewRect.h();
            SetScrollPos(hWnd, SB_VERT, viewWindow.viewRect.y(), TRUE);
            invalidate();
            break;
        default:
            break;
        }
        return 0;

    }
    LRESULT onExitSizeMove(HWND hWnd) {
        RectWrapper rc;
        if(classinWindow.relevantSizing) {
            GetClientRect(hWnd, &rc);
            SetWindowPos(classinWindow.hWnd, NULL, 0, 0, rc.w(), rc.h(), SWP_NOMOVE | SWP_NOZORDER);
        }
        return 0;
    }
    LRESULT onCommand(HWND hWnd, WORD nCtlNotifyCode, WORD nCtlID, HWND hControl) {
        RectWrapper rc;
        switch(nCtlID) {
        case idMenu://menu refresh
        {
            //refresh HWND's & Rect
            attachClassroom(getClassroomHWND());
            RectWrapper rc;
            GetClientRect(hWnd, &rc);
            viewWindow.viewRect.wh(rc.w(), rc.h());
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
            setRelevantSizing(!classinWindow.relevantSizing);
            break;
            /*
            {
                if(!hOutputHandle) {
                    AllocConsole();
                    hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
                    freopen("CONOUT$", "w+t", stdout);
                    freopen("CONOUT$", "w+t", stderr);
                    freopen("CONIN$", "r+t", stdin);
                }
                else {
                    if(FreeConsole())
                        hOutputHandle = NULL;
                    else
                        std::cout << "free console failed\n";
                }
                vhwnd.clear();
                GetAllWindowsFromProcessID(GetPIDByName(lpClassInProcessName), vhwnd);
                std::wcout << L"\n";
                for(auto& obj : vhwnd) {
                    GetWindowTextW(obj, lpStr, 32);
                    std::wcout << L"HWND:" << obj << L" Caption" << lpStr << L"\n";
                }
            }

            */
        case idMenu + 100://menu topmost
        {
            viewWindow.topmost = !viewWindow.topmost;
            if(viewWindow.topmost) {
                CheckMenuItem(viewWindow.hMenuClassOut, idMenu + 100, MF_BYCOMMAND | MF_CHECKED);
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
            }
            else {
                CheckMenuItem(viewWindow.hMenuClassOut, idMenu + 100, MF_BYCOMMAND | MF_UNCHECKED);
                SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
            }
            break;
        default:
            return -1;
            break;
        }
        }
        return 0;
    }

    void invalidate() {
        RectWrapper rc;
        GetClientRect(viewWindow.hWnd, &rc);
        InvalidateRect(viewWindow.hWnd, &rc, false);
    }
    void updateScrolls() {
        SCROLLINFO info{ 0 };
        info.cbSize = sizeof(SCROLLINFO);
        if(classinWindow.attached) {
            GetWindowRect(classinWindow.hWnd, &classinWindow.changedRect);
            info.fMask = SIF_ALL;
            info.nMin = 0;
            info.nMax = classinWindow.changedRect.w();
            info.nPage = viewWindow.viewRect.w();
            info.nPos = viewWindow.viewRect.x();
            info.nTrackPos = 0;
            SetScrollInfo(viewWindow.hWnd, SB_HORZ, &info, TRUE);
            info.fMask = SIF_ALL;
            info.nMin = 0;
            info.nMax = classinWindow.changedRect.h();
            info.nPage = viewWindow.viewRect.h();
            info.nPos = viewWindow.viewRect.y();
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
        if(classinWindow.attached) {
            //ClassInWindow valid
            success &= (bool)PrintWindow(classinWindow.hWnd, viewWindow.hMemDC, 0);
            success &= (bool)BitBlt(
                viewWindow.hWndDC, 0, 0, viewWindow.viewRect.w(), viewWindow.viewRect.h(),
                viewWindow.hMemDC, viewWindow.viewRect.x(), viewWindow.viewRect.y(),
                SRCCOPY
            );
            if(!success && GetLastError() == ERROR_INVALID_HANDLE) {
                detachClassroom();
            }
        }
    }
    void attachClassroom(HWND hWnd) {
        if(classinWindow.attached && hWnd != classinWindow.hWnd) {
            detachClassroom();
        }
        if(hWnd) {
            classinWindow.hWnd = hWnd;
            classinWindow.attached = GetWindowRect(hWnd, &classinWindow.originalRect);
            classinWindow.hide = false, classinWindow.bottomMost = false, classinWindow.relevantSizing = false;
            CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 1, MF_BYCOMMAND | MF_UNCHECKED);
        }

    }
    void detachClassroom() {
        if(classinWindow.attached) {
            // restore to where it started
            SetWindowPos(
                classinWindow.hWnd, HWND_TOPMOST,
                classinWindow.originalRect.x(), classinWindow.originalRect.y(),
                classinWindow.originalRect.w(), classinWindow.originalRect.h(),
                SWP_SHOWWINDOW | SWP_NOZORDER
            );
            classinWindow.hWnd = NULL;
            classinWindow.hide = false, classinWindow.bottomMost = false, classinWindow.relevantSizing = false;
        }
    }
    void setRelevantSizing(bool enable) {
        static LONG prevW = 0, prevH = 0, prevX = 0, prevY = 0;
        RectWrapper rc;
        if(enable) {
            // enable
            GetWindowRect(classinWindow.hWnd, &rc);
            // store previous size
            prevW = rc.w(), prevH = rc.h();
            prevX = viewWindow.viewRect.x(), prevY = viewWindow.viewRect.y();
            // resize
            GetClientRect(viewWindow.hWnd, &rc);
            classinWindow.relevantSizing = SetWindowPos(viewWindow.hWnd, NULL, 0, 0, rc.w(), rc.h(), SWP_NOMOVE | SWP_NOZORDER);
            viewWindow.viewRect.xy(0, 0);
        }
        else {
            // disable
            // restore size
            classinWindow.relevantSizing = !SetWindowPos(viewWindow.hWnd, NULL, prevW, prevH, rc.w(), rc.h(), SWP_NOMOVE | SWP_NOZORDER);
            GetWindowRect(classinWindow.hWnd, &rc);
            viewWindow.viewRect.xy(prevX, prevY);
            viewWindow.viewRect.wh(rc.w(), rc.h());
        }
        if(classinWindow.relevantSizing) {
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
        if(classinWindow.attached) {
            if(enable) {// hide
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
            if(classinWindow.hide)
                CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 1, MF_BYCOMMAND | MF_CHECKED);
            else
                CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 1, MF_BYCOMMAND | MF_UNCHECKED);
        }

    }
    void setBottomMost(bool enable) {
        // changes z-order
        if(classinWindow.attached) {
            if(enable)
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
        if(classinWindow.bottomMost)
            CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 2, MF_BYCOMMAND | MF_CHECKED);
        else
            CheckMenuItem(viewWindow.hMenuClassIn, idMenu + 2, MF_BYCOMMAND | MF_UNCHECKED);

    }
    static HWND getClassroomHWND() {
        // GetPidByName
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        DWORD dwProcessID = 0;
        HWND hCIWnd = NULL;
        DWORD checkProcessID = 0;
        WCHAR lpWindowCaption[16] = { 0 };
        if(INVALID_HANDLE_VALUE == hSnapshot) {
            return NULL;
        }
        PROCESSENTRY32W pe = { sizeof(pe) };
        for(BOOL ret = Process32FirstW(hSnapshot, &pe); ret; ret = Process32NextW(hSnapshot, &pe)) {
            // compare names
            if(wcscmp(pe.szExeFile, L"ClassIn.exe") == 0) {
                dwProcessID = pe.th32ProcessID;
                break;
            }
        }
        CloseHandle(hSnapshot);
        if(dwProcessID == 0)
            return NULL;

        //GetFirstWindowsFromProcessID
        do {
            hCIWnd = FindWindowExW(NULL, hCIWnd, NULL, NULL);
            GetWindowThreadProcessId(hCIWnd, &checkProcessID);
            if(checkProcessID == dwProcessID) {
                GetWindowTextW(hCIWnd, lpWindowCaption, 16);
                if(wcscmp(lpWindowCaption, L"ClassIn") == 0) {
                //if(wcsncmp(lpWindowCaption, L"Classroom_", wcslen(L"Classroom_")) == 0) {
                    return hCIWnd;
                }
            }
        } while(hCIWnd != NULL);
        return NULL;
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

    struct ViewWrapper {
        HWND hWnd = NULL;
        HDC hWndDC = NULL, hMemDC = NULL;
        HBITMAP hBMP = NULL;
        RectWrapper viewRect;// viewRect: relevant to classin window
        bool topmost = false;
        HMENU hMenuRoot = NULL, hMenuClassOut = NULL, hMenuClassIn = NULL;
    }viewWindow;
    struct ClassInWrapper {
        const LONG xHide = 6666, yHide = 6666;
        bool attached = false;
        HWND hWnd = NULL;
        RectWrapper originalRect;// where it all starts
        RectWrapper changedRect;
        bool hide = false, bottomMost = false, relevantSizing = false;
    }classinWindow;

    struct injectThreadMessage {
        bool exec, succ, isDpiAware;
    };

    CONST static UINT_PTR idTimer = 2233;
    CONST static UINT_PTR idMenu = 4233;
    CONST static INT idHotkey = 5233;
    HINSTANCE hInstance;
    HFONT hFontMSYH32;
    double systemZoom = 1.;
    msgChannel channel;
    static ClassOutApp* pInstance;
};
ClassOutApp* ClassOutApp::pInstance = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    ClassOutApp app(hInstance);
    return app.exec();
}

LRESULT ClassOutApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    wchar_t str[2]{ 0 };
    switch(message) {
    case WM_CREATE:
        return pInstance->onCreate(hWnd, (LPCREATESTRUCTW)lParam);
    case WM_PAINT:
        return pInstance->onPaint(hWnd);
    case WM_SIZE:
        return pInstance->onSize(hWnd, LOWORD(lParam), HIWORD(lParam));
    case WM_EXITSIZEMOVE:
        return pInstance->onExitSizeMove(hWnd);
    case WM_TIMER:
        return pInstance->onTimer(hWnd, (UINT_PTR)wParam, (TIMERPROC)lParam);
    case WM_HOTKEY:
        return pInstance->onHotKey(hWnd, (int)wParam, (UINT)lParam);
    case WM_COMMAND:
        if(pInstance->onCommand(hWnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam) == 0)
            return 0;
        return DefWindowProcW(hWnd, message, wParam, lParam);
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_SETCURSOR:
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
    case WM_IME_CHAR:
        str[0] = wParam;
        str[1] = 0;
        OutputDebugStringW(str);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
