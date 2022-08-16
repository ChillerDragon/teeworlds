#ifndef BASE_DISSECTOR_BYTE_PRINTER_H
#define BASE_DISSECTOR_BYTE_PRINTER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <base/system.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(CONF_FAMILY_UNIX)
	#include <sys/time.h>
	#include <unistd.h>

	/* unix net includes */
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <errno.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <pthread.h>
	#include <arpa/inet.h>

	#include <dirent.h>

	#if defined(CONF_PLATFORM_MACOSX)
		#include <Carbon/Carbon.h>
	#endif

#elif defined(CONF_FAMILY_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <fcntl.h>
	#include <direct.h>
	#include <errno.h>
	#include <process.h>
	#include <wincrypt.h>
	#include <share.h>
	#include <shellapi.h>
#else
	#error NOT IMPLEMENTED
#endif

#if defined(CONF_ARCH_IA32) || defined(CONF_ARCH_AMD64)
	#include <immintrin.h> //_mm_pause
#endif

#if defined(CONF_PLATFORM_SOLARIS)
	#include <sys/filio.h>
#endif

/*
	Function: str_hex_spaced
		str_hex but groups of 4 bytes are space seperated
*/
void str_hex_spaced(char *dst, int dst_size, const void *data, int data_size);
/*
	Function: str_raw
		get all printable chars of raw data
		and use dot for unprintables

	Parameters:
		dst - Buffer to fill with raw data
		dst_size - size of the buffer
		data - Data to turn into raw
		data - Size of the data

	Remarks:
		- The destination buffer will be zero-terminated
*/
void str_raw(char *dst, int dst_size, const void *data, int data_size);
void print_raw(const char *sys, const char *prefix, const void *data, int data_size);
void str_hex_highlighted(char *dst, int dst_size, const void *data, int data_size, int from, int to);
void str_hex_highlight_two(char *dst, int dst_size, const void *data, int data_size, int from1, int to1, int from2, int to2);
void str_hex_highlight_three(char *dst, int dst_size, const void *data, int data_size, int from1, int to1, int from2, int to2, int from3, int to3);
void print_hex(const char *type, const char *prefix, const void *data, int data_size, int max_width);
void print_hex_row_highlighted(const char *type, const char *prefix, const void *data, int data_size, int from, int to, const char *note, ...);
void print_hex_row_highlight_two(const char *type, const char *prefix, const void *data, int data_size, int from1, int to1, const char *note1, int from2, int to2, const char *note2, const char *info);
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
	const char *info);

#endif
