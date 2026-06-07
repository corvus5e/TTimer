#ifndef _CHAR_CODES_H_
#define _CHAR_CODES_H_

#include <HomeTUI/home_tui.h>

#if USE_UTF8
#define CHAR(ascii_char, utf8_str) utf8_str
#else
#define CHAR(ascii_char, utf8_str) ascii_char
#endif

#define GRAPH_LIGHT_CELL CHAR('#', L'░')
#define GRAPH_MEDIUM_CELL CHAR('#', L'▒')
#define GRAPH_HARD_CELL CHAR('#', L'▓')
#define GRAPH_SHORT_CELL CHAR('#', L'▇')

#define TIMER_H_BORDER CHAR('-', L'═')
#define TIMER_V_BORDER CHAR('|', L'║')

#define TIMER_UL_CORNER CHAR('*', L'╔')
#define TIMER_UR_CORNER CHAR('*', L'╗')
#define TIMER_LL_CORNER CHAR('*', L'╚')
#define TIMER_LR_CORNER CHAR('*', L'╝')

#endif
