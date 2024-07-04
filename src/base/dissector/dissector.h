#ifndef BASE_DISSECTOR_DISSECTOR_H
#define BASE_DISSECTOR_DISSECTOR_H

#include "byte_printer.h"

#include <engine/shared/protocol.h>

enum ENetDirection {
    NETWORK_IN,
    NETWORK_OUT
};

bool show_addr(const NETADDR *pAddr);
void print_packet(class CNetPacketConstruct *pPacket, unsigned char *pPacketData, int PacketSize, const NETADDR *pAddr, ENetDirection Direction, const class CConfig *pConfig);

const char *netmsg_to_s(int Msg);
void print_state(const char *type, const char *note, int State);

#endif
