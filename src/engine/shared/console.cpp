/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>

#include <base/math.h>
#include <base/system.h>

#include <engine/storage.h>
#include <engine/shared/protocol.h>

#include "config.h"
#include "console.h"
#include "linereader.h"

// todo: rework this

const char *CConsole::CResult::GetString(unsigned Index)
{
	return m_apArgs[Index];
}

int CConsole::CResult::GetInteger(unsigned Index)
{
	return str_toint(m_apArgs[Index]);
}

float CConsole::CResult::GetFloat(unsigned Index)
{
	return str_tofloat(m_apArgs[Index]);
}

const IConsole::CCommandInfo *CConsole::CCommand::NextCommandInfo(int AccessLevel, int FlagMask) const
{
	return 0;
}

const IConsole::CCommandInfo *CConsole::FirstCommandInfo(int AccessLevel, int FlagMask) const
{
	return 0;
}

int CConsole::ParseStart(CResult *pResult, const char *pString, int Length)
{
	return 0;
}

bool CConsole::ArgStringIsValid(const char *pFormat)
{
	return false;
}

int CConsole::ParseArgs(CResult *pResult, const char *pFormat)
{
	return 0;
}

bool CConsole::NextParam(char *pNext, const char *&pFormat)
{
	return false;
}

int CConsole::ParseCommandArgs(const char *pArgs, const char *pFormat, FCommandCallback pfnCallback, void *pContext)
{
	return 0;
}

int CConsole::RegisterPrintCallback(int OutputLevel, FPrintCallback pfnPrintCallback, void *pUserData)
{
	return 0;
}

void CConsole::SetPrintOutputLevel(int Index, int OutputLevel)
{
}

void CConsole::Print(int Level, const char *pFrom, const char *pStr, bool Highlighted)
{
	dbg_msg(pFrom, "%s", pStr);
}

bool CConsole::LineIsValid(const char *pStr)
{
	return true;
}

void CConsole::ExecuteLineStroked(int Stroke, const char *pStr)
{
}

int CConsole::PossibleCommands(const char *pStr, int FlagMask, bool Temp, FPossibleCallback pfnCallback, void *pUser)
{
	return 0;
}

int CConsole::PossibleMaps(const char *pStr, FPossibleCallback pfnCallback, void *pUser)
{
	return 0;
}

CConsole::CCommand *CConsole::FindCommand(const char *pName, int FlagMask)
{
	return 0x0;
}

void CConsole::ExecuteLine(const char *pStr)
{
}

void CConsole::ExecuteLineFlag(const char *pStr, int FlagMask)
{
}


bool CConsole::ExecuteFile(const char *pFilename)
{
	return false;
}

void CConsole::Con_Echo(IResult *pResult, void *pUserData)
{
}

void CConsole::Con_Exec(IResult *pResult, void *pUserData)
{
}

void CConsole::ConModCommandAccess(IResult *pResult, void *pUser)
{
}

void CConsole::ConModCommandStatus(IResult *pResult, void *pUser)
{
}

struct CIntVariableData
{
	IConsole *m_pConsole;
	int *m_pVariable;
	int m_Min;
	int m_Max;
};

struct CStrVariableData
{
	IConsole *m_pConsole;
	char *m_pStr;
	int m_MaxSize;
	int m_Length;
};

void CConsole::TraverseChain(FCommandCallback *ppfnCallback, void **ppUserData)
{
}

void CConsole::Con_EvalIf(IResult *pResult, void *pUserData)
{
}

void CConsole::ConToggle(IConsole::IResult *pResult, void *pUser)
{
}

void CConsole::ConToggleStroke(IConsole::IResult *pResult, void *pUser)
{
}

CConsole::CConsole(int FlagMask)
{
}

CConsole::~CConsole()
{
}

void CConsole::Init()
{
}

void CConsole::ParseArguments(int NumArgs, const char **ppArguments)
{
}

void CConsole::AddCommandSorted(CCommand *pCommand)
{
}

void CConsole::Register(const char *pName, const char *pParams,
	int Flags, FCommandCallback pfnFunc, void *pUser, const char *pHelp)
{
}

void CConsole::RegisterTemp(const char *pName, const char *pParams,	int Flags, const char *pHelp)
{
}

void CConsole::DeregisterTemp(const char *pName)
{
}

void CConsole::DeregisterTempAll()
{
}

void CConsole::RegisterTempMap(const char *pName)
{
}

void CConsole::DeregisterTempMap(const char *pName)
{
}

void CConsole::DeregisterTempMapAll()
{
}

void CConsole::Con_Chain(IResult *pResult, void *pUserData)
{
}

void CConsole::Chain(const char *pName, FChainCommandCallback pfnChainFunc, void *pUser)
{
}

void CConsole::StoreCommands(bool Store)
{
}


const IConsole::CCommandInfo *CConsole::GetCommandInfo(const char *pName, int FlagMask, bool Temp)
{
	return 0;
}


extern IConsole *CreateConsole(int FlagMask) { return new CConsole(FlagMask); }
