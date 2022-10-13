#pragma once

#include "windows.h"
#include "stdio.h"
#include "wchar.h"
#include "string.h"
#include "unordered_map"
#include "STATUS.h"

namespace DrvLoader
{
    class SrvUtils
    {
    public:
        static STATUS OpenSCM();
        static STATUS Lookup(PWSTR srvName);
        static STATUS Create(PWSTR drvPath, PWSTR srvName);
        static STATUS Start(PWSTR srvName);
        static STATUS Stop(PWSTR srvName, OPTIONAL BOOL force = FALSE);
        static STATUS Delete(PWSTR srvName, OPTIONAL BOOL force = FALSE);
        static STATUS Clear();

    private:
        static STATUS OpenSrv(SC_HANDLE& phService, PWSTR srvName, OPTIONAL ULONG Access);
        static VOID DelHandle(const SC_HANDLE& hService);

        static SC_HANDLE hSCManager;
        static std::unordered_map<PWSTR, SC_HANDLE> hSrvMap;
        static std::unordered_map<PWSTR, SC_HANDLE>::iterator srvIter;
        static std::unordered_map<DWORD, PCWSTR> expected_err;
        static std::unordered_map<DWORD, PCWSTR>::iterator errIter;

        friend inline void AppendErrInfo(PWSTR msg, DWORD errCode, PCWSTR end);
    };
}
