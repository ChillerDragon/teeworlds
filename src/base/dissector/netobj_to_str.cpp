#include <generated/protocol.h>
#include <game/generated/protocol.h>

#include <base/dissector/compat.h>

#include "netobj_to_str.h"

const char* netobj_to_str(int Type)
{
	init_compat();
#ifdef _PROTOCOL_VERSION_6
	if(Type == NETOBJTYPE_EX) { return "OBJ_EX"; }
	if(Type == NETOBJTYPE_PLAYERINPUT) { return "OBJ_PLAYERINPUT"; }
	if(Type == NETOBJTYPE_PROJECTILE) { return "OBJ_PROJECTILE"; }
	if(Type == NETOBJTYPE_LASER) { return "OBJ_LASER"; }
	if(Type == NETOBJTYPE_PICKUP) { return "OBJ_PICKUP"; }
	if(Type == NETOBJTYPE_FLAG) { return "OBJ_FLAG"; }
	if(Type == NETOBJTYPE_GAMEINFO) { return "OBJ_GAMEINFO"; }
	if(Type == NETOBJTYPE_GAMEDATA) { return "OBJ_GAMEDATA"; }
	if(Type == NETOBJTYPE_CHARACTERCORE) { return "OBJ_CHARACTERCORE"; }
	if(Type == NETOBJTYPE_CHARACTER) { return "OBJ_CHARACTER"; }
	if(Type == NETOBJTYPE_PLAYERINFO) { return "OBJ_PLAYERINFO"; }
	if(Type == NETOBJTYPE_CLIENTINFO) { return "OBJ_CLIENTINFO"; }
	if(Type == NETOBJTYPE_SPECTATORINFO) { return "OBJ_SPECTATORINFO"; }
	if(Type == NETEVENTTYPE_COMMON) { return "EVENT_COMMON"; }
	if(Type == NETEVENTTYPE_EXPLOSION) { return "EVENT_EXPLOSION"; }
	if(Type == NETEVENTTYPE_SPAWN) { return "EVENT_SPAWN"; }
	if(Type == NETEVENTTYPE_HAMMERHIT) { return "EVENT_HAMMERHIT"; }
	if(Type == NETEVENTTYPE_DEATH) { return "EVENT_DEATH"; }
	if(Type == NETEVENTTYPE_SOUNDGLOBAL) { return "EVENT_SOUNDGLOBAL"; }
	if(Type == NETEVENTTYPE_SOUNDWORLD) { return "EVENT_SOUNDWORLD"; }
	if(Type == NETEVENTTYPE_DAMAGEIND) { return "EVENT_DAMAGEIND"; }
#else
	if(Type == NETOBJ_INVALID) { return "OBJ_INVALID"; }
	if(Type == NETOBJTYPE_PLAYERINPUT) { return "OBJ_PLAYERINPUT"; }
	if(Type == NETOBJTYPE_PROJECTILE) { return "OBJ_PROJECTILE"; }
	if(Type == NETOBJTYPE_LASER) { return "OBJ_LASER"; }
	if(Type == NETOBJTYPE_PICKUP) { return "OBJ_PICKUP"; }
	if(Type == NETOBJTYPE_FLAG) { return "OBJ_FLAG"; }
	if(Type == NETOBJTYPE_GAMEDATA) { return "OBJ_GAMEDATA"; }
	if(Type == NETOBJTYPE_GAMEDATATEAM) { return "OBJ_GAMEDATATEAM"; }
	if(Type == NETOBJTYPE_GAMEDATAFLAG) { return "OBJ_GAMEDATAFLAG"; }
	if(Type == NETOBJTYPE_CHARACTERCORE) { return "OBJ_CHARACTERCORE"; }
	if(Type == NETOBJTYPE_CHARACTER) { return "OBJ_CHARACTER"; }
	if(Type == NETOBJTYPE_PLAYERINFO) { return "OBJ_PLAYERINFO"; }
	if(Type == NETOBJTYPE_SPECTATORINFO) { return "OBJ_SPECTATORINFO"; }
	if(Type == NETOBJTYPE_DE_CLIENTINFO) { return "OBJ_DE_CLIENTINFO"; }
	if(Type == NETOBJTYPE_DE_GAMEINFO) { return "OBJ_DE_GAMEINFO"; }
	if(Type == NETOBJTYPE_DE_TUNEPARAMS) { return "OBJ_DE_TUNEPARAMS"; }
	if(Type == NETEVENTTYPE_COMMON) { return "EVENT_COMMON"; }
	if(Type == NETEVENTTYPE_EXPLOSION) { return "EVENT_EXPLOSION"; }
	if(Type == NETEVENTTYPE_SPAWN) { return "EVENT_SPAWN"; }
	if(Type == NETEVENTTYPE_HAMMERHIT) { return "EVENT_HAMMERHIT"; }
	if(Type == NETEVENTTYPE_DEATH) { return "EVENT_DEATH"; }
	if(Type == NETEVENTTYPE_SOUNDWORLD) { return "EVENT_SOUNDWORLD"; }
	if(Type == NETEVENTTYPE_DAMAGE) { return "EVENT_DAMAGE"; }
	if(Type == NETOBJTYPE_PLAYERINFORACE) { return "OBJ_PLAYERINFORACE"; }
	if(Type == NETOBJTYPE_GAMEDATARACE) { return "OBJ_GAMEDATARACE"; }
#endif
	return "unknown";
}
