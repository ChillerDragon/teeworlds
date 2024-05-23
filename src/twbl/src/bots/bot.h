#ifndef BOTS_BOT_H
#define BOTS_BOT_H

#include <game/collision.h>

typedef void (*FBotTick)(CCollision *pCollision);

extern "C" {

void BotTick(CCollision *pCollision);
}

#endif
