#include <base/system.h>
#include "gamecontext.h"
#include <engine/storage.h>
#include <engine/shared/linereader.h>
#include "player.h"

#include "accounts.h"


void CAccounts::WriteAccount(const CAccountData &Data, int LoggedIn)
{
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "accounts/%s.txt", Data.m_aUsername);
	IOHANDLE File = GameServer()->Storage()->OpenFile(aBuf, IOFLAG_WRITE, IStorage::TYPE_ALL);
	if(!File)
	{
		dbg_msg("mymod", "failed to write account '%s'", Data.m_aUsername);
	}
	else
	{
		io_write(File, Data.m_aUsername, str_length(Data.m_aUsername)); // username
		io_write_newline(File);
		io_write(File, Data.m_aPassword, str_length(Data.m_aPassword)); // username
		io_write_newline(File);
		str_format(aBuf, sizeof(aBuf), "%d", Data.m_Xp);
		io_write(File, aBuf, str_length(aBuf)); // xp
		io_write_newline(File);
		str_format(aBuf, sizeof(aBuf), "%d", Data.m_Level);
		io_write(File, aBuf, str_length(aBuf)); // level
		io_write_newline(File);
		str_format(aBuf, sizeof(aBuf), "%d", LoggedIn);
		io_write(File, aBuf, str_length(aBuf)); // is logged in
		io_close(File);
		dbg_msg("mymod", "wrote account '%s'", Data.m_aUsername);
	}
}

bool CAccounts::ReadAccount(int ClientID, const char *pUsername, const char *pPassword)
{
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "accounts/%s.txt", pUsername);

	IOHANDLE File = GameServer()->Storage()->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		GameServer()->SendChat(-1, CHAT_ALL, ClientID, "Account does not exists.");
		return false;
	}

	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(pPlayer->m_LoginAttempts > 2)
	{
		char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
		GameServer()->Server()->GetClientAddr(ClientID, aAddrStr, sizeof(aAddrStr));
		str_format(aBuf, sizeof(aBuf), "ban %s 10 Too many login attempts", aAddrStr);
		GameServer()->Console()->ExecuteLine(aBuf);
		return false;
	}
	if(pPlayer->m_AccountData.m_aUsername[0])
	{
		GameServer()->SendChat(-1, CHAT_ALL, ClientID, "You are already logged in.");
		return false;
	}

	CLineReader lr;
	lr.Init(File);

	str_copy(pPlayer->m_AccountData.m_aUsername, lr.Get(), sizeof(pPlayer->m_AccountData.m_aUsername));
	str_copy(pPlayer->m_AccountData.m_aPassword, lr.Get(), sizeof(pPlayer->m_AccountData.m_aPassword));

	if(str_comp(pPlayer->m_AccountData.m_aPassword, pPassword))
	{
		pPlayer->m_AccountData.m_aUsername[0] = '\0';
		pPlayer->m_LoginAttempts++;
		GameServer()->SendChat(-1, CHAT_ALL, ClientID, "Wrong password.");
		return false;
	}

	pPlayer->m_AccountData.m_Xp = atoi(lr.Get());
	pPlayer->m_AccountData.m_Level = atoi(lr.Get());
	if(atoi(lr.Get()) != 0)
	{
		pPlayer->m_AccountData.m_aUsername[0] = '\0';
		pPlayer->m_AccountData.m_Xp = 0;
		pPlayer->m_AccountData.m_Level = 0;
		GameServer()->SendChat(-1, CHAT_ALL, ClientID, "Account already logged in.");
		return false;
	}
	GameServer()->SendChat(-1, CHAT_ALL, ClientID, "Logged in.");
	io_close(File);
	return true;
}

CAccounts::CAccounts(CGameContext *pGameServer)
{
    m_pGameServer = pGameServer;
}
