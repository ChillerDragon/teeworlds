#include "enum_to_str.h"

#include <generated/protocol.h>

const char *enum_pickup_to_str(int Value)
{
	if(Value == PICKUP_HEALTH) { return "PICKUP_HEALTH"; }
	if(Value == PICKUP_ARMOR) { return "PICKUP_ARMOR"; }
	if(Value == PICKUP_GRENADE) { return "PICKUP_GRENADE"; }
	if(Value == PICKUP_SHOTGUN) { return "PICKUP_SHOTGUN"; }
	if(Value == PICKUP_LASER) { return "PICKUP_LASER"; }
	if(Value == PICKUP_NINJA) { return "PICKUP_NINJA"; }
	if(Value == PICKUP_GUN) { return "PICKUP_GUN"; }
	if(Value == PICKUP_HAMMER) { return "PICKUP_HAMMER"; }
	if(Value == NUM_PICKUPS) { return "NUM_PICKUPS"; }
	return "(unknown enum value)";
}

const char *enum_emote_to_str(int Value)
{
	if(Value == EMOTE_NORMAL) { return "EMOTE_NORMAL"; }
	if(Value == EMOTE_PAIN) { return "EMOTE_PAIN"; }
	if(Value == EMOTE_HAPPY) { return "EMOTE_HAPPY"; }
	if(Value == EMOTE_SURPRISE) { return "EMOTE_SURPRISE"; }
	if(Value == EMOTE_ANGRY) { return "EMOTE_ANGRY"; }
	if(Value == EMOTE_BLINK) { return "EMOTE_BLINK"; }
	if(Value == NUM_EMOTES) { return "NUM_EMOTES"; }
	return "(unknown enum value)";
}

const char *enum_emoticon_to_str(int Value)
{
	if(Value == EMOTICON_OOP) { return "EMOTICON_OOP"; }
	if(Value == EMOTICON_EXCLAMATION) { return "EMOTICON_EXCLAMATION"; }
	if(Value == EMOTICON_HEARTS) { return "EMOTICON_HEARTS"; }
	if(Value == EMOTICON_DROP) { return "EMOTICON_DROP"; }
	if(Value == EMOTICON_DOTDOT) { return "EMOTICON_DOTDOT"; }
	if(Value == EMOTICON_MUSIC) { return "EMOTICON_MUSIC"; }
	if(Value == EMOTICON_SORRY) { return "EMOTICON_SORRY"; }
	if(Value == EMOTICON_GHOST) { return "EMOTICON_GHOST"; }
	if(Value == EMOTICON_SUSHI) { return "EMOTICON_SUSHI"; }
	if(Value == EMOTICON_SPLATTEE) { return "EMOTICON_SPLATTEE"; }
	if(Value == EMOTICON_DEVILTEE) { return "EMOTICON_DEVILTEE"; }
	if(Value == EMOTICON_ZOMG) { return "EMOTICON_ZOMG"; }
	if(Value == EMOTICON_ZZZ) { return "EMOTICON_ZZZ"; }
	if(Value == EMOTICON_WTF) { return "EMOTICON_WTF"; }
	if(Value == EMOTICON_EYES) { return "EMOTICON_EYES"; }
	if(Value == EMOTICON_QUESTION) { return "EMOTICON_QUESTION"; }
	if(Value == NUM_EMOTICONS) { return "NUM_EMOTICONS"; }
	return "(unknown enum value)";
}

const char *enum_vote_to_str(int Value)
{
	if(Value == VOTE_UNKNOWN) { return "VOTE_UNKNOWN"; }
	if(Value == VOTE_START_OP) { return "VOTE_START_OP"; }
	if(Value == VOTE_START_KICK) { return "VOTE_START_KICK"; }
	if(Value == VOTE_START_SPEC) { return "VOTE_START_SPEC"; }
	if(Value == VOTE_END_ABORT) { return "VOTE_END_ABORT"; }
	if(Value == VOTE_END_PASS) { return "VOTE_END_PASS"; }
	if(Value == VOTE_END_FAIL) { return "VOTE_END_FAIL"; }
	if(Value == NUM_VOTES) { return "NUM_VOTES"; }
	return "(unknown enum value)";
}

const char *enum_chat_to_str(int Value)
{
	if(Value == CHAT_NONE) { return "CHAT_NONE"; }
	if(Value == CHAT_ALL) { return "CHAT_ALL"; }
	if(Value == CHAT_TEAM) { return "CHAT_TEAM"; }
	if(Value == CHAT_WHISPER) { return "CHAT_WHISPER"; }
	if(Value == NUM_CHATS) { return "NUM_CHATS"; }
	return "(unknown enum value)";
}

const char *enum_gamemsg_to_str(int Value)
{
	if(Value == GAMEMSG_TEAM_SWAP) { return "GAMEMSG_TEAM_SWAP"; }
	if(Value == GAMEMSG_SPEC_INVALID_ID) { return "GAMEMSG_SPEC_INVALID_ID"; }
	if(Value == GAMEMSG_TEAM_SHUFFLE) { return "GAMEMSG_TEAM_SHUFFLE"; }
	if(Value == GAMEMSG_TEAM_BALANCE) { return "GAMEMSG_TEAM_BALANCE"; }
	if(Value == GAMEMSG_CTF_DROP) { return "GAMEMSG_CTF_DROP"; }
	if(Value == GAMEMSG_CTF_RETURN) { return "GAMEMSG_CTF_RETURN"; }
	if(Value == GAMEMSG_TEAM_ALL) { return "GAMEMSG_TEAM_ALL"; }
	if(Value == GAMEMSG_TEAM_BALANCE_VICTIM) { return "GAMEMSG_TEAM_BALANCE_VICTIM"; }
	if(Value == GAMEMSG_CTF_GRAB) { return "GAMEMSG_CTF_GRAB"; }
	if(Value == GAMEMSG_CTF_CAPTURE) { return "GAMEMSG_CTF_CAPTURE"; }
	if(Value == GAMEMSG_GAME_PAUSED) { return "GAMEMSG_GAME_PAUSED"; }
	if(Value == NUM_GAMEMSGS) { return "NUM_GAMEMSGS"; }
	return "(unknown enum value)";
}

const char *enum_team_to_str(int Value)
{
	if(Value == TEAM_SPECTATORS) { return "TEAM_SPECTATORS"; }
	if(Value == TEAM_RED) { return "TEAM_RED"; }
	if(Value == TEAM_BLUE) { return "TEAM_BLUE"; }
	if(Value == NUM_TEAMS) { return "NUM_TEAMS"; }
	return "(unknown enum value)";
}

const char *enum_sound_to_str(int Value)
{
	if(Value == SOUND_GUN_FIRE) { return "SOUND_GUN_FIRE"; }
	if(Value == SOUND_SHOTGUN_FIRE) { return "SOUND_SHOTGUN_FIRE"; }
	if(Value == SOUND_GRENADE_FIRE) { return "SOUND_GRENADE_FIRE"; }
	if(Value == SOUND_HAMMER_FIRE) { return "SOUND_HAMMER_FIRE"; }
	if(Value == SOUND_HAMMER_HIT) { return "SOUND_HAMMER_HIT"; }
	if(Value == SOUND_NINJA_FIRE) { return "SOUND_NINJA_FIRE"; }
	if(Value == SOUND_GRENADE_EXPLODE) { return "SOUND_GRENADE_EXPLODE"; }
	if(Value == SOUND_NINJA_HIT) { return "SOUND_NINJA_HIT"; }
	if(Value == SOUND_LASER_FIRE) { return "SOUND_LASER_FIRE"; }
	if(Value == SOUND_LASER_BOUNCE) { return "SOUND_LASER_BOUNCE"; }
	if(Value == SOUND_WEAPON_SWITCH) { return "SOUND_WEAPON_SWITCH"; }
	if(Value == SOUND_PLAYER_PAIN_SHORT) { return "SOUND_PLAYER_PAIN_SHORT"; }
	if(Value == SOUND_PLAYER_PAIN_LONG) { return "SOUND_PLAYER_PAIN_LONG"; }
	if(Value == SOUND_BODY_LAND) { return "SOUND_BODY_LAND"; }
	if(Value == SOUND_PLAYER_AIRJUMP) { return "SOUND_PLAYER_AIRJUMP"; }
	if(Value == SOUND_PLAYER_JUMP) { return "SOUND_PLAYER_JUMP"; }
	if(Value == SOUND_PLAYER_DIE) { return "SOUND_PLAYER_DIE"; }
	if(Value == SOUND_PLAYER_SPAWN) { return "SOUND_PLAYER_SPAWN"; }
	if(Value == SOUND_PLAYER_SKID) { return "SOUND_PLAYER_SKID"; }
	if(Value == SOUND_TEE_CRY) { return "SOUND_TEE_CRY"; }
	if(Value == SOUND_HOOK_LOOP) { return "SOUND_HOOK_LOOP"; }
	if(Value == SOUND_HOOK_ATTACH_GROUND) { return "SOUND_HOOK_ATTACH_GROUND"; }
	if(Value == SOUND_HOOK_ATTACH_PLAYER) { return "SOUND_HOOK_ATTACH_PLAYER"; }
	if(Value == SOUND_HOOK_NOATTACH) { return "SOUND_HOOK_NOATTACH"; }
	if(Value == SOUND_PICKUP_HEALTH) { return "SOUND_PICKUP_HEALTH"; }
	if(Value == SOUND_PICKUP_ARMOR) { return "SOUND_PICKUP_ARMOR"; }
	if(Value == SOUND_PICKUP_GRENADE) { return "SOUND_PICKUP_GRENADE"; }
	if(Value == SOUND_PICKUP_SHOTGUN) { return "SOUND_PICKUP_SHOTGUN"; }
	if(Value == SOUND_PICKUP_NINJA) { return "SOUND_PICKUP_NINJA"; }
	if(Value == SOUND_WEAPON_SPAWN) { return "SOUND_WEAPON_SPAWN"; }
	if(Value == SOUND_WEAPON_NOAMMO) { return "SOUND_WEAPON_NOAMMO"; }
	if(Value == SOUND_HIT) { return "SOUND_HIT"; }
	if(Value == SOUND_CHAT_SERVER) { return "SOUND_CHAT_SERVER"; }
	if(Value == SOUND_CHAT_CLIENT) { return "SOUND_CHAT_CLIENT"; }
	if(Value == SOUND_CHAT_HIGHLIGHT) { return "SOUND_CHAT_HIGHLIGHT"; }
	if(Value == SOUND_CTF_DROP) { return "SOUND_CTF_DROP"; }
	if(Value == SOUND_CTF_RETURN) { return "SOUND_CTF_RETURN"; }
	if(Value == SOUND_CTF_GRAB_PL) { return "SOUND_CTF_GRAB_PL"; }
	if(Value == SOUND_CTF_GRAB_EN) { return "SOUND_CTF_GRAB_EN"; }
	if(Value == SOUND_CTF_CAPTURE) { return "SOUND_CTF_CAPTURE"; }
	if(Value == SOUND_MENU) { return "SOUND_MENU"; }
	if(Value == NUM_SOUNDS) { return "NUM_SOUNDS"; }
	return "(unknown enum value)";
}

const char *enum_weapon_to_str(int Value)
{
	if(Value == WEAPON_HAMMER) { return "WEAPON_HAMMER"; }
	if(Value == WEAPON_GUN) { return "WEAPON_GUN"; }
	if(Value == WEAPON_SHOTGUN) { return "WEAPON_SHOTGUN"; }
	if(Value == WEAPON_GRENADE) { return "WEAPON_GRENADE"; }
	if(Value == WEAPON_LASER) { return "WEAPON_LASER"; }
	if(Value == WEAPON_NINJA) { return "WEAPON_NINJA"; }
	if(Value == NUM_WEAPONS) { return "NUM_WEAPONS"; }
	return "(unknown enum value)";
}

