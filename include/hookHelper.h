#pragma once
#include <Windows.h>
#include <detours/detours.h>
#include <cassert>

class HookInfo {
public:
	class SourceInfo {
	public:
		enum class SourceType {
			FROM_GLOBAL_FUNCTION,
			FROM_DLL
		} sourceType;
		union SourceData {
			class FromGlobalFunc {
			public:
			} fromGlobalFunc;
			class FromDLL {
			public:
				HMODULE* phModule;
				LPCSTR decoratedName;
			} fromDLL;
		} sourceData;
	} sourceInfo;
	LPVOID* ppTrampl;
	LPVOID pDetour;
};

#define HH_DETOUR_FUNC(_func) Detour_##_func
#define HH_TRAMPL_FUNC(_func) Trampl_##_func

#define HH_FUNC_GLOBAL(_retType, _callConv, _func, ...) _retType(_callConv*HH_TRAMPL_FUNC(_func))(__VA_ARGS__)=_func; _retType _callConv HH_DETOUR_FUNC(_func)(__VA_ARGS__)
#define HH_FUNC_DLL(_retType, _callConv, _func, ...) _retType(_callConv*HH_TRAMPL_FUNC(_func))(__VA_ARGS__)=NULL; _retType _callConv HH_DETOUR_FUNC(_func)(__VA_ARGS__)

#define HH_INFO_GLOBAL(_func) {.sourceInfo = {.sourceType = HookInfo::SourceInfo::SourceType::FROM_GLOBAL_FUNCTION,\
	.sourceData = {.fromGlobalFunc = {}}},.ppTrampl = &(PVOID&)HH_TRAMPL_FUNC(_func),.pDetour = HH_DETOUR_FUNC(_func)}
#define HH_INFO_DLL(_func, _hModule, _decName) {.sourceInfo = {.sourceType = HookInfo::SourceInfo::SourceType::FROM_DLL,\
	.sourceData = {.fromDLL = {.phModule=&_hModule,.decoratedName=_decName}}},.ppTrampl = &(PVOID&)HH_TRAMPL_FUNC(_func),.pDetour = HH_DETOUR_FUNC(_func)}

void* getProcAddr(HMODULE hModule, LPCSTR lpProcName) {
	return  GetProcAddress(hModule, lpProcName);
}
template<size_t _len>
void GetProcAddrMulti(HookInfo(&info)[_len]) {
	for (auto& obj : info) {
		switch (obj.sourceInfo.sourceType)
		{
		case HookInfo::SourceInfo::SourceType::FROM_DLL:
			*obj.ppTrampl = GetProcAddress(*obj.sourceInfo.sourceData.fromDLL.phModule, obj.sourceInfo.sourceData.fromDLL.decoratedName);
			assert(*obj.ppTrampl);
			break;
		default:
			break;
		}
	}
}
template<size_t _len>
void DetourAttachMulti(HookInfo(&info)[_len]) {
	for (auto& obj : info) {
		DetourAttach(obj.ppTrampl, obj.pDetour);
	}
}
template<size_t _len>
void DetourDetachMulti(HookInfo(&info)[_len]) {
	for (auto& obj : info) {
		DetourDetach(obj.ppTrampl, obj.pDetour);
	}
}

template<size_t _len>
LONG DetourAttachAllInOne(HookInfo(&info)[_len]) {
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttachMulti(info);
	GetProcAddrMulti(info);
	return DetourTransactionCommit();
}

template<size_t _len>
LONG DetourDetachAllInOne(HookInfo(&info)[_len]) {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetachMulti(info);
	return DetourTransactionCommit();
}
