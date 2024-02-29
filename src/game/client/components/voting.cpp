/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/vmath.h>

#include <generated/protocol.h>

#include "chat.h"
#include "voting.h"


void CVoting::Callvote(const char *pType, const char *pValue, const char *pReason, bool ForceVote)
{
	CNetMsg_Cl_CallVote Msg = {0};
	Msg.m_Type = pType;
	Msg.m_Value = pValue;
	Msg.m_Reason = pReason;
	Msg.m_Force = ForceVote;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CVoting::Vote(int Choice)
{
	CNetMsg_Cl_Vote Msg = { Choice };
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CVoting::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_VOTESET)
	{
		// CNetMsg_Sv_VoteSet *pMsg = (CNetMsg_Sv_VoteSet *)pRawMsg;
	}
	else if(MsgType == NETMSGTYPE_SV_VOTESTATUS)
	{
		CNetMsg_Sv_VoteStatus *pMsg = (CNetMsg_Sv_VoteStatus *)pRawMsg;
		dbg_msg("vote", "Yes=%d", pMsg->m_Yes);
		dbg_msg("vote", "No=%d", pMsg->m_No);
		dbg_msg("vote", "Pass=%d", pMsg->m_Pass);
		dbg_msg("vote", "Total=%d", pMsg->m_Total);
	}
	else if(MsgType == NETMSGTYPE_SV_VOTECLEAROPTIONS)
	{
	}
	else if(MsgType == NETMSGTYPE_SV_VOTEOPTIONADD)
	{
		// CNetMsg_Sv_VoteOptionAdd *pMsg = (CNetMsg_Sv_VoteOptionAdd *)pRawMsg;
	}
	else if(MsgType == NETMSGTYPE_SV_VOTEOPTIONREMOVE)
	{
		// CNetMsg_Sv_VoteOptionRemove *pMsg = (CNetMsg_Sv_VoteOptionRemove *)pRawMsg;
	}
}
