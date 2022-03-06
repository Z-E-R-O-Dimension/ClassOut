#pragma once
#include <Windows.h>

const wchar_t onStageStr[] = L"你上台了";
const wchar_t offStageStr[] = L"你下台了";

enum notifyMesage {
	NOTIFY_CURSOR_CHANGED = WM_USER + 1,
	NOTIFY_STRING_UPDATE,
	NOTIFY_CLASSIN_EXIT,
};

struct notifyStruct {
	bool classinPresent;
	bool classoutPresent;
	HWND classoutWnd;
	wchar_t string[260];
	LPWSTR cursorToLoad;
};