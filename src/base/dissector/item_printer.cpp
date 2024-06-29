#include <engine/shared/protocol.h>
#include <engine/shared/network.h>

#if __has_include(<generated/protocol.h>)
#include <generated/protocol.h>
#endif

#include <base/dissector/enum_to_str.h>

#include "item_printer.h"

void print_netobj_as_struct(const int *pData, const char *pPrefix)
{
        int Type = *pData++;
        int Id = *pData++;
        if(Type == NETOBJ_INVALID)
        {
                dbg_msg("network_in", "%sNETOBJ_INVALID!!", pPrefix);
        }
        else if(Type == NETOBJTYPE_PLAYERINPUT)
        {
                CNetObj_PlayerInput Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_PlayerInput", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int m_Direction     = %d;", pPrefix, Item.m_Direction);
                dbg_msg("network_in", "%s	int m_TargetX       = %d;", pPrefix, Item.m_TargetX);
                dbg_msg("network_in", "%s	int m_TargetY       = %d;", pPrefix, Item.m_TargetY);
                dbg_msg("network_in", "%s	int m_Jump      = %d;", pPrefix, Item.m_Jump);
                dbg_msg("network_in", "%s	int m_Fire      = %d;", pPrefix, Item.m_Fire);
                dbg_msg("network_in", "%s	int m_Hook      = %d;", pPrefix, Item.m_Hook);
                dbg_msg("network_in", "%s	int m_PlayerFlags       = %d;", pPrefix, Item.m_PlayerFlags);
                dbg_msg("network_in", "%s	int m_WantedWeapon      = %d;", pPrefix, Item.m_WantedWeapon);
                dbg_msg("network_in", "%s	int m_NextWeapon        = %d;", pPrefix, Item.m_NextWeapon);
                dbg_msg("network_in", "%s	int m_PrevWeapon        = %d;", pPrefix, Item.m_PrevWeapon);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PROJECTILE)
        {
                CNetObj_Projectile Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_Projectile", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s	int m_VelX = %d;", pPrefix, Item.m_VelX);
                dbg_msg("network_in", "%s	int m_VelY = %d;", pPrefix, Item.m_VelY);
                dbg_msg("network_in", "%s	int m_Type = %d;", pPrefix, Item.m_Type);
                dbg_msg("network_in", "%s	int m_StartTick = %d;", pPrefix, Item.m_StartTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_LASER)
        {
                CNetObj_Laser Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_Laser", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s	int m_FromX = %d;", pPrefix, Item.m_FromX);
                dbg_msg("network_in", "%s	int m_FromY = %d;", pPrefix, Item.m_FromY);
                dbg_msg("network_in", "%s	int m_StartTick = %d;", pPrefix, Item.m_StartTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PICKUP)
        {
                CNetObj_Pickup Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_Pickup", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X    = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y    = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s	int m_Type = %d; // %s", pPrefix, Item.m_Type, enum_pickup_to_str(Item.m_Type));
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_FLAG)
        {
                CNetObj_Flag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_Flag", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s	int m_Team = %d; // %s", pPrefix, Item.m_Team, enum_team_to_str(Item.m_Team));
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATA)
        {
                CNetObj_GameData Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_GameData", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_GameStartTick    = %d;", pPrefix, Item.m_GameStartTick);
                dbg_msg("network_in", "%s	int m_GameStateFlags   = %d;", pPrefix, Item.m_GameStateFlags);
                dbg_msg("network_in", "%s	int m_GameStateEndTick = %d;", pPrefix, Item.m_GameStateEndTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATATEAM)
        {
                CNetObj_GameDataTeam Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_GameDataTeam", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_TeamscoreRed = %d;", pPrefix, Item.m_TeamscoreRed);
                dbg_msg("network_in", "%s	int m_TeamscoreBlue = %d;", pPrefix, Item.m_TeamscoreBlue);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATAFLAG)
        {
                CNetObj_GameDataFlag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_GameDataFlag", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_FlagCarrierRed = %d;", pPrefix, Item.m_FlagCarrierRed);
                dbg_msg("network_in", "%s	int m_FlagCarrierBlue = %d;", pPrefix, Item.m_FlagCarrierBlue);
                dbg_msg("network_in", "%s	int m_FlagDropTickRed = %d;", pPrefix, Item.m_FlagDropTickRed);
                dbg_msg("network_in", "%s	int m_FlagDropTickBlue = %d;", pPrefix, Item.m_FlagDropTickBlue);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_CHARACTERCORE)
        {
                // only used for inheritance in character
        }
        else if(Type == NETOBJTYPE_CHARACTER)
        {
                CNetObj_Character Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_Character", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_Tick = %d;", pPrefix, Item.m_Tick);
                dbg_msg("network_in", "%s	int m_X = %d; // tile %d", pPrefix, Item.m_X, Item.m_X / 32);
                dbg_msg("network_in", "%s	int m_Y = %d; // tile %d", pPrefix, Item.m_Y, Item.m_Y / 32);
                dbg_msg("network_in", "%s	int m_VelX = %d;", pPrefix, Item.m_VelX);
                dbg_msg("network_in", "%s	int m_VelY = %d;", pPrefix, Item.m_VelY);
                dbg_msg("network_in", "%s	int m_Angle = %d;", pPrefix, Item.m_Angle);
                dbg_msg("network_in", "%s	int m_Direction = %d;", pPrefix, Item.m_Direction);
                dbg_msg("network_in", "%s	int m_Jumped = %d;", pPrefix, Item.m_Jumped);
                dbg_msg("network_in", "%s	int m_HookedPlayer = %d;", pPrefix, Item.m_HookedPlayer);
                dbg_msg("network_in", "%s	int m_HookState = %d;", pPrefix, Item.m_HookState);
                dbg_msg("network_in", "%s	int m_HookTick = %d;", pPrefix, Item.m_HookTick);
                dbg_msg("network_in", "%s	int m_HookX = %d;", pPrefix, Item.m_HookX);
                dbg_msg("network_in", "%s	int m_HookY = %d;", pPrefix, Item.m_HookY);
                dbg_msg("network_in", "%s	int m_HookDx = %d;", pPrefix, Item.m_HookDx);
                dbg_msg("network_in", "%s	int m_HookDy = %d;", pPrefix, Item.m_HookDy);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_Health = %d;", pPrefix, Item.m_Health);
                dbg_msg("network_in", "%s	int m_Armor = %d;", pPrefix, Item.m_Armor);
                dbg_msg("network_in", "%s	int m_AmmoCount = %d;", pPrefix, Item.m_AmmoCount);
                dbg_msg("network_in", "%s	int m_Weapon = %d;", pPrefix, Item.m_Weapon);
                dbg_msg("network_in", "%s	int m_Emote = %d; // %s", pPrefix, Item.m_Emote, enum_emote_to_str(Item.m_Emote));
                dbg_msg("network_in", "%s	int m_AttackTick = %d;", pPrefix, Item.m_AttackTick);
                dbg_msg("network_in", "%s	int m_TriggeredEvents = %d;", pPrefix, Item.m_TriggeredEvents);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PLAYERINFO)
        {
                CNetObj_PlayerInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_PlayerInfo", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_PlayerFlags = %d;", pPrefix, Item.m_PlayerFlags);
                dbg_msg("network_in", "%s	int m_Score       = %d;", pPrefix, Item.m_Score);
                dbg_msg("network_in", "%s	int m_Latency     = %d;", pPrefix, Item.m_Latency);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_SPECTATORINFO)
        {
                CNetObj_SpectatorInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_SpectatorInfo", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_SpecMode = %d;", pPrefix, Item.m_SpecMode);
                dbg_msg("network_in", "%s	int m_SpectatorID = %d;", pPrefix, Item.m_SpectatorID);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_CLIENTINFO)
        {
                CNetObj_De_ClientInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_De_ClientInfo", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_Local;", pPrefix);
                dbg_msg("network_in", "%s	int m_Team;", pPrefix);
                dbg_msg("network_in", "%s	int m_aName[4];", pPrefix);
                dbg_msg("network_in", "%s	int m_aClan[3];", pPrefix);
                dbg_msg("network_in", "%s	int m_Country;", pPrefix);
                dbg_msg("network_in", "%s	int m_aaSkinPartNames[6][6];", pPrefix);
                dbg_msg("network_in", "%s	int m_aUseCustomColors[6];", pPrefix);
                dbg_msg("network_in", "%s	int m_aSkinPartColors[6];", pPrefix);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_GAMEINFO)
        {
                CNetObj_De_GameInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_De_GameInfo", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_GameFlags = %d;", pPrefix, Item.m_GameFlags);
                dbg_msg("network_in", "%s	int m_ScoreLimit = %d;", pPrefix, Item.m_ScoreLimit);
                dbg_msg("network_in", "%s	int m_TimeLimit = %d;", pPrefix, Item.m_TimeLimit);
                dbg_msg("network_in", "%s	int m_MatchNum = %d;", pPrefix, Item.m_MatchNum);
                dbg_msg("network_in", "%s	int m_MatchCurrent = %d;", pPrefix, Item.m_MatchCurrent);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_TUNEPARAMS)
        {
                // CNetObj_De_TuneParams Item;
                // mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_De_TuneParams", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_aTuneParams[32];", pPrefix);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_COMMON)
        {
                CNetEvent_Common Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_EXPLOSION)
        {
                CNetEvent_Explosion Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Explosion : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_SPAWN)
        {
                CNetEvent_Spawn Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Spawn : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_HAMMERHIT)
        {
                CNetEvent_HammerHit Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_HammerHit : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_DEATH)
        {
                CNetEvent_Death Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Death : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s	int m_ClientID = %d;", pPrefix, Item.m_ClientID);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_SOUNDWORLD)
        {
                CNetEvent_SoundWorld Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_SoundWorld : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s	int m_SoundID = %d; // %s", pPrefix, Item.m_SoundID, enum_sound_to_str(Item.m_SoundID));
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_DAMAGE)
        {
                CNetEvent_Damage Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Damage : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_X = %d;", pPrefix, Item.m_X);
                dbg_msg("network_in", "%s	int m_Y = %d;", pPrefix, Item.m_Y);
                dbg_msg("network_in", "%s	int m_ClientID = %d;", pPrefix, Item.m_ClientID);
                dbg_msg("network_in", "%s	int m_Angle = %d;", pPrefix, Item.m_Angle);
                dbg_msg("network_in", "%s	int m_HealthAmount = %d;", pPrefix, Item.m_HealthAmount);
                dbg_msg("network_in", "%s	int m_ArmorAmount = %d;", pPrefix, Item.m_ArmorAmount);
                dbg_msg("network_in", "%s	int m_Self = %d;", pPrefix, Item.m_Self);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PLAYERINFORACE)
        {
                CNetObj_PlayerInfoRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_PlayerInfoRace", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_RaceStartTick = %d;", pPrefix, Item.m_RaceStartTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATARACE)
        {
                CNetObj_GameDataRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetObj_GameDataRace", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	int TypeId = %d;", pPrefix, Type);
                dbg_msg("network_in", "%s	int Id     = %d;", pPrefix, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	int m_BestTime  = %d;", pPrefix, Item.m_BestTime);
                dbg_msg("network_in", "%s	int m_Precision = %d;", pPrefix, Item.m_Precision);
                dbg_msg("network_in", "%s	int m_RaceFlags = %d;", pPrefix, Item.m_RaceFlags);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        dbg_msg("network_in", "%s", pPrefix);
}
