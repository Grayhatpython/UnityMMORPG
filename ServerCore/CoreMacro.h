#pragma once
#include <Windows.h>

#define OUT

#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define	WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx]);
#define WRITE_LOCK				WRITE_LOCK_IDX(0)


#define _STRING2(x) #x
#define _STRING(x)  _STRING2(x)
#define JUMP_MSG(desc) message(__FILE__ "(" _STRING(__LINE__) "):" #desc)

inline void MsgBox(PCSTR msg) {
	char str[MAX_PATH];
	::GetModuleFileNameA(NULL, str, _countof(str));
	::MessageBoxA(::GetActiveWindow(), msg, str, MB_OK);
}

inline void Fail(PSTR message) {
	MsgBox(message);
	::DebugBreak();
}

inline void AssertFail(LPCSTR file, int line, PCSTR cause) {
	char str[2 * MAX_PATH];
	::wsprintfA(str, "File %s, line %d : %s\n", file, line, cause);
	::OutputDebugStringA(str);
	Fail(str);
}

#ifdef _DEBUG
#define ASSERT_CRASH(cause) if (!(cause)) AssertFail(__FILE__, __LINE__, #cause)
#else
#define ASSERT_CRASH(cause)			\
{									\
	if(!(cause))					\
	{								\
		int* crashVal = nullptr;	\
		*crashVal = 0xDEADBEEF;		\
	}								\
}

#endif


