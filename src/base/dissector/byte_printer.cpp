// ChillerDragon 2022 - dissector

#include "byte_printer.h"

void str_raw(char *dst, int dst_size, const void *data, int data_size)
{
	int i;
	const unsigned char * pdat = (const unsigned char *)data;
	for(i = 0; i < data_size && i < dst_size - 1; i++)
		dst[i] = (pdat[i] < 32 || pdat[i] > 126) ? '.' : pdat[i];
	dst[i] = '\0';
}

void print_raw(const char *sys, const char *prefix, const void *data, int data_size)
{
	char aRaw[1024];
	str_raw(aRaw, sizeof(aRaw), data, data_size);
	dbg_msg(sys, "%s%s", prefix, aRaw);
}

void str_hex_spaced(char *dst, int dst_size, const void *data, int data_size)
{
	int i;
	char aChunk[64];
	dst[0] = '\0';
	for(i = 0; i < data_size && i < dst_size/4-4; i+=4)
	{
		str_hex(aChunk, sizeof(aChunk), (const unsigned char *)data + i, 4);
		str_append(dst, aChunk, dst_size);
		str_append(dst, " ", dst_size);
	}
}

void str_hex_highlighted(char *dst, int dst_size, const void *data, int data_size, int from, int to)
{
	static const char hex[] = "0123456789ABCDEF";
	int b;
	int i = 0;

	for(b = 0; b < data_size && b < dst_size/4-4; b++)
	{
		dst[b*4] = (i == from) ? '<' : ' ';
		dst[b*4+1] = hex[((const unsigned char *)data)[b]>>4];
		dst[b*4+2] = hex[((const unsigned char *)data)[b]&0xf];
		dst[b*4+3] = (i == to) ? '>' : ' ';
		dst[b*4+4] = 0;
		++i;
	}
}

void str_hex_highlight_two(char *dst, int dst_size, const void *data, int data_size, int from1, int to1, int from2, int to2)
{
	static const char hex[] = "0123456789ABCDEF";
	int b;
	int i = 0;

	for(b = 0; b < data_size && b < dst_size/4-4; b++)
	{
		dst[b*4] = (i == from1 || i == from2) ? '<' : ' ';
		dst[b*4+1] = hex[((const unsigned char *)data)[b]>>4];
		dst[b*4+2] = hex[((const unsigned char *)data)[b]&0xf];
		dst[b*4+3] = (i == to1 || i == to2) ? '>' : ' ';
		dst[b*4+4] = 0;
		++i;
	}
}

void str_hex_highlight_three(char *dst, int dst_size, const void *data, int data_size, int from1, int to1, int from2, int to2, int from3, int to3)
{
	static const char hex[] = "0123456789ABCDEF";
	int b;
	int i = 0;

	for(b = 0; b < data_size && b < dst_size/4-4; b++)
	{
		dst[b*4] = (i == from1 || i == from2 || i == from3) ? '<' : ' ';
		dst[b*4+1] = hex[((const unsigned char *)data)[b]>>4];
		dst[b*4+2] = hex[((const unsigned char *)data)[b]&0xf];
		dst[b*4+3] = (i == to1 || i == to2 || i == to3) ? '>' : ' ';
		dst[b*4+4] = 0;
		++i;
	}
}

int min(int a, int b) { return a > b ? b : a; }

void print_hex_row_highlight_two(const char *type, const char *prefix, const void *data, int data_size, int from1, int to1, const char *note1, int from2, int to2, const char *note2, const char *info)
{
	char aHexData[1024];
	char aRawData[1024];
	const unsigned char *pChunkData = (const unsigned char *)data;
	int offset1;
	int offset2;
	int offset_arrow_2;
	int offset_note_2;
	int note1_len = str_length(note1);
	int dist = from2 - from1;
	int space = dist * 4;
	// int note2_len = str_length(note2);

	str_hex_highlight_two(aHexData, sizeof(aHexData), data, data_size, from1, to1, from2, to2);
	str_raw(aRawData, sizeof(aRawData), pChunkData, data_size);
	dbg_msg(type, "%s%s    %s  %s", prefix, aHexData, aRawData, info);

	// dbg_msg("d", "note1len=%d f1=%d f2=%d dist=%d space=%d", note1_len, from1, from2, dist, space);
	// if note1 has no space put it in the second line
	if(note1_len > space)
	{
		offset1 = from1 * 4;
		offset2 = from2 * 4;
		offset_arrow_2 = (offset2 - offset1) - 1;
		offset_note_2 = offset_arrow_2;
		dbg_msg(type, "%s%*s%s%*s%s", prefix, offset1, " ", "^", offset_arrow_2, " ", "^");
		dbg_msg(type, "%s%*s%s%*s%s", prefix, offset1, " ", "|", offset_note_2, " ", note2);
		dbg_msg(type, "%s%*s%s", prefix, offset1, " ", note1);
	}
	else
	{
		offset1 = from1 * 4;
		offset2 = from2 * 4;
		offset_arrow_2 = (offset2 - offset1) - 1;
		offset_note_2 = (offset_arrow_2 - note1_len) + 1;
		dbg_msg(type, "%s%*s%s%*s%s", prefix, offset1, " ", "^", offset_arrow_2, " ", "^");
		// dbg_msg(type, "%s%*s%s%*s%s", prefix, offset1, " ", note1, offset_note_2, " ", note2); // single line comment
		// dbg_msg(type, "%s%*s%s%*s%s", prefix, offset1, " ", note1, offset_note_2, " ", note2); // single line
		dbg_msg(type, "%s%*s%s%*s%s", prefix, offset1, " ", note1, offset_note_2, " ", "|"); // multi line comment
		dbg_msg(type, "%s%*s%s", prefix, offset2, " ", note2);
	}
}

void print_hex_row_highlight_three(
	const char *type,
	const char *prefix,
	const void *data,
	int data_size,
	int from1,
	int to1,
	const char *note1,
	int from2,
	int to2,
	const char *note2,
	int from3,
	int to3,
	const char *note3,
	const char *info)
{
	char aHexData[1024];
	char aRawData[1024];
	const unsigned char *pChunkData = (const unsigned char *)data;
	int offset1;
	int offset2;
	int offset3;
	int offset_arrow_2;
	int offset_arrow_3;
	int offset_note_2;

	str_hex_highlight_three(aHexData, sizeof(aHexData), data, data_size, from1, to1, from2, to2, from3, to3);
	str_raw(aRawData, sizeof(aRawData), pChunkData, data_size);
	dbg_msg(type, "%s%s    %s  %s", prefix, aHexData, aRawData, info);
	offset1 = from1 * 4;
	offset2 = from2 * 4;
	offset3 = from3 * 4;
	offset_arrow_2 = (offset2 - offset1) - 1;
	offset_note_2 = (offset_arrow_2 - str_length(note1)) + 1;
	offset_arrow_3 = (offset3 - (offset_arrow_2 + offset1 + 1)) - 1;
	dbg_msg(type, "%s%*s^%*s^%*s^", prefix, offset1, " ", offset_arrow_2, " ", offset_arrow_3, " ");
	dbg_msg(type, "%s%*s%s%*s%s%*s%s", prefix, offset1, " ", note1, offset_note_2, " ", "|", offset_arrow_3, " ", note3);
	dbg_msg(type, "%s%*s%s", prefix, offset2, " ", note2);
}

void print_hex_row_highlighted(const char *type, const char *prefix, const void *data, int data_size, int from, int to, const char *note, ...)
{
	char aHexData[1024];
	char aRawData[1024];
	const unsigned char *pChunkData = (const unsigned char *)data;
	int offset;
	va_list args;
	int len = 0;
	size_t size = 0;
	char *msg = NULL;
	char str[1024];

	str_hex_highlighted(aHexData, sizeof(aHexData), data, data_size, from, to);
	str_raw(aRawData, sizeof(aRawData), pChunkData, data_size);
	dbg_msg(type, "%s%s    %s", prefix, aHexData, aRawData);
	offset = from * 4;
	dbg_msg(type, "%s%*s%s", prefix, offset, " ", "^");


	/* Determine required size */

	va_start(args, note);
	len = vsnprintf(msg, size, note, args);
	va_end(args);

	if (len < 0)
		return;

	/* One extra byte for '\0' */

	size = (size_t) len + 1;
	msg = (char *)malloc(size);
	if (msg == NULL)
		return;

	va_start(args, note);
	len = vsnprintf(msg, size, note, args);
	va_end(args);

	if (len < 0)
	{
		free(msg);
		return;
	}
	str_copy(str, msg, sizeof(str));
	dbg_msg(type, "%s%*s%s", prefix, offset, " ", str);

	free(msg);
}

void print_hex(const char *type, const char *prefix, const void *data, int data_size, int max_width)
{
	char aHexData[1024];
	char aRawData[1024];
	int hex_row_length;
	int row_length;
	const unsigned char *pChunkData;

	int i = 0;
	while(i < data_size)
	{
		// rows are usually of length max_width
		// but the last row might be shorter if it is not filling it fully
		row_length = min(max_width, data_size - i);
		str_hex_spaced(aHexData, sizeof(aHexData), (const unsigned char *)data + i, row_length);
		pChunkData = (const unsigned char *)data + i;
		str_raw(aRawData, sizeof(aRawData), pChunkData, row_length);
		hex_row_length = max_width * 3; // 1 raw byte is 2 characters of hex plus one seperation space
		dbg_msg(type, "%s%-*s    %s", prefix, hex_row_length, aHexData, aRawData);
		i += row_length;
	}
}
