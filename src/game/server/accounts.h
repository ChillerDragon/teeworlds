#ifndef GAME_SERVER_ACCOUNTS_H
#define GAME_SERVER_ACCOUNTS_H

struct CAccountData {
    CAccountData()
    {
        m_aUsername[0] = '\0';
        m_aPassword[0] = '\0';
        m_Xp = 0;
        m_Level = 0;
    }
    char m_aUsername[128];
    char m_aPassword[128];
    int m_Xp;
    int m_Level;
};

class CAccounts
{
    class CGameContext *m_pGameServer;
    CGameContext *GameServer() { return m_pGameServer; }

public:
    CAccounts(CGameContext *pGameServer);

    /*
        ReadAccount

        returns true on success and false on failure
    */
    bool ReadAccount(int ClientID, const char *pUsername, const char *pPassword);
    void WriteAccount(const CAccountData &Data, int LoggedIn);
};

#endif
