// ChillerDragon 2022 - dissector

#include "compat.h"

short _gs_aItemSizes[_MAX_NETOBJSIZES];

void init_compat()
{
    static bool did_init_already = false;
    if(did_init_already)
        return;
    did_init_already = true;

    _gs_aItemSizes[0] = 0;
    _gs_aItemSizes[1] = 40;
    _gs_aItemSizes[2] = 24;
    _gs_aItemSizes[3] = 20;
    _gs_aItemSizes[4] = 12;
    _gs_aItemSizes[5] = 12;
    _gs_aItemSizes[6] = 12;
    _gs_aItemSizes[7] = 8;
    _gs_aItemSizes[8] = 16;
    _gs_aItemSizes[9] = 60;
    _gs_aItemSizes[10] = 88;
    _gs_aItemSizes[11] = 12;
    _gs_aItemSizes[12] = 16;
    _gs_aItemSizes[13] = 232;
    _gs_aItemSizes[14] = 20;
    _gs_aItemSizes[15] = 128;
    _gs_aItemSizes[16] = 8;
    _gs_aItemSizes[17] = 8;
    _gs_aItemSizes[18] = 8;
    _gs_aItemSizes[19] = 8;
    _gs_aItemSizes[20] = 12;
    _gs_aItemSizes[21] = 12;
    _gs_aItemSizes[22] = 28;
    /*
        // HACK: only set static size for items, which were available in the first 0.7 release
        // so new items don't break the snapshot delta
        static const int OLD_NUM_NETOBJTYPES = 23;
    */
    _gs_aItemSizes[23] = 0;
    _gs_aItemSizes[24] = 0;
    _gs_aItemSizes[25] = 0;
    _gs_aItemSizes[26] = 0;
    _gs_aItemSizes[27] = 0;
    _gs_aItemSizes[28] = 0;
    _gs_aItemSizes[29] = 0;
    _gs_aItemSizes[30] = 0;
    _gs_aItemSizes[31] = 0;
    _gs_aItemSizes[32] = 0;
    _gs_aItemSizes[33] = 0;
    _gs_aItemSizes[34] = 0;
    _gs_aItemSizes[35] = 0;
    _gs_aItemSizes[36] = 0;
    _gs_aItemSizes[37] = 0;
    _gs_aItemSizes[38] = 0;
    _gs_aItemSizes[39] = 0;
    _gs_aItemSizes[40] = 0;
    _gs_aItemSizes[41] = 0;
    _gs_aItemSizes[42] = 0;
    _gs_aItemSizes[43] = 0;
    _gs_aItemSizes[44] = 0;
    _gs_aItemSizes[45] = 0;
    _gs_aItemSizes[46] = 0;
    _gs_aItemSizes[47] = 0;
    _gs_aItemSizes[48] = 0;
    _gs_aItemSizes[49] = 0;
    _gs_aItemSizes[50] = 0;
    _gs_aItemSizes[51] = 0;
    _gs_aItemSizes[52] = 0;
    _gs_aItemSizes[53] = 0;
    _gs_aItemSizes[54] = 0;
    _gs_aItemSizes[55] = 0;
    _gs_aItemSizes[56] = 0;
    _gs_aItemSizes[57] = 0;
    _gs_aItemSizes[58] = 0;
    _gs_aItemSizes[59] = 0;
    _gs_aItemSizes[60] = 0;
    _gs_aItemSizes[61] = 0;
    _gs_aItemSizes[62] = 0;
    _gs_aItemSizes[63] = 0;
}
