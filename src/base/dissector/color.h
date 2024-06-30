#ifndef BASE_DISSECTOR_COLOR_H
#define BASE_DISSECTOR_COLOR_H

#define TERM_RESET   "\033[0m"
#define TERM_BOLD    "\033[1m"
#define TERM_BLACK   "\033[30;1m"
#define TERM_RED     "\033[31;1m"
#define TERM_GREEN   "\033[32;1m"
#define TERM_YELLOW  "\033[33;1m"
#define TERM_BLUE    "\033[34;1m"
#define TERM_MAGENTA "\033[35;1m"
#define TERM_CYAN    "\033[36;1m"
#define TERM_WHITE   "\033[37;1m"

int count_color_code_len(const char *pStr);

#endif
