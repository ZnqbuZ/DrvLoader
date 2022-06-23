#include "SrvUtils.h"

using namespace DrvLoader;

std::unordered_map<DWORD, PCWSTR> SrvUtils::expected_err = {
	{ERROR_PATH_NOT_FOUND, TEXT("找不到服务二进制文件。")},
	{ERROR_ACCESS_DENIED, TEXT("拒绝访问。")},
	{ERROR_INVALID_NAME, TEXT("指定的服务名无效。")},
	{ERROR_BAD_EXE_FORMAT, TEXT("指定的二进制文件无效。")},
	{ERROR_DEPENDENT_SERVICES_RUNNING, TEXT("有依赖于此服务的组件正在运行。")},
	{ERROR_SERVICE_ALREADY_RUNNING, TEXT("服务的实例已在运行。")},
	{ERROR_SERVICE_DISABLED, TEXT("服务已被禁用。")},
	{ERROR_SERVICE_DOES_NOT_EXIST, TEXT("指定的服务不存在。")},
	{ERROR_SERVICE_CANNOT_ACCEPT_CTRL, TEXT("服务无法在此时接受控制信息。服务可能已停止/正在停止/正在启动。")},
	{ERROR_SERVICE_NOT_ACTIVE, TEXT("服务尚未启动。")},
	{ERROR_SERVICE_MARKED_FOR_DELETE, TEXT("服务已标记为删除。")},
	{ERROR_NO_MSG, TEXT("")}
};
std::unordered_map<DWORD, PCWSTR>::iterator SrvUtils::errIter;
std::unordered_map<PWSTR, SC_HANDLE> SrvUtils::hSrvMap;
std::unordered_map<PWSTR, SC_HANDLE>::iterator SrvUtils::srvIter;

SC_HANDLE SrvUtils::hSCManager;

STATUS SrvUtils::OpenSrv(SC_HANDLE& hService, PWSTR srvName, OPTIONAL ULONG Access = SERVICE_ALL_ACCESS)
{
	srvIter = hSrvMap.find(srvName);
	if (srvIter != hSrvMap.end())
	{
		hService = srvIter->second;
		return STATUS(ERROR_SUCCESS);
	}
	hService = OpenService(hSCManager, srvName, Access);
	if (hService == nullptr)
	{
		return STATUS(GetLastError(), TEXT("打开服务失败，"));
	}
	return STATUS(ERROR_SUCCESS);
}

VOID SrvUtils::DelHandle(const SC_HANDLE& hService)
{
	for (srvIter = hSrvMap.begin(); srvIter != hSrvMap.end(); ++srvIter)
	{
		if (srvIter->second == hService)
		{
			return;
		}
	}
	CloseServiceHandle(hService);
}

STATUS SrvUtils::OpenSCM()
{
	hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

	if (hSCManager == nullptr)
	{
		return STATUS(GetLastError(), TEXT("打开服务管理器失败，"));
	}

	return STATUS(ERROR_SUCCESS);
}

STATUS SrvUtils::Lookup(PWSTR srvName)
{
	DWORD errCode;
	SC_HANDLE hService;
	STATUS ret = OpenSrv(hService, srvName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
	if (!ret.Success())
	{
		return ret;
	}

	DWORD cbBytesNeeded = 0L;
	if (
		!QueryServiceConfig(
			hService,
			nullptr,
			0,
			&cbBytesNeeded))
	{
		errCode = GetLastError();
		if (errCode != ERROR_INSUFFICIENT_BUFFER)
		{
			DelHandle(hService);
			return STATUS(errCode, TEXT("获取信息失败，"));
		}
	}

	DWORD cbBufSize = cbBytesNeeded;
	const auto pSrvConf = static_cast<LPQUERY_SERVICE_CONFIG>(malloc(cbBufSize));
	if (pSrvConf == nullptr)
	{
		return STATUS(ERROR_UNKNOWN, TEXT("获取信息时内存申请失败，"));
	}

	if (!QueryServiceConfig(hService, pSrvConf, cbBufSize, &cbBytesNeeded))
	{
		free(pSrvConf);
		return STATUS(GetLastError(), TEXT("获取信息失败，"));
	}

	WCHAR errMsg[64];
	if (
		!QueryServiceConfig2(
			hService,
			SERVICE_CONFIG_DESCRIPTION,
			nullptr,
			0,
			&cbBytesNeeded))
	{
		errCode = GetLastError();
		if (errCode != ERROR_INSUFFICIENT_BUFFER)
		{
			wcscpy_s(errMsg, TEXT("获取描述失败，"));
			goto SET_DESC_FAILED_MSG;
		}
	}

	cbBufSize = max(cbBytesNeeded, 8);
	auto pSrvDesc = static_cast<LPSERVICE_DESCRIPTION>(malloc(cbBufSize));
	if (pSrvDesc == nullptr)
	{
		errCode = ERROR_UNKNOWN;
		wcscpy_s(errMsg, TEXT("获取描述信息时内存申请失败，"));
		goto SET_DESC_FAILED_MSG;
	}

	if (
		!QueryServiceConfig2(
			hService,
			SERVICE_CONFIG_DESCRIPTION,
			reinterpret_cast<LPBYTE>(pSrvDesc),
			cbBufSize,
			&cbBytesNeeded))
	{
		errCode = GetLastError();
		free(pSrvDesc);
		wcscpy_s(errMsg, TEXT("获取描述失败，"));
		goto SET_DESC_FAILED_MSG;
	}

	if (false)
	{
	SET_DESC_FAILED_MSG:
		pSrvDesc = static_cast<LPSERVICE_DESCRIPTION>(malloc(64 * sizeof(WCHAR)));
		pSrvDesc->lpDescription = reinterpret_cast<LPWSTR>(&pSrvDesc->lpDescription + 1);
		wcscpy_s(pSrvDesc->lpDescription, 63, errMsg);
		AppendErrInfo(pSrvDesc->lpDescription, errCode, TEXT(""));
	}

	if (
		!QueryServiceStatusEx(
			hService,
			SC_STATUS_PROCESS_INFO,
			nullptr,
			0,
			&cbBytesNeeded))
	{
		errCode = GetLastError();
		if (errCode != ERROR_INSUFFICIENT_BUFFER)
		{
			wcscpy_s(errMsg, TEXT("获取状态失败，"));
			goto SET_STATUS_FAILED_MSG;
		}
	}

	cbBufSize = cbBytesNeeded;
	auto pSrvStatus = static_cast<LPSERVICE_STATUS_PROCESS>(malloc(cbBufSize));
	if (pSrvStatus == nullptr)
	{
		errCode = ERROR_UNKNOWN;
		wcscpy_s(errMsg, TEXT("获取状态信息时内存申请失败，"));
		goto SET_STATUS_FAILED_MSG;
	}

	if (
		!QueryServiceStatusEx(
			hService,
			SC_STATUS_PROCESS_INFO,
			reinterpret_cast<LPBYTE>(pSrvStatus),
			cbBufSize,
			&cbBytesNeeded))
	{
		errCode = GetLastError();
		free(pSrvStatus);
		wcscpy_s(errMsg, TEXT("获取状态失败，"));
		goto SET_DESC_FAILED_MSG;
	}

	if (false)
	{
	SET_STATUS_FAILED_MSG:
		pSrvStatus = static_cast<LPSERVICE_STATUS_PROCESS>(malloc(sizeof(SERVICE_STATUS_PROCESS)));
		pSrvStatus->dwCurrentState = 0;
		pSrvStatus->dwProcessId = 0;
		AppendErrInfo(errMsg, errCode, TEXT(""));
	}

	WCHAR srvConf[MSG_BUF_SIZE];
	LPCSTR lpServiceType, lpStartType, lpErrorControl;
	LPCWSTR lpCurrentState;

	switch (pSrvConf->dwServiceType)
	{
	case SERVICE_FILE_SYSTEM_DRIVER:
		lpServiceType = "SERVICE_FILE_SYSTEM_DRIVER";
		break;
	case SERVICE_KERNEL_DRIVER:
		lpServiceType = "SERVICE_KERNEL_DRIVER";
		break;
	case SERVICE_WIN32_OWN_PROCESS:
		lpServiceType = "SERVICE_WIN32_OWN_PROCESS";
		break;
	case SERVICE_WIN32_SHARE_PROCESS:
		lpServiceType = "SERVICE_WIN32_SHARE_PROCESS";
		break;
	case SERVICE_INTERACTIVE_PROCESS:
		lpServiceType = "SERVICE_INTERACTIVE_PROCESS";
		break;
	default:
		lpServiceType = "UNKNOWN";
		break;
	}
	switch (pSrvConf->dwStartType)
	{
	case SERVICE_AUTO_START:
		lpStartType = "SERVICE_AUTO_START";
		break;
	case SERVICE_BOOT_START:
		lpStartType = "SERVICE_BOOT_START";
		break;
	case SERVICE_DEMAND_START:
		lpStartType = "SERVICE_DEMAND_START";
		break;
	case SERVICE_DISABLED:
		lpStartType = "SERVICE_DISABLED";
		break;
	case SERVICE_SYSTEM_START:
		lpStartType = "SERVICE_SYSTEM_START";
		break;
	default:
		lpStartType = "";
		break;
	}
	switch (pSrvConf->dwErrorControl)
	{
	case SERVICE_ERROR_CRITICAL:
		lpErrorControl = "SERVICE_ERROR_CRITICAL";
		break;
	case SERVICE_ERROR_IGNORE:
		lpErrorControl = "SERVICE_ERROR_IGNORE";
		break;
	case SERVICE_ERROR_NORMAL:
		lpErrorControl = "SERVICE_ERROR_NORMAL";
		break;
	case SERVICE_ERROR_SEVERE:
		lpErrorControl = "SERVICE_ERROR_SEVERE";
		break;
	default:
		lpErrorControl = "";
		break;
	}
	switch (pSrvStatus->dwCurrentState)
	{
	case SERVICE_CONTINUE_PENDING:
		lpCurrentState = TEXT("正在恢复");
		break;
	case SERVICE_PAUSE_PENDING:
		lpCurrentState = TEXT("正在暂停");
		break;
	case SERVICE_PAUSED:
		lpCurrentState = TEXT("已暂停");
		break;
	case SERVICE_RUNNING:
		lpCurrentState = TEXT("正在运行");
		break;
	case SERVICE_START_PENDING:
		lpCurrentState = TEXT("正在启动");
		break;
	case SERVICE_STOP_PENDING:
		lpCurrentState = TEXT("正在停止");
		break;
	case SERVICE_STOPPED:
		lpCurrentState = TEXT("已停止");
		break;
	default:
		lpCurrentState = errMsg;
		break;
	}

	for (PWCHAR ptr = pSrvConf->lpDependencies; ptr < pSrvConf->lpServiceStartName; ptr++)
	{
		if (*ptr == L'\0')
		{
			if (*(ptr + 1) != L'\0')
			{
				*ptr = L',';
			}
			else
			{
				break;
			}
		}
	}

	swprintf_s(
		srvConf,
		L"服务名称：%ls\r\n"
		L"显示名称：%ls\r\n"
		L"类型：%hs\r\n"
		L"描述：%ls\r\n"
		L"可执行文件的路径：%ls\r\n"
		L"启动类型：%hs\r\n"
		L"状态：%ls\r\n"
		L"PID：%d%hs\r\n"
		L"错误控制：%hs\r\n"
		L"登录身份：%ls\r\n"
		L"分组：%ls\r\n"
		L"组内标识：%d%hs\r\n"
		L"依赖项：%ls\r\n",
		srvName,
		pSrvConf->lpDisplayName,
		lpServiceType,
		pSrvDesc->lpDescription == nullptr ? L"" : pSrvDesc->lpDescription,
		pSrvConf->lpBinaryPathName,
		lpStartType,
		lpCurrentState,
		pSrvStatus->dwProcessId,
		pSrvStatus->dwProcessId == 0 ? " (N/A)" : "",
		lpErrorControl,
		pSrvConf->lpServiceStartName,
		pSrvConf->lpLoadOrderGroup == nullptr ? L"" : pSrvConf->lpLoadOrderGroup,
		pSrvConf->dwTagId,
		pSrvConf->lpLoadOrderGroup == nullptr ? " (N/A)" : "",
		pSrvConf->lpDependencies == nullptr ? L"" : pSrvConf->lpDependencies);

	return STATUS(ERROR_SUCCESS, srvConf);
}

STATUS SrvUtils::Create(PWSTR drvPath, PWSTR srvName)
{
	SC_HANDLE hService = CreateService(
		hSCManager,
		srvName,
		srvName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE,
		drvPath,
		nullptr, nullptr, nullptr, nullptr, nullptr);

	if (hService == nullptr)
	{
		const DWORD errCode = GetLastError();

		if (errCode == ERROR_SERVICE_EXISTS)
		{
			WCHAR msg[MSG_BUF_SIZE];
			wcscpy_s(msg, TEXT("创建服务失败，指定的服务名已被以下服务占用：\r\n"));
			wcsapp(msg, Lookup(srvName).Msg);
			return STATUS(ERROR_NO_MSG, msg);
		}

		return STATUS(GetLastError());
	}

	hSrvMap.insert(std::make_pair(srvName, hService));

	DelHandle(hService);
	return STATUS(ERROR_SUCCESS);
}

STATUS SrvUtils::Start(PWSTR srvName)
{
	SC_HANDLE hService;
	STATUS ret = OpenSrv(hService, srvName, SERVICE_START);
	if (!ret.Success())
	{
		return ret;
	}

	if (!StartService(hService, NULL, nullptr))
	{
		DelHandle(hService);
		return STATUS(GetLastError(), TEXT("启动失败，"));
	}

	DelHandle(hService);
	return STATUS(ERROR_SUCCESS);
}

STATUS SrvUtils::Stop(PWSTR srvName, OPTIONAL BOOL force)
{
	SC_HANDLE hService;
	STATUS ret = OpenSrv(hService, srvName, SERVICE_STOP);
	if (!ret.Success())
	{
		return ret;
	}

	SERVICE_STATUS srvStatus = {0};

	if (!ControlService(hService, SERVICE_CONTROL_STOP, &srvStatus))
	{
		DelHandle(hService);
		return STATUS(GetLastError(), TEXT("停止失败，"));
	}

	DelHandle(hService);
	return STATUS(ERROR_SUCCESS);
}

STATUS SrvUtils::Delete(PWSTR srvName, OPTIONAL BOOL force)
{
	SC_HANDLE hService;
	STATUS ret = OpenSrv(hService, srvName, DELETE);
	if (!ret.Success())
	{
		return ret;
	}

	if (!force && hSrvMap.find(srvName) == hSrvMap.end())
	{
		return STATUS(ERROR_NEED_CONFIRM, TEXT("删除操作需要确认。"));
	}

	if (!DeleteService(hService))
	{
		DelHandle(hService);
		return STATUS(GetLastError(), TEXT("删除失败，"));
	}
	if (hSrvMap.find(srvName) != hSrvMap.end())
	{
		hSrvMap.erase(srvName);
	}
	DelHandle(hService);
	return STATUS(ERROR_SUCCESS);
}

STATUS SrvUtils::Clear()
{
	SERVICE_STATUS srvStatus = {0};
	for (srvIter = hSrvMap.begin(); srvIter != hSrvMap.end(); ++srvIter)
	{
		ControlService(srvIter->second, SERVICE_CONTROL_STOP, &srvStatus);
		DeleteService(srvIter->second);
		CloseServiceHandle(srvIter->second);
	}
	CloseServiceHandle(hSCManager);
	hSrvMap.clear();

	return STATUS(ERROR_SUCCESS);
}
