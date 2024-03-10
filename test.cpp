#include "src/flag_to_str.h"
#include "src/packer.h"

#include <cstdio>

int main()
{
	// unpack
	int Flags, Size, Sequence;
	unsigned char aHeader[3] = {0x40, 0x02, 0x26};
	chunk_header_unpack(aHeader, Flags, Size, Sequence);

	char aFlags[512];
	flag_chunk_to_str(Flags, aFlags, sizeof(aFlags));
	printf("flags=%s (%d) size=%d sequence=%d\n", aFlags, Flags, Size, Sequence);

	// // pack
	// Flags = 0;
	// Size = 107;
	// Sequence = -1;
	// chunk_header_pack(aHeader, Flags, Size, Sequence);
	// printf("%02x %02x %02x \n", aHeader[0], aHeader[1], aHeader[2]);
}
