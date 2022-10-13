#include "STATUS.h"
#include "SrvUtils.h"

using namespace DrvLoader;

inline void DrvLoader::AppendErrInfo(PWSTR msg, DWORD errCode, PCWSTR end)
{
	SrvUtils::errIter = SrvUtils::expected_err.find(errCode);
	if (SrvUtils::errIter != SrvUtils::expected_err.end())
	{
		if (SrvUtils::errIter->first != ERROR_NO_MSG)
		{
			wcsapp(msg, MSG_BUF_SIZE, SrvUtils::errIter->second, end);
		}
	}
	else
	{
		wcsapp(msg, MSG_BUF_SIZE, TEXT("未知错误。"), end);
	}
}

STATUS::STATUS(DWORD exitCode, OPTIONAL PCWSTR msg)
{
	this->exitCode = exitCode;
	wcscpy_s(this->Msg, msg);
	if (!this->Success())
	{
		AppendErrInfo(this->Msg, exitCode);
	}
	DebugLog("--------------------------------------------------------------\n");
	DebugLog("STATUS constructed. Location: 0x%p\n", this);
	DebugLog("--------------------------------------------------------------\n");
}

STATUS::~STATUS()
{
	DebugLog("--------------------------------------------------------------\n");
	DebugLog("STATUS destructed. Location: 0x%p\n", this);
	DebugLog("--------------------------------------------------------------\n");
}
