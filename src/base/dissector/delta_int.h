#ifndef BASE_DISSECTOR_DELTA_HEX_H
#define BASE_DISSECTOR_DELTA_HEX_H

// dump decompressed snapshot payload
// given an array of integers (unpacked netints payload of NETMSG_SNAP)
// it will list all the integers and their meaning
int snapshot_delta_intdump(
    const class CSnapshot *pFrom,
    class CSnapshot *pTo,
    const void *pSrcData,
    int DataSize,
    const short *ppItemSizes,
    bool Sixup);

#endif
