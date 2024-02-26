/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "gamecore.h"

const char *CTuningParams::s_apNames[] =
{
	#define MACRO_TUNING_PARAM(Name,ScriptName,Value) #ScriptName,
	#undef MACRO_TUNING_PARAM
};


bool CTuningParams::Set(int Index, float Value)
{
	return true;
}

bool CTuningParams::Get(int Index, float *pValue) const
{
	return true;
}

bool CTuningParams::Set(const char *pName, float Value)
{
	return false;
}

bool CTuningParams::Get(const char *pName, float *pValue) const
{
	return false;
}

int CTuningParams::PossibleTunings(const char *pStr, IConsole::FPossibleCallback pfnCallback, void *pUser)
{
	return 0;
}

void CCharacterCore::Init(CWorldCore *pWorld, CCollision *pCollision)
{
}

void CCharacterCore::Reset()
{
}

void CCharacterCore::Tick(bool UseInput)
{
}

void CCharacterCore::AddDragVelocity()
{
}

void CCharacterCore::ResetDragVelocity()
{
}

void CCharacterCore::Move()
{
}

void CCharacterCore::Write(CNetObj_CharacterCore *pObjCore) const
{
}

void CCharacterCore::Read(const CNetObj_CharacterCore *pObjCore)
{
}

void CCharacterCore::Quantize()
{
}
