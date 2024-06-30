#include <base/system.h>

#include "color.h"

#define COUNT_COLOR(color) \
	pColor = str_find(pStr, color); \
	while(pColor) \
	{ \
		Len += sizeof(color) - 1; \
		pColor = str_find(pColor + 1, color); \
	}

int count_color_code_len(const char *pStr)
{
	int Len = 0;
	const char *pColor;
	COUNT_COLOR(TERM_RESET);
	COUNT_COLOR(TERM_BOLD);
	COUNT_COLOR(TERM_BLACK);
	COUNT_COLOR(TERM_RED);
	COUNT_COLOR(TERM_GREEN);
	COUNT_COLOR(TERM_YELLOW);
	COUNT_COLOR(TERM_BLUE);
	COUNT_COLOR(TERM_MAGENTA);
	COUNT_COLOR(TERM_CYAN);
	COUNT_COLOR(TERM_WHITE);
	return Len;
}

