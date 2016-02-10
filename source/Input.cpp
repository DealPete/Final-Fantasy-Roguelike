#include "Input.h"
#include "Message.h"
#include "Party.h"

int _getch(input_type state)
{
	flush_message_buf();

#ifdef _CURSES_
	chtype ch = getch();
#endif
#ifdef _LIBTCOD_
	mainWindow->flush();
	TCOD_key_t key = TCODConsole::waitForKeypress(true);
	int ch = key.c;
	if (!ch || key.vk == _KEY_ENTER)
		ch = key.vk;
#endif

	switch (ch)
	{
	case KEY_ESC:
	case _KEY_KEYPAD_ZERO:
	case 'X':
	case 'x':
	case '0':
		return _KEY_CANCEL;
		
	case KEY_SPACE:
	case _KEY_ENTER:
	case _KEY_KEYPAD_ENTER:
	case 'Z':
	case 'z':
		return KEY_ACCEPT;

	case '8':
		return _KEY_UP;

	case KEY_PGUP:
	case '9':
		if (Party.enemyPad_viewport > 0)
		Party.enemyPad_viewport--;
		return KEY_PGUP;

	case '4':
		return _KEY_LEFT;

	case '6':
		return _KEY_RIGHT;

	case '2':
		return _KEY_DOWN;

	case KEY_PGDN:
	case '3':
		if (Party.enemyPad_viewport < ENEMY_PAD_SIZE - LEFT_HEIGHT)
			Party.enemyPad_viewport++;
		return KEY_PGDN;
	}

	if (ch == 's' && state == INPUT_NORMAL)
		Party.display_status();

	if (ch == 'P' && state == INPUT_NORMAL)
		browse_buffer(bigWindow, &message_buffer);

	return ch;
}

int _mvgetnstr(int y, int x, char* str, int n)
{
	int retval;
#ifdef _CURSES_
	echo();
	retval = mvgetnstr(y, x, str, n);
	noecho();
#endif // _CURSES_

#ifdef _LIBTCOD_
	int cx = 0;
	
	mainWindow->flush();
	mainWindow->setDefaultForeground(TEXT_COLOR);
	TCOD_key_t ch = TCODConsole::waitForKeypress(true);
	
	while (ch.vk != _KEY_ENTER && ch.vk != _KEY_KEYPAD_ENTER)
	{
		if (ch.vk == TCODK_BACKSPACE)
		{
			if (cx > 0)
				cx--;
			mainWindow->putChar(x + cx, y, 0);
		}
		else if (ch.c && cx < n)
		{
			mainWindow->putChar(x + cx, y, (int)ch.c);
			str[cx++] = ch.c;
		}
		
		mainWindow->flush();	
		ch = TCODConsole::waitForKeypress(true);
	}
	
	str[cx] = '\0';
#endif

	return retval;
}

/*
int _mvscanw(int y, int x, const char* format, ...)
{
	va_list args;
	int retval;
	
	move(y, x);

	echo();
	
	va_start(args, format);
	retval = vwscanw(stdscr, format, args);
	va_end(args);

	noecho();

	return retval;
}
*/

