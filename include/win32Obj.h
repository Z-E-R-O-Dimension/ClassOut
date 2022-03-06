#pragma once
// the inter-process win32 object library

#include <Windows.h>
#include <iostream>
#include <initializer_list>
#include <utility>

#pragma warning(disable:26812)

///
///   +-----------------------------------------+
///   |                                         |
///   |           win32Obj exceptions           |
///   |                                         |
///   +-----------------------------------------+
///

//                                   +32bits+-32bits-
#define HIDWORD(dword64) ((DWORD)((0xffffffff00000000&dword64)>>32))
#define LODWORD(dword64) ((DWORD)((0x00000000ffffffff&dword64)))

#pragma region Dull_Stuff
enum win32WaitResult :DWORD {
	WaitObject_0 = WAIT_OBJECT_0,
	WaitObject_1, WaitObject_2, WaitObject_3, WaitObject_4, WaitObject_5,
	WaitObject_6, WaitObject_7, WaitObject_8, WaitObject_9, WaitObject_10,
	WaitObject_11, WaitObject_12, WaitObject_13, WaitObject_14, WaitObject_15,
	WaitObject_16, WaitObject_17, WaitObject_18, WaitObject_19, WaitObject_20,
	WaitObject_21, WaitObject_22, WaitObject_23, WaitObject_24, WaitObject_25,
	WaitObject_26, WaitObject_27, WaitObject_28, WaitObject_29, WaitObject_30,
	WaitObject_31, WaitObject_32, WaitObject_33, WaitObject_34, WaitObject_35,
	WaitObject_36, WaitObject_37, WaitObject_38, WaitObject_39, WaitObject_40,
	WaitObject_41, WaitObject_42, WaitObject_43, WaitObject_44, WaitObject_45,
	WaitObject_46, WaitObject_47, WaitObject_48, WaitObject_49, WaitObject_50,
	WaitObject_51, WaitObject_52, WaitObject_53, WaitObject_54, WaitObject_55,
	WaitObject_56, WaitObject_57, WaitObject_58, WaitObject_59, WaitObject_60,
	WaitObject_61, WaitObject_62, WaitObject_63,
	WaitAbandoned_0 = WAIT_ABANDONED_0,
	WaitAbandoned_1, WaitAbandoned_2, WaitAbandoned_3, WaitAbandoned_4, WaitAbandoned_5,
	WaitAbandoned_6, WaitAbandoned_7, WaitAbandoned_8, WaitAbandoned_9, WaitAbandoned_10,
	WaitAbandoned_11, WaitAbandoned_12, WaitAbandoned_13, WaitAbandoned_14, WaitAbandoned_15,
	WaitAbandoned_16, WaitAbandoned_17, WaitAbandoned_18, WaitAbandoned_19, WaitAbandoned_20,
	WaitAbandoned_21, WaitAbandoned_22, WaitAbandoned_23, WaitAbandoned_24, WaitAbandoned_25,
	WaitAbandoned_26, WaitAbandoned_27, WaitAbandoned_28, WaitAbandoned_29, WaitAbandoned_30,
	WaitAbandoned_31, WaitAbandoned_32, WaitAbandoned_33, WaitAbandoned_34, WaitAbandoned_35,
	WaitAbandoned_36, WaitAbandoned_37, WaitAbandoned_38, WaitAbandoned_39, WaitAbandoned_40,
	WaitAbandoned_41, WaitAbandoned_42, WaitAbandoned_43, WaitAbandoned_44, WaitAbandoned_45,
	WaitAbandoned_46, WaitAbandoned_47, WaitAbandoned_48, WaitAbandoned_49, WaitAbandoned_50,
	WaitAbandoned_51, WaitAbandoned_52, WaitAbandoned_53, WaitAbandoned_54, WaitAbandoned_55,
	WaitAbandoned_56, WaitAbandoned_57, WaitAbandoned_58, WaitAbandoned_59, WaitAbandoned_60,
	WaitAbandoned_61, WaitAbandoned_62, WaitAbandoned_63,
	WaitTimeout = WAIT_TIMEOUT,
	WaitFailed = WAIT_FAILED,
};
#pragma endregion

#pragma region Win32Obj Exceptions
class win32ObjExceptionBase {
public:
	win32ObjExceptionBase() :dwLastError(GetLastError()) {}
	virtual const char* what() {
		return "win32ObjExceptionBase";
	}
	DWORD code() {
		return dwLastError;
	}
private:
	DWORD dwLastError;
};

class win32ObjExInvalidObject :win32ObjExceptionBase {
public:
	virtual const char* what() {
		return "Invalid Object";
	}
};
class win32ObjExWaitAbandoned :win32ObjExceptionBase {
public:
	virtual const char* what() {
		return "Mutex was destroyed before released";
	}
};
class win32ObjExOperationError :win32ObjExceptionBase {
public:
	virtual const char* what() {
		return "Operation error";
	}
};
class win32ObjExWaitError :win32ObjExceptionBase {
public:
	win32ObjExWaitError(win32WaitResult waitResult) :wwaitResult(waitResult) {}
	virtual const char* what() {
		return "Wait error";
	}
	DWORD waitResult() {
		return this->wwaitResult;
	}
private:
	win32WaitResult wwaitResult;
};
class win32ObjExCreationFault :win32ObjExceptionBase {
public:
	virtual const char* what() {
		return "Object Creation Failed";
	}
};
#pragma endregion

///
///   +-----------------------------------------+
///   |                                         |
///   |             win32Obj classes            |
///   |                                         |
///   +-----------------------------------------+
///

// win32ObjWithName                    win32Waitable <- win32ObjBase
//      |                                   |                |
//      +--------> win32Mutex <-------------+                |
//      +--------> win32Event <-------------+                |
//      |          win32Thread <------------+                |
//      +--------> win32SharedMemory <-----------------------+

#pragma region Base Classes
class win32ObjBase {
public:
	win32ObjBase(const win32ObjBase&) = delete;
	win32ObjBase(LPSECURITY_ATTRIBUTES nSecAttr) :pSecAttr(nSecAttr) {}
	~win32ObjBase() {
		this->bGood = false;
		if (this->handleValValid())
			CloseHandle(this->hThis);
	}
	bool good() {
		DWORD dwHandleInf = 0;
		if (this->bGood)
			this->bGood = GetHandleInformation(this->hThis, &dwHandleInf);
		return this->bGood;
	}
	operator bool() {
		return this->good();
	}
protected:
	static bool handleValValid(const HANDLE handle) {
		return !(handle == INVALID_HANDLE_VALUE || handle == nullptr);
	}
	bool handleValValid() {
		return win32ObjBase::handleValValid(this->hThis);
	}
	bool bGood = false;
	HANDLE hThis = INVALID_HANDLE_VALUE;
	LPSECURITY_ATTRIBUTES pSecAttr = nullptr;
};

class win32Waitable :public win32ObjBase {
public:
	win32Waitable() :win32ObjBase(nullptr) {}
	win32Waitable(LPSECURITY_ATTRIBUTES nSecAttr) :win32ObjBase(nSecAttr) {}
	template<typename... Ts>
	static win32WaitResult waitForMultiple(bool waitAll, DWORD timeout, const  Ts&... args) {
		static_assert(sizeof...(args) < MAXIMUM_WAIT_OBJECTS, "object exceeded maximum");
		const HANDLE lpHandles[sizeof...(args)]{ args.toHandle()... };
		win32WaitResult waitResult = (win32WaitResult)WaitForMultipleObjects(sizeof...(args), lpHandles, waitAll, timeout);
		return waitResult;
	}
	static win32WaitResult waitForSingle(DWORD timeout, const win32Waitable& obj) {
		return (win32WaitResult)WaitForSingleObject(obj.toHandle(), timeout);
	}
	bool wait(DWORD timeout = INFINITE) {
		if (!this->bGood) {
			throw win32ObjExInvalidObject();
		}
		win32WaitResult waitResult = win32Waitable::waitForSingle(timeout, *this);
		switch (waitResult) {
		case WaitObject_0:
			break;
		case WaitTimeout:
			return false;
			break;
		case WaitAbandoned_0:
			this->bGood = false;
			throw win32ObjExWaitAbandoned();
			break;
		default:
			this->bGood = true;
			throw win32ObjExWaitError(waitResult);
		}
		return true;
	}
private:
	const HANDLE toHandle()const {
		return this->hThis;
	}
};

class win32ObjWithName {
public:
	win32ObjWithName() :strName(nullptr) {}
	win32ObjWithName(win32ObjWithName&) = delete;
	win32ObjWithName(LPCWSTR name) {
		// make name
		if (name) {
			strName = new WCHAR[MAX_PATH]{ 0 };
			wcscat_s(this->strName, MAX_PATH, name);
		}
		else {
			//no name
			this->strName = nullptr;
		}
	}
	~win32ObjWithName() {
		if (this->strName)
			delete[] this->strName;
	}

protected:
	void regName(LPCWSTR name) {
		// make name
		if (this->strName) {
			delete[] this->strName;
		}
		if (name) {
			strName = new WCHAR[MAX_PATH]{ 0 };
			wcscat_s(this->strName, MAX_PATH, name);
		}
		else {
			//no name
			this->strName = nullptr;
		}
	}
	LPWSTR strName = nullptr;
};
#pragma endregion

#pragma region Win32Obj Classes
class win32Mutex :public win32ObjWithName, public win32Waitable {
public:
	win32Mutex() :win32Waitable(), win32ObjWithName() {}
	/// <summary>
	/// constructor of win32Mutex
	/// </summary>
	/// <param name="name">.name of the mutex</param>
	/// <param name="secAttr">.security attributes</param>
	win32Mutex(LPCWSTR name, LPSECURITY_ATTRIBUTES secAttr = nullptr) :win32Waitable(secAttr), win32ObjWithName(name) {}
	bool reg(LPCWSTR name, LPSECURITY_ATTRIBUTES secAttr = nullptr) {
		if (this->bGood) {
			return false;
		}
		this->pSecAttr = secAttr;
		this->regName(name);
		return true;
	}
	bool open() {
		this->hThis = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, this->strName);
		this->bGood = this->handleValValid();
		return this->bGood;
	}
	bool createOnly(bool bInitialOwner = false) {
		this->hThis = CreateMutexW(this->pSecAttr, bInitialOwner, this->strName);
		if (GetLastError() == ERROR_ALREADY_EXISTS && this->handleValValid()) {
			// creation failed
			CloseHandle(this->hThis);
			this->hThis = INVALID_HANDLE_VALUE;
			this->bGood = false;
		}
		else {
			// succeed
			this->bGood = true;
		}
		return this->bGood;
	}
	bool create(bool bInitialOwner = false) {
		this->hThis = CreateMutexW(this->pSecAttr, bInitialOwner, this->strName);
		this->bGood = this->handleValValid();
		return this->bGood && GetLastError() != ERROR_ALREADY_EXISTS;
	}
	~win32Mutex() {}
	bool lock(DWORD timeout = INFINITE) {
		return  this->wait(timeout);
	}
	void unlock() {
		if (!this->bGood) {
			throw win32ObjExInvalidObject();
		}
		if (!ReleaseMutex(this->hThis)) {
			throw win32ObjExOperationError();
		}
	}

private:
};

class win32Event :public win32ObjWithName, public win32Waitable {
public:
	win32Event() :win32Waitable(), win32ObjWithName() {}
	/// <summary>
	/// constructor of win32Event
	/// </summary>
	/// <param name="name">.name of event object</param>
	/// <param name="secAttr">.security attributes</param>
	win32Event(LPCWSTR name, LPSECURITY_ATTRIBUTES secAttr = nullptr) : win32Waitable(secAttr), win32ObjWithName(name) {}
	bool reg(LPCWSTR name, LPSECURITY_ATTRIBUTES secAttr = nullptr) {
		if (this->bGood) {
			return false;
		}
		this->pSecAttr = secAttr;
		this->regName(name);
		return true;
	}
	bool create(BOOL bManualReset = FALSE, BOOL bInitialState = FALSE) {
		this->hThis = CreateEventW(this->pSecAttr, bManualReset, bInitialState, this->strName);
		this->bGood = this->handleValValid();
		return this->bGood && GetLastError() != ERROR_ALREADY_EXISTS;
	}
	bool createOnly(BOOL bManualReset = FALSE, BOOL bInitialState = FALSE) {
		this->hThis = CreateEventW(this->pSecAttr, bManualReset, bInitialState, this->strName);
		if (GetLastError() == ERROR_ALREADY_EXISTS && this->handleValValid()) {
			// creation failed
			CloseHandle(this->hThis);
			this->hThis = INVALID_HANDLE_VALUE;
			this->bGood = false;
		}
		else {
			// succeed
			this->bGood = true;
		}
		return this->bGood;
	}

	bool open() {
		if (this->strName)
			this->hThis = OpenEventW(EVENT_ALL_ACCESS, FALSE, this->strName);
		this->bGood = this->handleValValid();
		return this->bGood;
	}
	bool waitForSingal(DWORD timeout = INFINITE) {
		return wait(timeout);
	}
	void set() {
		if (!this->bGood) {
			throw win32ObjExInvalidObject();
		}
		if (!SetEvent(this->hThis)) {
			throw win32ObjExOperationError();
		}
	}
	void reset() {
		if (!this->bGood) {
			throw win32ObjExInvalidObject();
		}
		if (!ResetEvent(this->hThis)) {
			throw win32ObjExOperationError();
		}
	}
	void pulse() {
		if (!this->bGood) {
			throw win32ObjExInvalidObject();
		}
		if (!PulseEvent(this->hThis)) {
			throw win32ObjExOperationError();
		}
	}
private:

};

class win32SharedMemory :public win32ObjWithName, public win32ObjBase {
public:
	win32SharedMemory() :win32ObjWithName(nullptr), win32ObjBase(nullptr), szSharedMem(0) {}
	win32SharedMemory(LPCWSTR name, LPSECURITY_ATTRIBUTES secAttr = nullptr) :
		win32ObjWithName(name), win32ObjBase(secAttr), szSharedMem(0) {}
	bool reg(LPCWSTR name, LPSECURITY_ATTRIBUTES secAttr = nullptr) {
		if (this->bGood) {
			return false;
		}
		this->regName(name);
		this->pSecAttr = secAttr;
		return true;
	}
	bool open() {
		if (this->strName)
			this->hThis = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, this->strName);
		this->bGood = this->handleValValid();
		return this->bGood;
	}
	bool createOnly(DWORD64 size) {
		if (size > 0)
			this->szSharedMem = size;
		else
			return false;
		this->hThis = CreateFileMappingW(
			INVALID_HANDLE_VALUE,
			this->pSecAttr,
			PAGE_READWRITE,
			HIDWORD(this->szSharedMem),
			LODWORD(this->szSharedMem),
			this->strName
		);
		if (!this->handleValValid() || GetLastError() == ERROR_ALREADY_EXISTS) {
			CloseHandle(this->hThis);
			this->bGood = false;
		}
		else {
			this->bGood = true;
		}
		return this->bGood;
	}
	bool create(DWORD64 size) {
		if (size > 0)
			this->szSharedMem = size;
		else
			return false;
		this->hThis = CreateFileMappingW(
			INVALID_HANDLE_VALUE,
			this->pSecAttr,
			PAGE_READWRITE,
			HIDWORD(this->szSharedMem),
			LODWORD(this->szSharedMem),
			this->strName
		);
		this->bGood = this->handleValValid();
		return this->bGood&&GetLastError()!=ERROR_ALREADY_EXISTS;
	}
	void* map() {
		if (!this->bGood) {
			throw win32ObjExInvalidObject();
			return nullptr;
		}
		// unmap if mapped
		if (this->pBaseMapFile)
			UnmapViewOfFile(this->pBaseMapFile);
		this->pBaseMapFile = nullptr;
		this->pBaseMapFile = MapViewOfFile(this->hThis, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		// map error
		if (this->pBaseMapFile == nullptr) {
			this->bGood = false;
			throw win32ObjExInvalidObject();
			return nullptr;
		}
		// get sm size
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(this->pBaseMapFile, &mbi, sizeof(mbi));
		this->szSharedMem = mbi.RegionSize;
		return this->pBaseMapFile;
	}
	bool unmap() {
		if (!this->bGood) {
			throw win32ObjExInvalidObject();
		}
		if (this->pBaseMapFile) {
			if (UnmapViewOfFile(this->pBaseMapFile)) {
				this->pBaseMapFile = nullptr;
				return true;
			}
			else {
				throw win32ObjExOperationError();
			}
		}
		return false;
	}
	DWORD64 getSize() {
		return this->szSharedMem;
	}
	void* getBase() {
		if (this->pBaseMapFile) {
			return this->pBaseMapFile;
		}
		else {
			throw win32ObjExInvalidObject();
		}
	}
private:
	DWORD64 szSharedMem = 0;
	void* pBaseMapFile = nullptr;
};

class win32Thread : public win32Waitable {
public:
	using fnThreadCallback = LPTHREAD_START_ROUTINE;
	win32Thread() = delete;
	win32Thread(const win32Thread&) = delete;
	win32Thread(fnThreadCallback lpFn, size_t stackSize = 0, void* parameter = nullptr, LPSECURITY_ATTRIBUTES secAttr = nullptr)
		:fnCb(lpFn), threadID(0), lpParam(parameter), win32Waitable(secAttr) {
		this->hThis = CreateThread(this->pSecAttr, stackSize, lpFn, parameter, CREATE_SUSPENDED, &this->threadID);
		this->bGood = this->handleValValid();
	}
	bool detach() {
		if (this->bGood) {
			this->bGood = (ResumeThread(this->hThis) != (-1));
		}
		return this->bGood;
	}
	bool join() {
		bool ret = this->detach();
		if (ret)
			this->wait();
		return ret;
	}
	bool suspend() {
		if (this->bGood) {
			this->bGood = (SuspendThread(this->hThis) != (-1));
		}
		return this->bGood;
	}
	DWORD getID() {
		return this->threadID;
	}
protected:
	fnThreadCallback fnCb;
	DWORD threadID;
	void* lpParam;
};
#pragma endregion

class spinLock {
public:
	spinLock() :val(0) {};
	void lock() {
		while (InterlockedCompareExchange(&this->val, 1, 0) != 0);
	}
	void unlock() {
		WriteRelease(&this->val, 0);
	}
protected:
	LONG val;
};

class srwLock {
	srwLock() :nReaders(0), nWriters(0) {};
	void readLock() {

	}
	void readUnlock() {

	}
	void writeLock() {

	}
	void writeUnlock() {

	}
protected:
	LONG nReaders, nWriters;
};

// wait for multiple object - returns when the state of [any one of the objects] is set to signaled 
template<typename... Ts>
win32WaitResult waitFor(DWORD timeout, const Ts&... args) {
	return win32Waitable::waitForMultiple(false, timeout, args...);
}

// wait for multiple object - returns when the state of [all objects] is signaled 
template<typename... Ts>
win32WaitResult waitForAll(DWORD timeout, const Ts&... args) {
	return win32Waitable::waitForMultiple(true, timeout, args...);
}

// wait for single object
win32WaitResult waitFor(DWORD timeout, const win32Waitable& obj) {
	return win32Waitable::waitForSingle(timeout, obj);
}

