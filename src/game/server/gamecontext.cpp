/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <generated/server_data.h>
#include <game/version.h>

#include "gamecontext.h"
#include "player.h"

void CGameContext::Construct()
{
	m_pServer = 0;
}

CGameContext::CGameContext()
{
	Construct();
}

void CGameContext::CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self)
{
	float f = angle(Source);
	CNetEvent_Damage *pEvent = (CNetEvent_Damage *)m_Events.Create(NETEVENTTYPE_DAMAGE, sizeof(CNetEvent_Damage));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_ClientID = Id;
		pEvent->m_Angle = (int)(f*256.0f);
		pEvent->m_HealthAmount = HealthAmount;
		pEvent->m_ArmorAmount = ArmorAmount;
		pEvent->m_Self = Self;
	}
}

void CGameContext::CreateHammerHit(vec2 Pos)
{
	// create the event
	CNetEvent_HammerHit *pEvent = (CNetEvent_HammerHit *)m_Events.Create(NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}


void CGameContext::CreateExplosion(vec2 Pos, int Owner, int Weapon, int MaxDamage)
{
	// create the event
	CNetEvent_Explosion *pEvent = (CNetEvent_Explosion *)m_Events.Create(NETEVENTTYPE_EXPLOSION, sizeof(CNetEvent_Explosion));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}

void CGameContext::CreatePlayerSpawn(vec2 Pos)
{
	// create the event
	CNetEvent_Spawn *ev = (CNetEvent_Spawn *)m_Events.Create(NETEVENTTYPE_SPAWN, sizeof(CNetEvent_Spawn));
	if(ev)
	{
		ev->m_X = (int)Pos.x;
		ev->m_Y = (int)Pos.y;
	}
}

void CGameContext::CreateDeath(vec2 Pos, int ClientID)
{
	// create the event
	CNetEvent_Death *pEvent = (CNetEvent_Death *)m_Events.Create(NETEVENTTYPE_DEATH, sizeof(CNetEvent_Death));
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_ClientID = ClientID;
	}
}

void CGameContext::CreateSound(vec2 Pos, int Sound, int64 Mask)
{
	if (Sound < 0)
		return;

	// create a sound
	CNetEvent_SoundWorld *pEvent = (CNetEvent_SoundWorld *)m_Events.Create(NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_SoundID = Sound;
	}
}

void CGameContext::SendChat()
{
	CNetMsg_Sv_Chat Msg;
	Msg.m_Mode = CHAT_ALL;
	Msg.m_ClientID = 1;
	Msg.m_pMessage = "hello world";
	Msg.m_TargetID = -1;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
}

void CGameContext::SendBroadcast(const char* pText, int ClientID)
{
	CNetMsg_Sv_Broadcast Msg;
	Msg.m_pMessage = pText;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendEmoticon(int ClientID, int Emoticon)
{
	CNetMsg_Sv_Emoticon Msg;
	Msg.m_ClientID = ClientID;
	Msg.m_Emoticon = Emoticon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
}

void CGameContext::SendWeaponPickup(int ClientID, int Weapon)
{
	CNetMsg_Sv_WeaponPickup Msg;
	Msg.m_Weapon = Weapon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendMotd(int ClientID)
{
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = "hello world";
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendSettings(int ClientID)
{
	CNetMsg_Sv_ServerSettings Msg;
	Msg.m_KickVote = 0; // m_SvVoteKick
	Msg.m_KickMin = 0; // m_SvVoteKickMin
	Msg.m_SpecVote = 0; // m_SvVoteSpectate
	Msg.m_TeamLock = 0;
	Msg.m_TeamBalance = 0; // m_SvTeambalanceTime != 0;
	Msg.m_PlayerSlots = 10; // m_SvPlayerSlots
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendSkinChange(int ClientID, int TargetID)
{
	CNetMsg_Sv_SkinChange Msg;
	Msg.m_ClientID = ClientID;
	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		Msg.m_apSkinPartNames[p] = "greesward";
		Msg.m_aUseCustomColors[p] = 0;
		Msg.m_aSkinPartColors[p] = 0;
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, TargetID);
}

void CGameContext::SendGameMsg(int GameMsgID, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendGameMsg(int GameMsgID, int ParaI1, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Msg.AddInt(ParaI1);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendGameMsg(int GameMsgID, int ParaI1, int ParaI2, int ParaI3, int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_GAMEMSG);
	Msg.AddInt(GameMsgID);
	Msg.AddInt(ParaI1);
	Msg.AddInt(ParaI2);
	Msg.AddInt(ParaI3);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// void CGameContext::SendChatCommand(const CCommandManager::CCommand *pCommand, int ClientID)
// {
// 	CNetMsg_Sv_CommandInfo Msg;
// 	Msg.m_Name = pCommand->m_aName;
// 	Msg.m_HelpText = pCommand->m_aHelpText;
// 	Msg.m_ArgsFormat = pCommand->m_aArgsFormat;

// 	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
// }

void CGameContext::SendChatCommands(int ClientID)
{
	// for(int i = 0; i < CommandManager()->CommandCount(); i++)
	// {
	// 	SendChatCommand(CommandManager()->GetCommand(i), ClientID);
	// }
}

// void CGameContext::SendRemoveChatCommand(const CCommandManager::CCommand *pCommand, int ClientID)
// {
// 	CNetMsg_Sv_CommandInfoRemove Msg;
// 	Msg.m_Name = pCommand->m_aName;

// 	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
// }

void CGameContext::ForceVote(int Type, const char *pDescription, const char *pReason)
{
	CNetMsg_Sv_VoteSet Msg;
	Msg.m_Type = Type;
	Msg.m_Timeout = 0;
	Msg.m_ClientID = -1;
	Msg.m_pDescription = pDescription;
	Msg.m_pReason = pReason;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
}

void CGameContext::SendVoteSet(int Type, int ToClientID)
{
	int VoteCreator = 1;
	CNetMsg_Sv_VoteSet Msg;
	Msg.m_Type = Type;
	Msg.m_Timeout = 0;
	Msg.m_ClientID = VoteCreator;
	Msg.m_pDescription = "";
	Msg.m_pReason = "";
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ToClientID);
}

void CGameContext::SendVoteStatus(int ClientID, int Total, int Yes, int No)
{
	CNetMsg_Sv_VoteStatus Msg = {0};
	Msg.m_Total = Total;
	Msg.m_Yes = Yes;
	Msg.m_No = No;
	Msg.m_Pass = Total - (Yes+No);

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::OnClientEnter(int ClientID)
{
	// send chat commands
	SendChatCommands(ClientID);

	// update client infos (others before local)
	CNetMsg_Sv_ClientInfo NewClientInfoMsg;
	NewClientInfoMsg.m_ClientID = ClientID;
	NewClientInfoMsg.m_Local = 0;
	NewClientInfoMsg.m_Team = TEAM_RED;
	NewClientInfoMsg.m_pName = "namless tee";
	NewClientInfoMsg.m_pClan = "";
	NewClientInfoMsg.m_Country = 0;
	NewClientInfoMsg.m_Silent = false;

	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		NewClientInfoMsg.m_apSkinPartNames[p] = "greensward";
		NewClientInfoMsg.m_aUseCustomColors[p] = 0;
		NewClientInfoMsg.m_aSkinPartColors[p] = 0;
	}


	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(i == ClientID || !m_apPlayers[i] || (!Server()->ClientIngame(i) && !m_apPlayers[i]->IsDummy()))
			continue;

		// new info for others
		if(Server()->ClientIngame(i))
			Server()->SendPackMsg(&NewClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);

		// existing infos for new player
		CNetMsg_Sv_ClientInfo ClientInfoMsg;
		ClientInfoMsg.m_ClientID = i;
		ClientInfoMsg.m_Local = 0;
		ClientInfoMsg.m_Team = TEAM_RED;
		ClientInfoMsg.m_pName = "nameless tee";
		ClientInfoMsg.m_pClan = "";
		ClientInfoMsg.m_Country = 0;
		ClientInfoMsg.m_Silent = false;
		for(int p = 0; p < NUM_SKINPARTS; p++)
		{
			ClientInfoMsg.m_apSkinPartNames[p] = "greensward";
			ClientInfoMsg.m_aUseCustomColors[p] = 0;
			ClientInfoMsg.m_aSkinPartColors[p] = 0;
		}
		Server()->SendPackMsg(&ClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
	}

	// local info
	NewClientInfoMsg.m_Local = 1;
	Server()->SendPackMsg(&NewClientInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
}

void CGameContext::OnClientConnected(int ClientID, bool Dummy, bool AsSpec)
{
	SendMotd(ClientID);
	SendSettings(ClientID);
}

void CGameContext::OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
{
	void *pRawMsg = m_NetObjHandler.SecureUnpackMsg(MsgID, pUnpacker);
	CPlayer *pPlayer = m_apPlayers[ClientID];

	if(!pRawMsg)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "dropped weird message '%s' (%d), failed on '%s'", m_NetObjHandler.GetMsgName(MsgID), MsgID, m_NetObjHandler.FailedMsgOn());
		dbg_msg("server", "%s", aBuf);
		return;
	}

	if(Server()->ClientIngame(ClientID))
	{
		if(MsgID == NETMSGTYPE_CL_SAY)
		{
			CNetMsg_Cl_Say *pMsg = (CNetMsg_Cl_Say *)pRawMsg;

			// trim right and set maximum length to 128 utf8-characters
			int Length = 0;
			const char *p = pMsg->m_pMessage;
			const char *pEnd = 0;
			while(*p)
			{
				const char *pStrOld = p;
				int Code = str_utf8_decode(&p);

				// check if unicode is not empty
				if(!str_utf8_is_whitespace(Code))
				{
					pEnd = 0;
				}
				else if(pEnd == 0)
					pEnd = pStrOld;

				if(++Length >= 127)
				{
					*(const_cast<char *>(p)) = 0;
					break;
				}
			}
			if(pEnd != 0)
				*(const_cast<char *>(pEnd)) = 0;

			// drop empty and autocreated spam messages (more than 20 characters per second)
			if(Length == 0)
				return;

			pPlayer->m_LastChatTeamTick = Server()->Tick();
			dbg_msg("chat", "ClientID=%d pMsg->m_Mode=%d pMsg->m_Target=%d pMsg->m_pMessage=%s",ClientID, pMsg->m_Mode, pMsg->m_Target, pMsg->m_pMessage);
		}
		else if(MsgID == NETMSGTYPE_CL_CALLVOTE)
		{
			// CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)pRawMsg;
		}
		else if(MsgID == NETMSGTYPE_CL_VOTE)
		{
		}
		else if(MsgID == NETMSGTYPE_CL_SETTEAM)
		{
			// CNetMsg_Cl_SetTeam *pMsg = (CNetMsg_Cl_SetTeam *)pRawMsg;
		}
		else if (MsgID == NETMSGTYPE_CL_SETSPECTATORMODE)
		{
			// CNetMsg_Cl_SetSpectatorMode *pMsg = (CNetMsg_Cl_SetSpectatorMode *)pRawMsg;
			// if(!pPlayer->SetSpectatorID(pMsg->m_SpecMode, pMsg->m_SpectatorID))
			// 	SendGameMsg(GAMEMSG_SPEC_INVALIDID, ClientID);
		}
		else if (MsgID == NETMSGTYPE_CL_EMOTICON)
		{
			// CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)pRawMsg;
			// SendEmoticon(ClientID, pMsg->m_Emoticon);
		}
		else if (MsgID == NETMSGTYPE_CL_KILL)
		{
		}
		else if (MsgID == NETMSGTYPE_CL_READYCHANGE)
		{
		}
		else if(MsgID == NETMSGTYPE_CL_SKINCHANGE)
		{
			// CNetMsg_Cl_SkinChange *pMsg = (CNetMsg_Cl_SkinChange *)pRawMsg;

			// for(int p = 0; p < NUM_SKINPARTS; p++)
			// {
			// 	str_utf8_copy_num(pPlayer->m_TeeInfos.m_aaSkinPartNames[p], pMsg->m_apSkinPartNames[p], sizeof(pPlayer->m_TeeInfos.m_aaSkinPartNames[p]), MAX_SKIN_LENGTH);
			// 	pPlayer->m_TeeInfos.m_aUseCustomColors[p] = pMsg->m_aUseCustomColors[p];
			// 	pPlayer->m_TeeInfos.m_aSkinPartColors[p] = pMsg->m_aSkinPartColors[p];
			// }
			// for(int i = 0; i < MAX_CLIENTS; ++i)
			// 	SendSkinChange(pPlayer->GetCID(), i);
		}
		else if (MsgID == NETMSGTYPE_CL_COMMAND)
		{
			// CNetMsg_Cl_Command *pMsg = (CNetMsg_Cl_Command*)pRawMsg;
		}
	}
	else
	{
		if (MsgID == NETMSGTYPE_CL_STARTINFO)
		{
			CNetMsg_Cl_StartInfo *pMsg = (CNetMsg_Cl_StartInfo *)pRawMsg;
			pPlayer->m_LastChangeInfoTick = Server()->Tick();

			// set start infos
			Server()->SetClientName(ClientID, pMsg->m_pName);
			Server()->SetClientClan(ClientID, pMsg->m_pClan);
			Server()->SetClientCountry(ClientID, pMsg->m_Country);

			for(int p = 0; p < NUM_SKINPARTS; p++)
			{
				str_utf8_copy_num(pPlayer->m_TeeInfos.m_aaSkinPartNames[p], pMsg->m_apSkinPartNames[p], sizeof(pPlayer->m_TeeInfos.m_aaSkinPartNames[p]), MAX_SKIN_LENGTH);
				pPlayer->m_TeeInfos.m_aUseCustomColors[p] = pMsg->m_aUseCustomColors[p];
				pPlayer->m_TeeInfos.m_aSkinPartColors[p] = pMsg->m_aSkinPartColors[p];
			}

			// send vote options
			CNetMsg_Sv_VoteClearOptions ClearMsg;
			Server()->SendPackMsg(&ClearMsg, MSGFLAG_VITAL, ClientID);

			SendVotes(ClientID);

			// client is ready to enter
			pPlayer->m_IsReadyToEnter = true;
			CNetMsg_Sv_ReadyToEnter m;
			Server()->SendPackMsg(&m, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
		}
	}
}

void CGameContext::SendVotes(int ClientID)
{
	CMsgPacker Msg(NETMSGTYPE_SV_VOTEOPTIONLISTADD);
	Msg.AddInt(3);
	Msg.AddString("foo", VOTE_DESC_LENGTH);
	Msg.AddString("bar", VOTE_DESC_LENGTH);
	Msg.AddString("baz", VOTE_DESC_LENGTH);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

// void CGameContext::NewCommandHook(const CCommandManager::CCommand *pCommand, void *pContext)
// {
// 	CGameContext *pSelf = (CGameContext *)pContext;
// 	pSelf->SendChatCommand(pCommand, -1);
// }

// void CGameContext::RemoveCommandHook(const CCommandManager::CCommand *pCommand, void *pContext)
// {
// 	CGameContext *pSelf = (CGameContext *)pContext;
// 	pSelf->SendRemoveChatCommand(pCommand, -1);
// }

void CGameContext::OnInit(IServer *pServer)
{
	// init everything
	m_pServer = pServer;

	// HACK: only set static size for items, which were available in the first 0.7 release
	// so new items don't break the snapshot delta
	static const int OLD_NUM_NETOBJTYPES = 23;
	for(int i = 0; i < OLD_NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));
}

void CGameContext::OnSnap(int ClientID)
{
	m_Events.Snap(ClientID, this);
	// m_apPlayers[i]->Snap(ClientID);
}

const char *CGameContext::GameType() const { return "dm"; }
const char *CGameContext::Version() const { return GAME_VERSION; }
const char *CGameContext::NetVersion() const { return GAME_NETVERSION; }
const char *CGameContext::NetVersionHashUsed() const { return GAME_NETVERSION_HASH_FORCED; }
const char *CGameContext::NetVersionHashReal() const { return GAME_NETVERSION_HASH; }

IGameServer *CreateGameServer() { return new CGameContext; }
