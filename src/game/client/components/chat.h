/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_CHAT_H
#define GAME_CLIENT_COMPONENTS_CHAT_H
#include <game/client/component.h>

class CChat : public CComponent
{
	// Mode defined by the CHAT_* constants in protocol.h
	int m_Mode;
	int m_WhisperTarget;
	const char *GetModeName(int Mode) const;

public:
	enum
	{
		CLIENT_MSG = -2,
		SERVER_MSG = -1,
	};

	void AddLine(const char *pLine, int ClientID = SERVER_MSG, int Mode = CHAT_NONE, int TargetID = -1);
	void Say(int Mode, const char *pLine);

	virtual void OnMessage(int MsgType, void *pRawMsg);
};
#endif
