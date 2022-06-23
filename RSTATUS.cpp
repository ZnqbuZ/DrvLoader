#include "RSTATUS.h"

using namespace DrvLoader;

RSTATUS::RSTATUS(const STATUS& source)
{
	exitCode = source.exitCode;
	msg = gcnew System::String(source.Msg);
	DebugLog("--------------------------------------------------------------\n");
	DebugLog("RSTATUS constructed by: 0x%p. Loaction: 0x%p\n", &source, this);
	DebugLog("--------------------------------------------------------------\n");
}

RSTATUS::~RSTATUS()
{
	DebugLog("--------------------------------------------------------------\n");
	DebugLog("RSTATUS destructed. Location: 0x%p\n", this);
	DebugLog("--------------------------------------------------------------\n");
	delete msg;
}

RSTATUS::operator String^()
{
	return msg;
}

bool RSTATUS::Success::get()
{
	return exitCode == ERROR_SUCCESS;
}

DWORD RSTATUS::ExitCode::get()
{
	return exitCode;
}
