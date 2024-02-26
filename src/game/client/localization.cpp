/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/system.h>
#include <base/tl/algorithm.h>

#include <engine/external/json-parser/json.h>
#include <engine/console.h>
#include <engine/storage.h>

#include "localization.h"

const char *Localize(const char *pStr, const char *pContext)
{
	return pStr;
}

CLocConstString::CLocConstString(const char *pStr, const char *pContext)
{
}

void CLocConstString::Reload()
{
}

CLocalizationDatabase::CLocalizationDatabase()
{
}

void CLocalizationDatabase::AddString(const char *pOrgStr, const char *pNewStr, const char *pContext)
{
}

bool CLocalizationDatabase::Load(const char *pFilename, IStorage *pStorage, IConsole *pConsole)
{
	return true;
}

CLocalizationDatabase g_Localization;
