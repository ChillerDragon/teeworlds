/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_VOTING_H
#define GAME_CLIENT_COMPONENTS_VOTING_H

#include <game/voting.h>
#include <game/client/component.h>

class CVoting : public CComponent
{
	void Callvote(const char *pType, const char *pValue, const char *pReason, bool ForceVote);

public:
	virtual void OnMessage(int Msgtype, void *pRawMsg);
	void Vote(int Choice);
};

#endif
