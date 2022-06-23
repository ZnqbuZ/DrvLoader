#pragma once
#include "windows.h"
#include "winerror.h"

#define ERROR_UNKNOWN                      99999L
#define ERROR_NO_MSG                       10001L
#define ERROR_NEED_CONFIRM                 10002L
/*
* * Already defined in winerror.h
**
#define ERROR_SUCCESS                          0L
#define ERROR_PATH_NOT_FOUND                   3L
#define ERROR_ACCESS_DENIED                    5L
#define ERROR_INVALID_NAME                   123L
#define ERROR_BAD_EXE_FORMAT                 193L
#define ERROR_SERVICE_ALREADY_RUNNING       1056L
#define ERROR_SERVICE_DISABLED              1058L
#define ERROR_SERVICE_DOES_NOT_EXIST        1060L
#define ERROR_SERVICE_CANNOT_ACCEPT_CTRL    1061L
#define ERROR_SERVICE_NOT_ACTIVE            1062L
#define ERROR_SERVICE_MARKED_FOR_DELETE     1072L
#define ERROR_SERVICE_EXISTS                1073L
*/

#define __CONCAT(x, y) x##y
#define CONCAT(x, y) __CONCAT(x, y)

#ifdef _DEBUG
#define DebugLog(msg, ...)                                              \
    do                                                                  \
    {                                                                   \
        WCHAR CONCAT(__msg_, __LINE__)[MSG_BUF_SIZE];                   \
        swprintf_s(CONCAT(__msg_, __LINE__), TEXT(msg), ##__VA_ARGS__); \
        OutputDebugString(CONCAT(__msg_, __LINE__));                    \
    } while (0)
#else
#define DebugLog
#endif

#define MSG_BUF_SIZE (size_t)1024

namespace DrvLoader
{
#pragma region wcsapp

	template <typename P, typename Q, typename...R>
	inline void wcsapp(P p, Q q, R ...rs)
	{
		static_assert(0, "[wcsapp] Argument type mismatch.");
	}

	template <size_t __Size, typename...R>
	inline void wcsapp(WCHAR (&_Destination)[__Size], PCWSTR _Source, R ..._res)
	{
		wcscat_s(_Destination, _Source);
		wcsapp(_Destination, _res...);
	}

	template <size_t __Size, typename...R>
	inline void wcsapp(WCHAR (&_Destination)[__Size], PWSTR _Source, R ..._res)
	{
		wcscat_s(_Destination, _Source);
		wcsapp(_Destination, _res...);
	}

	template <size_t __Size>
	inline void wcsapp(WCHAR (&_Destination)[__Size], PCWSTR _Source)
	{
		wcscat_s(_Destination, _Source);
	}

	template <size_t __Size>
	inline void wcsapp(WCHAR (&_Destination)[__Size], PWSTR _Source)
	{
		wcscat_s(_Destination, _Source);
	}

	template <typename...R>
	inline void wcsapp(PWSTR (&_Destination), size_t _SizeInWords, PCWSTR _Source, R ..._res)
	{
		wcscat_s(_Destination, _SizeInWords, _Source);
		wcsapp(_Destination, _SizeInWords - wcslen(_Source), _res...);
	}

	template <typename...R>
	inline void wcsapp(PWSTR (&_Destination), size_t _SizeInWords, PWSTR _Source, R ..._res)
	{
		wcscat_s(_Destination, _SizeInWords, _Source);
		wcsapp(_Destination, _SizeInWords - wcslen(_Source), _res...);
	}

	template <>
	inline void wcsapp(PWSTR (&_Destination), size_t _SizeInWords, PCWSTR _Source)
	{
		wcscat_s(_Destination, _SizeInWords, _Source);
	}

	template <>
	inline void wcsapp(PWSTR (&_Destination), size_t _SizeInWords, PWSTR _Source)
	{
		wcscat_s(_Destination, _SizeInWords, _Source);
	}

#pragma endregion

	inline void AppendErrInfo(PWSTR msg, DWORD errCode, PCWSTR end = TEXT("\r\n"));

	struct STATUS
	{
		DWORD exitCode;
		WCHAR Msg[MSG_BUF_SIZE] = TEXT("");

		STATUS(DWORD exitCode, OPTIONAL PCWSTR msg = TEXT(""));
		~STATUS();
		BOOL Success() const { return exitCode == ERROR_SUCCESS; }
	};
}
