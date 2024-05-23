#include <cstdio>

#include <bots/bot.h>

extern "C" {

void BotTick(CCollision *pCollision)
{
	printf(
		"bot tick tile(0, 0)=%d tile(1, 1)=%d\n",
		pCollision->GetTile(0, 0),
		pCollision->GetTile(1, 1));
	puts("hello");

	pCollision->SetTile(0, 0, 99);
}

}
