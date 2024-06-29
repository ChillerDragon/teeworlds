#include <engine/shared/protocol.h>
#include <engine/shared/network.h>

#if __has_include(<generated/protocol.h>)
#include <generated/protocol.h>
#endif

#include <base/dissector/enum_to_str.h>
#include <base/dissector/color.h>

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
                dbg_msg("network_in", "%s%sstruct%s CNetObj_PlayerInput", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_Direction     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Direction);
                dbg_msg("network_in", "%s	%sint%s m_TargetX       = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_TargetX);
                dbg_msg("network_in", "%s	%sint%s m_TargetY       = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_TargetY);
                dbg_msg("network_in", "%s	%sint%s m_Jump      = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Jump);
                dbg_msg("network_in", "%s	%sint%s m_Fire      = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Fire);
                dbg_msg("network_in", "%s	%sint%s m_Hook      = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Hook);
                dbg_msg("network_in", "%s	%sint%s m_PlayerFlags       = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_PlayerFlags);
                dbg_msg("network_in", "%s	%sint%s m_WantedWeapon      = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_WantedWeapon);
                dbg_msg("network_in", "%s	%sint%s m_NextWeapon        = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_NextWeapon);
                dbg_msg("network_in", "%s	%sint%s m_PrevWeapon        = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_PrevWeapon);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PROJECTILE)
        {
                CNetObj_Projectile Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Projectile", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s	%sint%s m_VelX = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_VelX);
                dbg_msg("network_in", "%s	%sint%s m_VelY = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_VelY);
                dbg_msg("network_in", "%s	%sint%s m_Type = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Type);
                dbg_msg("network_in", "%s	%sint%s m_StartTick = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_StartTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_LASER)
        {
                CNetObj_Laser Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Laser", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s	%sint%s m_FromX = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_FromX);
                dbg_msg("network_in", "%s	%sint%s m_FromY = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_FromY);
                dbg_msg("network_in", "%s	%sint%s m_StartTick = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_StartTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PICKUP)
        {
                CNetObj_Pickup Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Pickup", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X    = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y    = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s	%sint%s m_Type = %d; // %s", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Type, enum_pickup_to_str(Item.m_Type));
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_FLAG)
        {
                CNetObj_Flag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Flag", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s	%sint%s m_Team = %d; // %s", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Team, enum_team_to_str(Item.m_Team));
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATA)
        {
                CNetObj_GameData Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameData", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_GameStartTick    = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_GameStartTick);
                dbg_msg("network_in", "%s	%sint%s m_GameStateFlags   = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_GameStateFlags);
                dbg_msg("network_in", "%s	%sint%s m_GameStateEndTick = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_GameStateEndTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATATEAM)
        {
                CNetObj_GameDataTeam Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameDataTeam", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_TeamscoreRed = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_TeamscoreRed);
                dbg_msg("network_in", "%s	%sint%s m_TeamscoreBlue = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_TeamscoreBlue);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATAFLAG)
        {
                CNetObj_GameDataFlag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameDataFlag", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_FlagCarrierRed = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_FlagCarrierRed);
                dbg_msg("network_in", "%s	%sint%s m_FlagCarrierBlue = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_FlagCarrierBlue);
                dbg_msg("network_in", "%s	%sint%s m_FlagDropTickRed = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_FlagDropTickRed);
                dbg_msg("network_in", "%s	%sint%s m_FlagDropTickBlue = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_FlagDropTickBlue);
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
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Character", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_Tick = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Tick);
                dbg_msg("network_in", "%s	%sint%s m_X = %d; %s// tile %d%s", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X, TERM_GREEN, Item.m_X / 32, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d; %s// tile %d%s", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y, TERM_GREEN, Item.m_Y / 32, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_VelX = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_VelX);
                dbg_msg("network_in", "%s	%sint%s m_VelY = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_VelY);
                dbg_msg("network_in", "%s	%sint%s m_Angle = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Angle);
                dbg_msg("network_in", "%s	%sint%s m_Direction = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Direction);
                dbg_msg("network_in", "%s	%sint%s m_Jumped = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Jumped);
                dbg_msg("network_in", "%s	%sint%s m_HookedPlayer = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HookedPlayer);
                dbg_msg("network_in", "%s	%sint%s m_HookState = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HookState);
                dbg_msg("network_in", "%s	%sint%s m_HookTick = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HookTick);
                dbg_msg("network_in", "%s	%sint%s m_HookX = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HookX);
                dbg_msg("network_in", "%s	%sint%s m_HookY = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HookY);
                dbg_msg("network_in", "%s	%sint%s m_HookDx = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HookDx);
                dbg_msg("network_in", "%s	%sint%s m_HookDy = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HookDy);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_Health = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Health);
                dbg_msg("network_in", "%s	%sint%s m_Armor = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Armor);
                dbg_msg("network_in", "%s	%sint%s m_AmmoCount = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_AmmoCount);
                dbg_msg("network_in", "%s	%sint%s m_Weapon = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Weapon);
                dbg_msg("network_in", "%s	%sint%s m_Emote = %d; %s// %s%s", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Emote, TERM_GREEN, enum_emote_to_str(Item.m_Emote), TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_AttackTick = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_AttackTick);
                dbg_msg("network_in", "%s	%sint%s m_TriggeredEvents = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_TriggeredEvents);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PLAYERINFO)
        {
                CNetObj_PlayerInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_PlayerInfo", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_PlayerFlags = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_PlayerFlags);
                dbg_msg("network_in", "%s	%sint%s m_Score       = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Score);
                dbg_msg("network_in", "%s	%sint%s m_Latency     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Latency);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_SPECTATORINFO)
        {
                CNetObj_SpectatorInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_SpectatorInfo", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_SpecMode = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_SpecMode);
                dbg_msg("network_in", "%s	%sint%s m_SpectatorID = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_SpectatorID);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_CLIENTINFO)
        {
                CNetObj_De_ClientInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_De_ClientInfo", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_Local;", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_Team;", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_aName[4];", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_aClan[3];", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_Country;", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_aaSkinPartNames[6][6];", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_aUseCustomColors[6];", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s	%sint%s m_aSkinPartColors[6];", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_GAMEINFO)
        {
                CNetObj_De_GameInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_De_GameInfo", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_GameFlags = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_GameFlags);
                dbg_msg("network_in", "%s	%sint%s m_ScoreLimit = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_ScoreLimit);
                dbg_msg("network_in", "%s	%sint%s m_TimeLimit = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_TimeLimit);
                dbg_msg("network_in", "%s	%sint%s m_MatchNum = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_MatchNum);
                dbg_msg("network_in", "%s	%sint%s m_MatchCurrent = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_MatchCurrent);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_TUNEPARAMS)
        {
                // CNetObj_De_TuneParams Item;
                // mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_De_TuneParams", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_aTuneParams[32];", pPrefix, TERM_BLUE, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_COMMON)
        {
                CNetEvent_Common Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_EXPLOSION)
        {
                CNetEvent_Explosion Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Explosion : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_SPAWN)
        {
                CNetEvent_Spawn Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Spawn : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_HAMMERHIT)
        {
                CNetEvent_HammerHit Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_HammerHit : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_DEATH)
        {
                CNetEvent_Death Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Death : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s	%sint%s m_ClientID = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_ClientID);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_SOUNDWORLD)
        {
                CNetEvent_SoundWorld Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_SoundWorld : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s	%sint%s m_SoundID = %d; %s// %s%s", pPrefix, TERM_BLUE, TERM_RESET, Item.m_SoundID, TERM_GREEN, enum_sound_to_str(Item.m_SoundID), TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_DAMAGE)
        {
                CNetEvent_Damage Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Damage : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_X = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_X);
                dbg_msg("network_in", "%s	%sint%s m_Y = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Y);
                dbg_msg("network_in", "%s	%sint%s m_ClientID = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_ClientID);
                dbg_msg("network_in", "%s	%sint%s m_Angle = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Angle);
                dbg_msg("network_in", "%s	%sint%s m_HealthAmount = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_HealthAmount);
                dbg_msg("network_in", "%s	%sint%s m_ArmorAmount = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_ArmorAmount);
                dbg_msg("network_in", "%s	%sint%s m_Self = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Self);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PLAYERINFORACE)
        {
                CNetObj_PlayerInfoRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_PlayerInfoRace", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_RaceStartTick = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_RaceStartTick);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATARACE)
        {
                CNetObj_GameDataRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameDataRace", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	%sint%s TypeId = %d;", pPrefix, TERM_BLUE, TERM_RESET, Type);
                dbg_msg("network_in", "%s	%sint%s Id     = %d;", pPrefix, TERM_BLUE, TERM_RESET, Id);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	%sint%s m_BestTime  = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_BestTime);
                dbg_msg("network_in", "%s	%sint%s m_Precision = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_Precision);
                dbg_msg("network_in", "%s	%sint%s m_RaceFlags = %d;", pPrefix, TERM_BLUE, TERM_RESET, Item.m_RaceFlags);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        dbg_msg("network_in", "%s", pPrefix);
}
