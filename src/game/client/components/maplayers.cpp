/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/tl/array.h>


#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/render.h>

#include "menus.h"
#include "maplayers.h"

CMapLayers::CMapLayers(int Type)
{
}

void CMapLayers::OnStateChange(int NewState, int OldState)
{
}

void CMapLayers::LoadBackgroundMap()
{
}

int CMapLayers::GetInitAmount() const
{
	return 0;
}

void CMapLayers::OnInit()
{
}

void CMapLayers::OnMapLoad()
{
}

void CMapLayers::OnShutdown()
{
}

void CMapLayers::LoadEnvPoints(const CLayers *pLayers, array<CEnvPoint>& lEnvPoints)
{
}

void CMapLayers::EnvelopeEval(float TimeOffset, int Env, float *pChannels, void *pUser)
{
}

void CMapLayers::OnRender()
{
}

void CMapLayers::ConchainBackgroundMap(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
}

void CMapLayers::OnConsoleInit()
{
}

void CMapLayers::BackgroundMapUpdate()
{
}

void CMapLayers::PlaceEasterEggs(const CLayers *pLayers)
{
}
