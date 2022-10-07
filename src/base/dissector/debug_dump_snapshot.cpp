#include <engine/shared/snapshot.h>

#include "debug_dump_snapshot.h"

#include "num_to_str.h"

void debug_dump(CSnapshot *pSnapShot)
{
	dbg_msg("snapshot", "data_size=%d num_items=%d", NULL, pSnapShot->NumItems());
	for(int i = 0; i < pSnapShot->NumItems(); i++)
	{
		CSnapshotItem *pItem = pSnapShot->GetItem(i);
		int Size = pSnapShot->GetItemSize(i);
    char aType[128];
		netobj_to_str(pItem->Type(), aType, sizeof(aType));
		dbg_msg("snapshot", "\ttype=%d (%s)  id=%d", pItem->Type(), aType, pItem->ID());
		for(int b = 0; b < Size / 4; b++)
			dbg_msg("snapshot", "\t\t%3d %12d\t%08x", b, pItem->Data()[b], pItem->Data()[b]);
	}
}
