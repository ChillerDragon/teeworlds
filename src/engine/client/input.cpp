/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>

#include "input.h"

//print >>f, "int inp_key_code(const char *key_name) { int i; if (!strcmp(key_name, \"-?-\")) return -1; else for (i = 0; i < 512; i++) if (!strcmp(key_strings[i], key_name)) return i; return -1; }"

// this header is protected so you don't include it from anywere
#define KEYS_INCLUDE
#include "keynames.h"
#undef KEYS_INCLUDE

void CInput::AddEvent(int Unicode, int Key, int Flags)
{
	if(m_NumEvents != INPUT_BUFFER_SIZE)
	{
		m_aInputEvents[m_NumEvents].m_Unicode = Unicode;
		m_aInputEvents[m_NumEvents].m_Key = Key;
		m_aInputEvents[m_NumEvents].m_Flags = Flags;
		m_NumEvents++;
	}
}

CInput::CInput()
{
	mem_zero(m_aInputCount, sizeof(m_aInputCount));
	mem_zero(m_aInputState, sizeof(m_aInputState));

	m_InputCurrent = 0;
	m_InputGrabbed = 0;
	m_InputDispatched = false;

	m_LastRelease = 0;
	m_ReleaseDelta = -1;

	m_NumEvents = 0;
}

void CInput::Init()
{
	m_pGraphics = Kernel()->RequestInterface<IEngineGraphics>();
}

void CInput::MouseRelative(float *x, float *y)
{
}

void CInput::MouseModeAbsolute()
{
}

void CInput::MouseModeRelative()
{
}

int CInput::MouseDoubleClick()
{
	return 0;
}

void CInput::ClearKeyStates()
{
	mem_zero(m_aInputState, sizeof(m_aInputState));
	mem_zero(m_aInputCount, sizeof(m_aInputCount));
}

int CInput::KeyState(int Key)
{
	return m_aInputState[m_InputCurrent][Key];
}

int CInput::Update()
{
	return 0;
}


IEngineInput *CreateEngineInput() { return new CInput; }
