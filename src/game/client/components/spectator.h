/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SPECTATOR_H
#define GAME_CLIENT_COMPONENTS_SPECTATOR_H

#include <game/client/component.h>

class CSpectator : public CComponent
{
public:
	void Spectate(int SpecMode, int SpectatorID);
};

#endif
