/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/config.h>
#include <engine/storage.h>
#include <engine/shared/config.h>
#include <game/version.h>


void EscapeParam(char *pDst, const char *pSrc, int size)
{
	for(int i = 0; *pSrc && i < size - 1; ++i)
	{
		if(*pSrc == '"' || *pSrc == '\\') // escape \ and "
			*pDst++ = '\\';
		*pDst++ = *pSrc++;
	}
	*pDst = 0;
}

CConfigManager::CConfigManager()
{
	m_pStorage = 0;
	m_ConfigFile = 0;
	m_FlagMask = 0;
	m_NumCallbacks = 0;
}

void CConfigManager::Init(int FlagMask)
{
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	m_FlagMask = FlagMask;
	Reset();
}

void CConfigManager::Reset()
{
}

void CConfigManager::RestoreStrings()
{
}

void CConfigManager::Save(const char *pFilename)
{
}

void CConfigManager::RegisterCallback(SAVECALLBACKFUNC pfnFunc, void *pUserData)
{
	dbg_assert(m_NumCallbacks < MAX_CALLBACKS, "too many config callbacks");
	m_aCallbacks[m_NumCallbacks].m_pfnFunc = pfnFunc;
	m_aCallbacks[m_NumCallbacks].m_pUserData = pUserData;
	m_NumCallbacks++;
}

void CConfigManager::WriteLine(const char *pLine)
{
	if(!m_ConfigFile)
		return;

	io_write(m_ConfigFile, pLine, str_length(pLine));
	io_write_newline(m_ConfigFile);
}

IConfigManager *CreateConfigManager() { return new CConfigManager; }
