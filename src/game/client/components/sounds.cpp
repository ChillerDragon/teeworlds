/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/engine.h>
#include <engine/sound.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>
#include <game/client/gameclient.h>
#include <game/client/components/camera.h>
#include <game/client/components/menus.h>
#include "sounds.h"


struct CUserData
{
	CGameClient *m_pGameClient;
	bool m_Render;
} g_UserData;

int CSounds::GetInitAmount() const
{
	return 0;
}

void CSounds::OnInit()
{
}

void CSounds::OnReset()
{
}

void CSounds::OnStateChange(int NewState, int OldState)
{
}

void CSounds::OnRender()
{
}

void CSounds::ClearQueue()
{
}

void CSounds::Enqueue(int Channel, int SetId)
{
}

void CSounds::Play(int Chn, int SetId, float Vol)
{
}

void CSounds::PlayAt(int Chn, int SetId, float Vol, vec2 Pos)
{
}

void CSounds::Stop(int SetId)
{
}

bool CSounds::IsPlaying(int SetId)
{
	return false;
}
