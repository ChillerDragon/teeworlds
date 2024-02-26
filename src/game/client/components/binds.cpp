/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/config.h>
#include <engine/shared/config.h>
#include "binds.h"

const int CBinds::s_aaDefaultBindKeys[][2] = {
	{KEY_F1, 0}, {KEY_F2, 0}, {KEY_TAB, 0}, {'e', 0}, {'u', 0}, {KEY_F10, 0}, {'s', CBinds::MODIFIER_CTRL},
	{'a', 0}, {'d', 0},
	{KEY_SPACE, 0}, {KEY_MOUSE_1, 0}, {KEY_MOUSE_2, 0}, {KEY_LSHIFT, 0}, {KEY_RSHIFT, 0}, {KEY_RIGHT, 0}, {KEY_LEFT, 0},
	{'1', 0}, {'2', 0}, {'3', 0}, {'4', 0}, {'5', 0},
	{KEY_MOUSE_WHEEL_UP, 0}, {KEY_MOUSE_WHEEL_DOWN, 0},
	{'t', 0}, {'y', 0}, {'x', 0},
	{KEY_F3, 0}, {KEY_F4, 0},
	{'r', 0},
};
const char CBinds::s_aaDefaultBindValues[][32] = {
	"toggle_local_console", "toggle_remote_console", "+scoreboard", "+stats", "+show_chat", "screenshot", "snd_toggle",
	"+left", "+right",
	"+jump", "+fire", "+hook", "+emote", "+spectate", "spectate_next", "spectate_previous",
	"+weapon1", "+weapon2", "+weapon3", "+weapon4", "+weapon5",
	"+prevweapon", "+nextweapon",
	"chat all", "chat team", "chat whisper",
	"vote yes", "vote no",
	"ready_change",
};

CBinds::CBinds()
{
	mem_zero(m_aaaKeyBindings, sizeof(m_aaaKeyBindings));
	m_SpecialBinds.m_pBinds = this;
}

void CBinds::Bind(int KeyID, int Modifier, const char *pStr)
{
}


int CBinds::GetModifierMaskOfKey(int Key)
{
	switch(Key)
	{
		case KEY_LSHIFT:
		case KEY_RSHIFT:
			return 1 << CBinds::MODIFIER_SHIFT;
		case KEY_LCTRL:
		case KEY_RCTRL:
			return 1 << CBinds::MODIFIER_CTRL;
		case KEY_LALT:
			return 1 << CBinds::MODIFIER_ALT;
		default:
			return 0;
	}
}

bool CBinds::ModifierMatchesKey(int Modifier, int Key)
{
	switch(Modifier)
	{
		case MODIFIER_SHIFT:
			return Key == KEY_LSHIFT || Key == KEY_RSHIFT;
		case MODIFIER_CTRL:
			return Key == KEY_LCTRL || Key == KEY_RCTRL;
		case MODIFIER_ALT:
			return Key == KEY_LALT;
		case MODIFIER_NONE:
		default:
			return false;
	}
}

void CBinds::UnbindAll()
{
	for(int i = 0; i < KEY_LAST; i++)
		for(int m = 0; m < MODIFIER_COUNT; m++)
			m_aaaKeyBindings[i][m][0] = 0;
}

const char *CBinds::Get(int KeyID, int Modifier)
{
	if(KeyID > 0 && KeyID < KEY_LAST)
		return m_aaaKeyBindings[KeyID][Modifier];
	return "";
}

void CBinds::GetKeyID(const char *pBindStr, int& KeyID, int& Modifier)
{
	KeyID = KEY_LAST;
	Modifier = MODIFIER_COUNT;

	for(int LocalKeyID = 0; LocalKeyID < KEY_LAST; LocalKeyID++)
	{
		for(int LocalModifier = 0; LocalModifier < MODIFIER_COUNT; LocalModifier++)
		{
			const char *pBind = Get(LocalKeyID, LocalModifier);
			if(!pBind[0])
				continue;

			if(str_comp(pBind, pBindStr) == 0)
			{
				KeyID = LocalKeyID;
				Modifier = LocalModifier;
				return;
			}
			if(str_find(pBind, pBindStr) != 0)
			{
				KeyID = LocalKeyID;
				Modifier = LocalModifier;
			}
		}
	}
}

void CBinds::GetKey(const char *pBindStr, char aKey[64], unsigned BufSize, int KeyID, int Modifier)
{
}

void CBinds::GetKey(const char *pBindStr, char aKey[64], unsigned BufSize)
{
}

void CBinds::SetDefaults()
{
	// set default key bindings
	UnbindAll();
	const int count = sizeof(s_aaDefaultBindKeys)/sizeof(int)/2;
	dbg_assert(count == sizeof(s_aaDefaultBindValues)/32, "the count of bind keys differs from that of bind values!");
	for(int i = 0; i < count; i++)
		Bind(s_aaDefaultBindKeys[i][0], s_aaDefaultBindKeys[i][1], s_aaDefaultBindValues[i]);
}

void CBinds::OnConsoleInit()
{
	// bindings
	IConfigManager *pConfigManager = Kernel()->RequestInterface<IConfigManager>();
	if(pConfigManager)
		pConfigManager->RegisterCallback(ConfigSaveCallback, this);

	Console()->Register("bind", "s[key] r[command]", CFGFLAG_CLIENT, ConBind, this, "Bind key to execute the command");
	Console()->Register("unbind", "s[key]", CFGFLAG_CLIENT, ConUnbind, this, "Unbind key");
	Console()->Register("unbindall", "", CFGFLAG_CLIENT, ConUnbindAll, this, "Unbind all keys");
	Console()->Register("binds", "?s[key]", CFGFLAG_CLIENT, ConBinds, this, "Show list of key bindings");

	// default bindings
	SetDefaults();
}

void CBinds::ConBind(IConsole::IResult *pResult, void *pUserData)
{
	CBinds *pBinds = (CBinds *)pUserData;
	const char *pKeyName = pResult->GetString(0);
	int Modifier;
	int KeyID = pBinds->DecodeBindString(pKeyName, &Modifier);

	if(!KeyID)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "key %s not found", pKeyName);
		pBinds->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "binds", aBuf);
		return;
	}

	pBinds->Bind(KeyID, Modifier, pResult->GetString(1));
}


void CBinds::ConUnbind(IConsole::IResult *pResult, void *pUserData)
{
	CBinds *pBinds = (CBinds *)pUserData;
	const char *pKeyName = pResult->GetString(0);
	int Modifier;
	int KeyID = pBinds->DecodeBindString(pKeyName, &Modifier);

	if(!KeyID)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "key %s not found", pKeyName);
		pBinds->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "binds", aBuf);
		return;
	}

	pBinds->Bind(KeyID, Modifier, "");
}


void CBinds::ConUnbindAll(IConsole::IResult *pResult, void *pUserData)
{
	CBinds *pBinds = (CBinds *)pUserData;
	pBinds->UnbindAll();
}


void CBinds::ConBinds(IConsole::IResult *pResult, void *pUserData)
{
}

int CBinds::DecodeBindString(const char *pKeyName, int* pModifier)
{
	return 0;
}


const char *CBinds::GetModifierName(int m)
{
	switch(m)
	{
		case 0:
			return "";
		case MODIFIER_SHIFT:
			return "shift+";
		case MODIFIER_CTRL:
			return "ctrl+";
		case MODIFIER_ALT:
			return "alt+";
		default:
			return "";
	}
}

void CBinds::ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData)
{
}
