#pragma once
#include "SrvUtils.h"

// 最大路径长度
#define __MAX_PATH__ 32768L
// 最大服务名长度
#define __MAX_NAME__ 256L

#define ThrowIfFailed(x)                         \
    do{                                          \
        RSTATUS^ CONCAT(__ret_, __LINE__) = (gcnew RSTATUS(x));   \
        if (!CONCAT(__ret_, __LINE__)->Success) \
        {                                        \
            throw CONCAT(__ret_, __LINE__);      \
        }                                        \
    delete CONCAT(__ret_, __LINE__);            \
    CONCAT(__ret_, __LINE__) = nullptr;         \
    }while(0)

#define CatchDisplay(...)  \
  catch (RSTATUS^ ret)     \
  {                        \
    DisplayException(ret); \
    __VA_ARGS__            \
    delete ret;            \
    ret = nullptr;         \
  }                        \
  do                       \
  {                        \
  } while (0) // 保证末尾需要分号

#define __LEFT_BRACKET (
#define __RIGHT_BRACKET )

#define __DEL_COMMA(...) , ##__VA_ARGS__

#define __SELECT_ARG32(                     \
	_0, _1, _2, _3, _4, _5, _6, _7,         \
	_8, _9, _10, _11, _12, _13, _14, _15,   \
	_16, _17, _18, _19, _20, _21, _22, _23, \
	_24, _25, _26, _27, _28, _29, _30, _31, ...) _31

#define __SELECT_FUN(_0, _1, ...)       \
    __SELECT_ARG32 __LEFT_BRACKET       \
        __DEL_COMMA(__VA_ARGS__),       \
        _1, _1, _1, _1, _1, _1, _1, _1, \
        _1, _1, _1, _1, _1, _1, _1, _1, \
        _1, _1, _1, _1, _1, _1, _1, _1, \
        _1, _1, _1, _1, _1, _1, _0,     \
        __RIGHT_BRACKET

#define __LogTo0(x, msg) (x->AppendText(msg))
#define __LogTo1(x, msg, ...) (x->AppendText(System::String::Format(msg, __VA_ARGS__)))
#define Log(msg, ...)                             \
    __SELECT_FUN(__LogTo0, __LogTo1, __VA_ARGS__) \
    (txtLog, msg, __VA_ARGS__)

namespace DrvLoader
{
	using System::String;

	ref class RSTATUS
	{
	public:
		RSTATUS(const STATUS& source);
		~RSTATUS();
		property bool Success
		{
			bool get();
		};
		property DWORD ExitCode
		{
			DWORD get();
		};
		operator String^();
	private:
		DWORD exitCode;
		String^ msg;
	};
}
