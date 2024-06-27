
#include <engine/shared/protocol.h>
#include <engine/shared/network.h>

#if __has_include(<generated/protocol.h>)
#include <generated/protocol.h>
#endif

#include "item_printer.h"

void print_netobj_as_struct(const int *pData)
{
        int Type = *pData++;
        int Id = *pData++;
        if(Type == NETOBJ_INVALID)
        {
                dbg_msg("network_in", "  NETOBJ_INVALID!!");
        }
        else if(Type == NETOBJTYPE_PLAYERINPUT)
        {
                CNetObj_PlayerInput Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_PlayerInput");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "  	int m_Direction     = %d;", Item.m_Direction);
                dbg_msg("network_in", "  	int m_TargetX       = %d;", Item.m_TargetX);
                dbg_msg("network_in", "  	int m_TargetY       = %d;", Item.m_TargetY);
                dbg_msg("network_in", "  	int m_Jump      = %d;", Item.m_Jump);
                dbg_msg("network_in", "  	int m_Fire      = %d;", Item.m_Fire);
                dbg_msg("network_in", "  	int m_Hook      = %d;", Item.m_Hook);
                dbg_msg("network_in", "  	int m_PlayerFlags       = %d;", Item.m_PlayerFlags);
                dbg_msg("network_in", "  	int m_WantedWeapon      = %d;", Item.m_WantedWeapon);
                dbg_msg("network_in", "  	int m_NextWeapon        = %d;", Item.m_NextWeapon);
                dbg_msg("network_in", "  	int m_PrevWeapon        = %d;", Item.m_PrevWeapon);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_PROJECTILE)
        {
                CNetObj_Projectile Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_Projectile");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_VelX = %d;", Item.m_VelX);
                dbg_msg("network_in", "  	int m_VelY = %d;", Item.m_VelY);
                dbg_msg("network_in", "  	int m_Type = %d;", Item.m_Type);
                dbg_msg("network_in", "  	int m_StartTick = %d;", Item.m_StartTick);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_LASER)
        {
                CNetObj_Laser Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_Laser");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_FromX = %d;", Item.m_FromX);
                dbg_msg("network_in", "  	int m_FromY = %d;", Item.m_FromY);
                dbg_msg("network_in", "  	int m_StartTick = %d;", Item.m_StartTick);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_PICKUP)
        {
                CNetObj_Pickup Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_Pickup");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X    = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y    = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_Type = %d;", Item.m_Type);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_FLAG)
        {
                CNetObj_Flag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_Flag");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_Team = %d;", Item.m_Team);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_GAMEDATA)
        {
                CNetObj_GameData Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "    struct CNetObj_GameData");
                dbg_msg("network_in", "    {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "    	int m_GameStartTick    %d", Item.m_GameStartTick);
                dbg_msg("network_in", "    	int m_GameStateFlags   %d", Item.m_GameStateFlags);
                dbg_msg("network_in", "    	int m_GameStateEndTick %d", Item.m_GameStateEndTick);
                dbg_msg("network_in", "    };");
        }
        else if(Type == NETOBJTYPE_GAMEDATATEAM)
        {
                CNetObj_GameDataTeam Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_GameDataTeam");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_TeamscoreRed = %d;", Item.m_TeamscoreRed);
                dbg_msg("network_in", "  	int m_TeamscoreBlue = %d;", Item.m_TeamscoreBlue);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_GAMEDATAFLAG)
        {
                CNetObj_GameDataFlag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_GameDataFlag");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_FlagCarrierRed = %d;", Item.m_FlagCarrierRed);
                dbg_msg("network_in", "  	int m_FlagCarrierBlue = %d;", Item.m_FlagCarrierBlue);
                dbg_msg("network_in", "  	int m_FlagDropTickRed = %d;", Item.m_FlagDropTickRed);
                dbg_msg("network_in", "  	int m_FlagDropTickBlue = %d;", Item.m_FlagDropTickBlue);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_CHARACTERCORE)
        {
                // only used for inheritance in character
        }
        else if(Type == NETOBJTYPE_CHARACTER)
        {
                CNetObj_Character Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_Character");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_Tick = %d;", Item.m_Tick);
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_VelX = %d;", Item.m_VelX);
                dbg_msg("network_in", "  	int m_VelY = %d;", Item.m_VelY);
                dbg_msg("network_in", "  	int m_Angle = %d;", Item.m_Angle);
                dbg_msg("network_in", "  	int m_Direction = %d;", Item.m_Direction);
                dbg_msg("network_in", "  	int m_Jumped = %d;", Item.m_Jumped);
                dbg_msg("network_in", "  	int m_HookedPlayer = %d;", Item.m_HookedPlayer);
                dbg_msg("network_in", "  	int m_HookState = %d;", Item.m_HookState);
                dbg_msg("network_in", "  	int m_HookTick = %d;", Item.m_HookTick);
                dbg_msg("network_in", "  	int m_HookX = %d;", Item.m_HookX);
                dbg_msg("network_in", "  	int m_HookY = %d;", Item.m_HookY);
                dbg_msg("network_in", "  	int m_HookDx = %d;", Item.m_HookDx);
                dbg_msg("network_in", "  	int m_HookDy = %d;", Item.m_HookDy);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_Health = %d;", Item.m_Health);
                dbg_msg("network_in", "  	int m_Armor = %d;", Item.m_Armor);
                dbg_msg("network_in", "  	int m_AmmoCount = %d;", Item.m_AmmoCount);
                dbg_msg("network_in", "  	int m_Weapon = %d;", Item.m_Weapon);
                dbg_msg("network_in", "  	int m_Emote = %d;", Item.m_Emote);
                dbg_msg("network_in", "  	int m_AttackTick = %d;", Item.m_AttackTick);
                dbg_msg("network_in", "  	int m_TriggeredEvents = %d;", Item.m_TriggeredEvents);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_PLAYERINFO)
        {
                CNetObj_PlayerInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_PlayerInfo");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_PlayerFlags = %d;", Item.m_PlayerFlags);
                dbg_msg("network_in", "  	int m_Score       = %d;", Item.m_Score);
                dbg_msg("network_in", "  	int m_Latency     = %d;", Item.m_Latency);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_SPECTATORINFO)
        {
                CNetObj_SpectatorInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_SpectatorInfo");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_SpecMode = %d;", Item.m_SpecMode);
                dbg_msg("network_in", "  	int m_SpectatorID = %d;", Item.m_SpectatorID);
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_DE_CLIENTINFO)
        {
                CNetObj_De_ClientInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_De_ClientInfo");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_Local;");
                dbg_msg("network_in", "  	int m_Team;");
                dbg_msg("network_in", "  	int m_aName[4];");
                dbg_msg("network_in", "  	int m_aClan[3];");
                dbg_msg("network_in", "  	int m_Country;");
                dbg_msg("network_in", "  	int m_aaSkinPartNames[6][6];");
                dbg_msg("network_in", "  	int m_aUseCustomColors[6];");
                dbg_msg("network_in", "  	int m_aSkinPartColors[6];");
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_DE_GAMEINFO)
        {
                CNetObj_De_GameInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_De_GameInfo");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_GameFlags = %d;", Item.m_GameFlags);
                dbg_msg("network_in", "  	int m_ScoreLimit = %d;", Item.m_ScoreLimit);
                dbg_msg("network_in", "  	int m_TimeLimit = %d;", Item.m_TimeLimit);
                dbg_msg("network_in", "  	int m_MatchNum = %d;", Item.m_MatchNum);
                dbg_msg("network_in", "  	int m_MatchCurrent = %d;", Item.m_MatchCurrent);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_DE_TUNEPARAMS)
        {
                // CNetObj_De_TuneParams Item;
                // mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_De_TuneParams");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_aTuneParams[32];");
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETEVENTTYPE_COMMON)
        {
                CNetEvent_Common Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetEvent_Common");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETEVENTTYPE_EXPLOSION)
        {
                CNetEvent_Explosion Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetEvent_Explosion : public CNetEvent_Common");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETEVENTTYPE_SPAWN)
        {
                CNetEvent_Spawn Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetEvent_Spawn : public CNetEvent_Common");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETEVENTTYPE_HAMMERHIT)
        {
                CNetEvent_HammerHit Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetEvent_HammerHit : public CNetEvent_Common");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETEVENTTYPE_DEATH)
        {
                CNetEvent_Death Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetEvent_Death : public CNetEvent_Common");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_ClientID = %d;", Item.m_ClientID);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETEVENTTYPE_SOUNDWORLD)
        {
                CNetEvent_SoundWorld Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetEvent_SoundWorld : public CNetEvent_Common");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_SoundID = %d;", Item.m_SoundID);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETEVENTTYPE_DAMAGE)
        {
                CNetEvent_Damage Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetEvent_Damage : public CNetEvent_Common");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_X = %d;", Item.m_X);
                dbg_msg("network_in", "  	int m_Y = %d;", Item.m_Y);
                dbg_msg("network_in", "  	int m_ClientID = %d;", Item.m_ClientID);
                dbg_msg("network_in", "  	int m_Angle = %d;", Item.m_Angle);
                dbg_msg("network_in", "  	int m_HealthAmount = %d;", Item.m_HealthAmount);
                dbg_msg("network_in", "  	int m_ArmorAmount = %d;", Item.m_ArmorAmount);
                dbg_msg("network_in", "  	int m_Self = %d;", Item.m_Self);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_PLAYERINFORACE)
        {
                CNetObj_PlayerInfoRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_PlayerInfoRace");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_RaceStartTick = %d;", Item.m_RaceStartTick);
                dbg_msg("network_in", "  };");
        }
        else if(Type == NETOBJTYPE_GAMEDATARACE)
        {
                CNetObj_GameDataRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "  struct CNetObj_GameDataRace");
                dbg_msg("network_in", "  {");
                dbg_msg("network_in", "    	int TypeId = %d;", Type);
                dbg_msg("network_in", "    	int Id     = %d;", Id);
                dbg_msg("network_in", " ");
                dbg_msg("network_in", "  	int m_BestTime  = %d;", Item.m_BestTime);
                dbg_msg("network_in", "  	int m_Precision = %d;", Item.m_Precision);
                dbg_msg("network_in", "  	int m_RaceFlags = %d;", Item.m_RaceFlags);
                dbg_msg("network_in", "  };");
        }
        dbg_msg("network_in", " ");
}
