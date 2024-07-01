#include "base/system.h"
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
                dbg_msg("network_in", "%s%sstruct%s CNetObj_PlayerInput Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Direction     = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Direction, TERM_RESET);
                dbg_msg("network_in", "%s	.m_TargetX       = %s%d%s,", pPrefix, TERM_BLUE, Item.m_TargetX, TERM_RESET);
                dbg_msg("network_in", "%s	.m_TargetY       = %s%d%s,", pPrefix, TERM_BLUE, Item.m_TargetY, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Jump      = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Jump, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Fire      = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Fire, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Hook      = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Hook, TERM_RESET);
                dbg_msg("network_in", "%s	.m_PlayerFlags       = %s%d%s,", pPrefix, TERM_BLUE, Item.m_PlayerFlags, TERM_RESET);
                dbg_msg("network_in", "%s	.m_WantedWeapon      = %s%d%s,", pPrefix, TERM_BLUE, Item.m_WantedWeapon, TERM_RESET);
                dbg_msg("network_in", "%s	.m_NextWeapon        = %s%d%s,", pPrefix, TERM_BLUE, Item.m_NextWeapon, TERM_RESET);
                dbg_msg("network_in", "%s	.m_PrevWeapon        = %s%d%s,", pPrefix, TERM_BLUE, Item.m_PrevWeapon, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PROJECTILE)
        {
                CNetObj_Projectile Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Projectile Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s	.m_VelX = %s%d%s,", pPrefix, TERM_BLUE, Item.m_VelX, TERM_RESET);
                dbg_msg("network_in", "%s	.m_VelY = %s%d%s,", pPrefix, TERM_BLUE, Item.m_VelY, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Type = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Type, TERM_RESET);
                dbg_msg("network_in", "%s	.m_StartTick = %s%d%s,", pPrefix, TERM_BLUE, Item.m_StartTick, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_LASER)
        {
                CNetObj_Laser Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Laser Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s	.m_FromX = %s%d%s,", pPrefix, TERM_BLUE, Item.m_FromX, TERM_RESET);
                dbg_msg("network_in", "%s	.m_FromY = %s%d%s,", pPrefix, TERM_BLUE, Item.m_FromY, TERM_RESET);
                dbg_msg("network_in", "%s	.m_StartTick = %s%d%s,", pPrefix, TERM_BLUE, Item.m_StartTick, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PICKUP)
        {
                CNetObj_Pickup Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Pickup Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X    = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y    = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Type = %s%d%s; %s// %s%s", pPrefix, TERM_BLUE, Item.m_Type, TERM_RESET, TERM_GREEN, enum_pickup_to_str(Item.m_Type), TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_FLAG)
        {
                CNetObj_Flag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Flag Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Team = %s%d%s; %s// %s%s", pPrefix, TERM_BLUE, Item.m_Team, TERM_RESET, TERM_GREEN, enum_team_to_str(Item.m_Team), TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATA)
        {
                CNetObj_GameData Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameData Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_GameStartTick    = %s%d%s,", pPrefix, TERM_BLUE, Item.m_GameStartTick, TERM_RESET);
                dbg_msg("network_in", "%s	.m_GameStateFlags   = %s%d%s,", pPrefix, TERM_BLUE, Item.m_GameStateFlags, TERM_RESET);
                dbg_msg("network_in", "%s	.m_GameStateEndTick = %s%d%s,", pPrefix, TERM_BLUE, Item.m_GameStateEndTick, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATATEAM)
        {
                CNetObj_GameDataTeam Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameDataTeam Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_TeamscoreRed = %s%d%s,", pPrefix, TERM_BLUE, Item.m_TeamscoreRed, TERM_RESET);
                dbg_msg("network_in", "%s	.m_TeamscoreBlue = %s%d%s,", pPrefix, TERM_BLUE, Item.m_TeamscoreBlue, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATAFLAG)
        {
                CNetObj_GameDataFlag Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameDataFlag Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_FlagCarrierRed = %s%d%s,", pPrefix, TERM_BLUE, Item.m_FlagCarrierRed, TERM_RESET);
                dbg_msg("network_in", "%s	.m_FlagCarrierBlue = %s%d%s,", pPrefix, TERM_BLUE, Item.m_FlagCarrierBlue, TERM_RESET);
                dbg_msg("network_in", "%s	.m_FlagDropTickRed = %s%d%s,", pPrefix, TERM_BLUE, Item.m_FlagDropTickRed, TERM_RESET);
                dbg_msg("network_in", "%s	.m_FlagDropTickBlue = %s%d%s,", pPrefix, TERM_BLUE, Item.m_FlagDropTickBlue, TERM_RESET);
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
                dbg_msg("network_in", "%s%sstruct%s CNetObj_Character Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_Tick = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Tick, TERM_RESET);
                dbg_msg("network_in", "%s	.m_X = %s%d%s; %s// %d tiles (divided by 32)%s", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET, TERM_GREEN, Item.m_X / 32, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s; %s// %d tiles (divided by 32)%s", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET, TERM_GREEN, Item.m_Y / 32, TERM_RESET);
                dbg_msg("network_in", "%s	.m_VelX = %s%d%s,", pPrefix, TERM_BLUE, Item.m_VelX, TERM_RESET);
                dbg_msg("network_in", "%s	.m_VelY = %s%d%s,", pPrefix, TERM_BLUE, Item.m_VelY, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Angle = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Angle, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Direction = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Direction, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Jumped = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Jumped, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HookedPlayer = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HookedPlayer, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HookState = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HookState, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HookTick = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HookTick, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HookX = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HookX, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HookY = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HookY, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HookDx = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HookDx, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HookDy = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HookDy, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_Health = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Health, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Armor = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Armor, TERM_RESET);
                dbg_msg("network_in", "%s	.m_AmmoCount = %s%d%s,", pPrefix, TERM_BLUE, Item.m_AmmoCount, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Weapon = %s%d%s; %s// %s%s", pPrefix, TERM_BLUE, Item.m_Weapon, TERM_RESET, TERM_GREEN, enum_weapon_to_str(Item.m_Weapon), TERM_RESET);
                dbg_msg("network_in", "%s	.m_Emote = %s%d%s; %s// %s%s", pPrefix, TERM_BLUE, Item.m_Emote, TERM_RESET, TERM_GREEN, enum_emote_to_str(Item.m_Emote), TERM_RESET);
                dbg_msg("network_in", "%s	.m_AttackTick = %s%d%s,", pPrefix, TERM_BLUE, Item.m_AttackTick, TERM_RESET);
                dbg_msg("network_in", "%s	.m_TriggeredEvents = %s%d%s,", pPrefix, TERM_BLUE, Item.m_TriggeredEvents, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PLAYERINFO)
        {
                CNetObj_PlayerInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_PlayerInfo Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_PlayerFlags = %s%d%s,", pPrefix, TERM_BLUE, Item.m_PlayerFlags, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Score       = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Score, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Latency     = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Latency, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_SPECTATORINFO)
        {
                CNetObj_SpectatorInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_SpectatorInfo Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_SpecMode = %s%d%s,", pPrefix, TERM_BLUE, Item.m_SpecMode, TERM_RESET);
                dbg_msg("network_in", "%s	.m_SpectatorID = %s%d%s,", pPrefix, TERM_BLUE, Item.m_SpectatorID, TERM_RESET);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_CLIENTINFO)
        {
                CNetObj_De_ClientInfo Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_De_ClientInfo Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
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
                dbg_msg("network_in", "%s%sstruct%s CNetObj_De_GameInfo Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_GameFlags = %s%d%s,", pPrefix, TERM_BLUE, Item.m_GameFlags, TERM_RESET);
                dbg_msg("network_in", "%s	.m_ScoreLimit = %s%d%s,", pPrefix, TERM_BLUE, Item.m_ScoreLimit, TERM_RESET);
                dbg_msg("network_in", "%s	.m_TimeLimit = %s%d%s,", pPrefix, TERM_BLUE, Item.m_TimeLimit, TERM_RESET);
                dbg_msg("network_in", "%s	.m_MatchNum = %s%d%s,", pPrefix, TERM_BLUE, Item.m_MatchNum, TERM_RESET);
                dbg_msg("network_in", "%s	.m_MatchCurrent = %s%d%s,", pPrefix, TERM_BLUE, Item.m_MatchCurrent, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_DE_TUNEPARAMS)
        {
                // CNetObj_De_TuneParams Item;
                // mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_De_TuneParams Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
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
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_EXPLOSION)
        {
                CNetEvent_Explosion Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Explosion : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_SPAWN)
        {
                CNetEvent_Spawn Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Spawn : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_HAMMERHIT)
        {
                CNetEvent_HammerHit Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_HammerHit : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_DEATH)
        {
                CNetEvent_Death Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Death : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s	.m_ClientID = %s%d%s,", pPrefix, TERM_BLUE, Item.m_ClientID, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_SOUNDWORLD)
        {
                CNetEvent_SoundWorld Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_SoundWorld : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s	.m_SoundID = %s%d%s; %s// %s%s", pPrefix, TERM_BLUE, Item.m_SoundID, TERM_RESET, TERM_GREEN, enum_sound_to_str(Item.m_SoundID), TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETEVENTTYPE_DAMAGE)
        {
                CNetEvent_Damage Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%sstruct CNetEvent_Damage : public CNetEvent_Common", pPrefix);
                dbg_msg("network_in", "%s{", pPrefix);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_X = %s%d%s,", pPrefix, TERM_BLUE, Item.m_X, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Y = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Y, TERM_RESET);
                dbg_msg("network_in", "%s	.m_ClientID = %s%d%s,", pPrefix, TERM_BLUE, Item.m_ClientID, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Angle = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Angle, TERM_RESET);
                dbg_msg("network_in", "%s	.m_HealthAmount = %s%d%s,", pPrefix, TERM_BLUE, Item.m_HealthAmount, TERM_RESET);
                dbg_msg("network_in", "%s	.m_ArmorAmount = %s%d%s,", pPrefix, TERM_BLUE, Item.m_ArmorAmount, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Self = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Self, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_PLAYERINFORACE)
        {
                int ItemSize = (*pData++) * 4;
                CNetObj_PlayerInfoRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_PlayerInfoRace Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s	.Size   = %s%d%s,", pPrefix, TERM_BLUE, ItemSize, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_RaceStartTick = %s%d%s,", pPrefix, TERM_BLUE, Item.m_RaceStartTick, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        else if(Type == NETOBJTYPE_GAMEDATARACE)
        {
                int ItemSize = (*pData++) * 4;
                CNetObj_GameDataRace Item;
                mem_copy(&Item, pData, sizeof(Item));
                dbg_msg("network_in", "%s%sstruct%s CNetObj_GameDataRace Item = {", pPrefix, TERM_MAGENTA, TERM_RESET);
                dbg_msg("network_in", "%s	.TypeId = %s%d%s,", pPrefix, TERM_BLUE, Type, TERM_RESET);
                dbg_msg("network_in", "%s	.Id     = %s%d%s,", pPrefix, TERM_BLUE, Id, TERM_RESET);
                dbg_msg("network_in", "%s	.Size   = %s%d%s,", pPrefix, TERM_BLUE, ItemSize, TERM_RESET);
                dbg_msg("network_in", "%s", pPrefix);
                dbg_msg("network_in", "%s	.m_BestTime  = %s%d%s,", pPrefix, TERM_BLUE, Item.m_BestTime, TERM_RESET);
                dbg_msg("network_in", "%s	.m_Precision = %s%d%s,", pPrefix, TERM_BLUE, Item.m_Precision, TERM_RESET);
                dbg_msg("network_in", "%s	.m_RaceFlags = %s%d%s,", pPrefix, TERM_BLUE, Item.m_RaceFlags, TERM_RESET);
                dbg_msg("network_in", "%s};", pPrefix);
        }
        dbg_msg("network_in", "%s", pPrefix);
}
