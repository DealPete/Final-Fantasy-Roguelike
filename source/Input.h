#ifndef __INPUT_H_
#define __INPUT_H_

#include "Defines.h"
#include _MEDIA_HEADER_

#define KEY_ESC		27
#define KEY_SPACE	32
#define CTL_BIT		64

#ifdef _LIBTCOD_

#define _KEY_UP		TCODK_UP
#define _KEY_DOWN	TCODK_DOWN
#define _KEY_LEFT	TCODK_LEFT
#define _KEY_RIGHT	TCODK_RIGHT

#define _KEY_ENTER	TCODK_ENTER
#define KEY_ACCEPT 	TCODK_SPACE

#define _KEY_KEYPAD_ENTER	TCODK_KPENTER
#define _KEY_KEYPAD_ZERO	TCODK_KP0

#define _KEY_UP		TCODK_UP
#define _KEY_LEFT	TCODK_LEFT
#define _KEY_RIGHT	TCODK_RIGHT
#define _KEY_DOWN	TCODK_DOWN
#define KEY_PGUP	TCODK_PAGEUP
#define KEY_PGDN	TCODK_PAGEDOWN

#endif // _LIBTCOD_

#define _KEY_CANCEL	KEY_ESC

#ifdef _CURSES_

#define _KEY_ENTER	10
#define KEY_ACCEPT  KEY_SPACE

#define _KEY_KEYPAD_ENTER	459
#define _KEY_KEYPAD_ZERO	506

#define _KEY_UP		KEY_UP
#define _KEY_LEFT	KEY_LEFT
#define _KEY_RIGHT	KEY_RIGHT
#define _KEY_DOWN	KEY_DOWN
#define KEY_PGUP	KEY_PPAGE
#define KEY_PGDN	KEY_NPAGE

#endif // _CURSES_

enum input_type
{
	INPUT_START,
	INPUT_NORMAL,
	INPUT_STATUS,
	INPUT_MORE,
	INPUT_ERROR
};


int _getch(input_type state = INPUT_NORMAL);
int _mvgetnstr(int y, int x, char* str, int n);
int _mvscanw(int y, int x, const char* format, ...);

#endif
