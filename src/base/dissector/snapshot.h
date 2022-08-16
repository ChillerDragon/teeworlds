#ifndef BASE_DISSECTOR_SNAPSHOT_H
#define BASE_DISSECTOR_SNAPSHOT_H

#include "compat.h"

int CSnapshotDelta_UnpackDelta(const class CSnapshot *pFrom, class CSnapshot *pTo, const void *pSrcData, int DataSize, const short *ppItemSizes);

void print_snapshot(int Msg,
    CUnpacker &Unpacker,
    const class CConfig *pConfig,
    int m_ReceivedSnapshots,
    unsigned int m_SnapshotParts,
    const class CSnapshotDelta &m_SnapshotDelta,
    int m_CurrentRecvTick,
#ifdef _PROTOCOL_VERSION_7
    const class CSnapshotStorage &m_SnapshotStorage,
#else
    class CSnapshotStorage &m_SnapshotStorage,
#endif
    int m_SnapCrcErrors,
    // const class CSmoothTime &GameTime,
    const char *m_aSnapshotIncomingData,
    class CSnapshotStorage::CHolder **m_aSnapshots, // TODO: const this
    class CClient *pClient);

#endif
