#ifndef BASE_DISSECTOR_ITEM_PRINTER_H
#define BASE_DISSECTOR_ITEM_PRINTER_H

// pData has to be the FULL item
// containing its key, id and size
// keep in mind snap items are in the format <KEY><ID>[SIZE]<PAYLOAD>
// and size is only set for special items that were not included
// in the major release such as race extensions and ddnet extensions
void print_netobj_as_struct(const int *pData, const char *pPrefix = "  ");

#endif
