/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENT_H
#define GAME_CLIENT_COMPONENT_H

#include "gameclient.h"

class CComponent
{
protected:
	friend class CGameClient;

	CGameClient *m_pClient;

	// perhaps propagte pointers for these as well
	class IClient *Client() const { return m_pClient->Client(); }
public:
	virtual ~CComponent() {}

	virtual void OnReset() {}
	virtual void OnMapLoad() {}
	virtual void OnMessage(int Msg, void *pRawMsg) {}
	virtual bool OnCursorMove(float x, float y, int CursorType) { return false; }
};

#endif
