#include "Defines.h"
#include "Message.h"
#include "Output.h"
#include "Input.h"

void buffer::push(std::string text, _colortype color)
{
	buffer buf;
	format_string(buf, text, width);
	for(std::string& it : buf)
	{
		push_back(it);
		back().color = color;
	}
}

std::string font_file;
std::string font_orient;

#ifdef _WIN32
#include <Windows.h>
#include <Wincon.h>
#endif

std::map<std::string, _colortype> colormap;

void init_color()
{
	start_color();
	init_color(COLOR_MAGENTA, 300, 300, 300);
	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_CYAN, COLOR_BLACK);
	init_pair(4, COLOR_RED, COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(6, COLOR_YELLOW, COLOR_BLACK);
	init_pair(7, COLOR_WHITE, COLOR_BLACK);
	init_pair(8, COLOR_BLACK, COLOR_BLACK);

	colormap["Black"] = BLACK;
	colormap["Blue"] = BLUE;
	colormap["Green"] = GREEN;
	colormap["Cyan"] = CYAN;
	colormap["Red"] = RED;
	colormap["Yellow"] = YELLOW;
	colormap["Lightgray"] = LIGHTGRAY;
	colormap["Darkgray"] = DARKGRAY;
	colormap["Lightblue"] = LIGHTBLUE;
	colormap["Lightgreen"] = LIGHTGREEN;
	colormap["Lightcyan"] = LIGHTCYAN;
	colormap["Lightred"] = LIGHTRED;
	colormap["Lightyellow"] = LIGHTYELLOW;
	colormap["White"] = WHITE;
}

void draw_screen_borders()
{
	attron(BORDER_COLOR);
	mvhline(21,0,ACS_DBL_HLINE,80);
	mvvline(0,41,ACS_DBL_VLINE,21);
	mvaddch(21,41,ACS_DBL_BTEE);
	attroff(BORDER_COLOR);
}

void init_display()
{
#ifdef _WIN32
	SetConsoleOutputCP(437);
#endif

	setlocale(LC_ALL, "");

	mainWindow = initscr();
	
	bigWindow = newwin(BIG_HEIGHT, BIG_WIDTH, BIG_UL_Y, BIG_UL_X);
	helpWindow = newwin(HELP_HEIGHT, HELP_WIDTH, HELP_UL_Y, HELP_UL_X);
	leftWindow = newwin(LEFT_HEIGHT, LEFT_WIDTH, LEFT_UL_Y, LEFT_UL_X);
	messageWindow = newwin(MSG_HEIGHT, MSG_WIDTH, MSG_UL_Y, MSG_UL_X);
	rightWindow = newwin(RIGHT_HEIGHT, RIGHT_WIDTH, RIGHT_UL_Y,
		RIGHT_UL_X);
	upperWindow = newwin(UPPER_HEIGHT, UPPER_WIDTH, UPPER_UL_Y,
		UPPER_UL_X);
	levelupWindow = newwin(LVUP_HEIGHT, LVUP_WIDTH, LVUP_UL_Y,
		LVUP_UL_X);

	keypad(stdscr, true);
	cbreak();
	noecho();
	curs_set(0);
	ESCDELAY = 0;

	init_color();
	draw_screen_borders();
}

void end_display()
{
	endwin();
}

int _wputchar(_wintype *window, int y, int x, _chtype ch,
	_colortype color, bool reverse)
{
	if (reverse) ch |= A_REVERSE;
	wmove(window, y, x);
#ifndef _WIN32
   	return waddch(window, ch | color);
#else
	return waddch(window, ch | color | A_ALTCHARSET);
#endif
}

int _wputcstr_reverse(_wintype *window, int y, int x, _colortype color,
	                  const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	int retval = 0;
	wmove(window, y, x);
	
	wattron(window, color | A_REVERSE);
	retval = vwprintw(window, format, argp);
	wattroff(window, color | A_REVERSE);

	return retval;
}

int _wputcstr(_wintype *window, int y, int x, _colortype color,
				const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	int retval = 0;
	wmove(window, y, x);
	
	wattron(window, color);
	retval = vwprintw(window, format, argp);
	wattroff(window, color);

	return retval;
}

int _wputstring(_wintype *window, int y, int x, _colortype color,
				std::string str, bool reverse)
{
	if (reverse) color |= A_REVERSE;
	return _wputcstr(window, y, x, color, str.c_str());
}

/*
int _wprintw(_wintype *window, std::string str)
{
	return wprintw(window, str.c_str());
}

int _wprintw(_wintype *window, const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	int retval = vwprintw(window, format, argp);

	return retval;
}

int wcprintw(_wintype* window, _colortype color, std::string str)
{
	int retval;
	
#ifdef _CURSES_
	wattron(window, color);
	retval = wprintw(window, str.c_str());
	wattroff(window, color);
#endif
	return retval;
}

int wcprintw(_wintype *window, _colortype color, const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	wattron(window, color);
	int retval = vwprintw(window, format, argp);
	wattroff(window, color);

	return retval;
}
*/

void _wrefresh(_wintype* window)
{
	wrefresh(window);
	clearok(mainWindow, false);
}

void _delwin(_wintype* window)
{
	delwin(window);
}

_wintype* _derwin(_wintype* superwindow, int nlines, int ncols,
	int begin_y, int begin_x)
{
	return derwin(superwindow, nlines, ncols, begin_y, begin_x);
}

_wintype* _dupwin(_wintype* window)
{
	return dupwin(window);
}

void show_window(_wintype* window)
{
	overwrite(window, mainWindow);
}

void cbox(_wintype *window, _colortype color)
{
	wattron(window, color);
	box(window, 0, 0);
	wattroff(window, color);
}

void divide_window_vert(_wintype *window, _wintype*& lwindow,
	_wintype*& rwindow,	int middle)
{
	cbox(window, BORDER_COLOR);

	wattron(window, BORDER_COLOR);
	mvwvline(window, 1, middle, ACS_VLINE, _getheight(window) - 2);
	mvwaddch(window, 0, middle, ACS_TTEE);
	mvwaddch(window, _getheight(window) - 1, middle, ACS_BTEE);
	wattroff(window, BORDER_COLOR);

	lwindow = _derwin(window, _getheight(window) - 2, middle - 1, 1, 1);
	rwindow = _derwin(window, _getheight(window) - 2,
		_getwidth(window) - middle - 2, 1, middle + 1);
}

void show_enemy_pad(int y)
{
	//prefresh(enemyPad, y, 0, 0, 18, LEFT_HEIGHT - 1, RIGHT_UL_X - 2);

	copywin(enemyPad, mainWindow, y, 0, 0, 20, LEFT_HEIGHT - 1,
		RIGHT_UL_X - 2, false);

	refresh();
}

void prepare_combat_output()
{
	_werase(leftWindow);

	battleQWindow = _derwin(leftWindow, LEFT_HEIGHT - 1, 17, 1, 1);
	//enemyWindow = derwin(leftWindow, LEFT_HEIGHT - 1, 21, 0, 18);

	enemyPad = newpad(ENEMY_PAD_SIZE, 23);
	wattron(leftWindow, BORDER_COLOR);
	mvwvline(leftWindow, 0, 19, ACS_VLINE, LEFT_HEIGHT);
	wattroff(leftWindow, BORDER_COLOR);

	_wputchar(mainWindow, 21, 19, ACS_COMBINE_BTEE, BORDER_COLOR);

	show_window(leftWindow);
}
