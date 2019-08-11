#include "Defines.h"
#include "Message.h"
#include "Output.h"
#include "Input.h"
#include "Party.h"
#include "Player.h"

std::string font_file;
std::string font_orient;

std::map<_wintype*, int> winXmap;
std::map<_wintype*, int> winYmap;

std::map<std::string, _colortype> colormap;

void init_color()
{
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
	mainWindow->setDefaultForeground(BORDER_COLOR);

	int x = 0, y = 21;
	TCODLine::init(x, y, 79, 21);
	do {
		_wputchar(mainWindow, y, x, ACS_DBL_HLINE, BORDER_COLOR);
		//mainWindow->putChar(x, y, ACS_DBL_HLINE);
	} while (! TCODLine::step(&x, &y) );

	x = 41; y = 0;
	TCODLine::init(x, y, 41, 20);
	do {
		_wputchar(mainWindow, y, x, ACS_DBL_VLINE, BORDER_COLOR);
		//mainWindow->putChar(x, y, ACS_DBL_VLINE);
	} while (! TCODLine::step(&x, &y) );
	
	_wputchar(mainWindow, 21, 41, ACS_DBL_BTEE, BORDER_COLOR);
	
	mainWindow->flush();
}

void init_display()
{
	TCODConsole::setCustomFont(font_file.c_str(), font_orient == "COL" ?
		TCOD_FONT_LAYOUT_ASCII_INCOL : TCOD_FONT_LAYOUT_ASCII_INROW);
	TCODConsole::initRoot (80, 25, "Final Fantasy", false,
		TCOD_RENDERER_OPENGL);

	mainWindow = TCODConsole::root;

	bigWindow = new TCODConsole(BIG_WIDTH, BIG_HEIGHT);
	winXmap[bigWindow] = BIG_UL_X;
	winYmap[bigWindow] = BIG_UL_Y;

	helpWindow = new TCODConsole(HELP_WIDTH, HELP_HEIGHT);
	winXmap[helpWindow] = HELP_UL_X;
	winYmap[helpWindow] = HELP_UL_Y;
	
	leftWindow = new TCODConsole(LEFT_WIDTH, LEFT_HEIGHT);
	winXmap[leftWindow] = LEFT_UL_X;
	winYmap[leftWindow] = LEFT_UL_Y;
	
	messageWindow = new TCODConsole(MSG_WIDTH, MSG_HEIGHT);
	winXmap[messageWindow] = MSG_UL_X;
	winYmap[messageWindow] = MSG_UL_Y;
	
	rightWindow = new TCODConsole(RIGHT_WIDTH, RIGHT_HEIGHT);
	winXmap[rightWindow] = RIGHT_UL_X;
	winYmap[rightWindow] = RIGHT_UL_Y;
	
	upperWindow = new TCODConsole(UPPER_WIDTH, UPPER_HEIGHT);
	winXmap[upperWindow] = UPPER_UL_X;
	winYmap[upperWindow] = UPPER_UL_Y;
	
	levelupWindow = new TCODConsole(LVUP_WIDTH, LVUP_HEIGHT);
	winXmap[levelupWindow] = LVUP_UL_X;
	winYmap[levelupWindow] = LVUP_UL_Y;

	init_color();
	draw_screen_borders();
}

void end_display()
{
}

int _wputchar(_wintype *window, int y, int x, _chtype ch,
	_colortype color, bool reverse)
{
	if (reverse)
	{
		window->setDefaultBackground(color);
		window->setDefaultForeground(TCODColor::black);
		window->putChar(x, y, ch, TCOD_BKGND_SET);
		window->setDefaultBackground(TCODColor::black);
	}
	else
	{
		window->setDefaultForeground(color);
		window->putChar(x, y, ch);
	}
	
	return 0;
}

int _wputcstr_reverse(_wintype *window, int y, int x, _colortype color,
	                  const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	int retval = 0;
	window->setDefaultBackground(color);
	window->setBackgroundFlag(TCOD_BKGND_SET);
	window->print(x, y, format, argp);
	window->setDefaultBackground(TCODColor::black);

	return retval;
}

int _wputcstr(_wintype *window, int y, int x, _colortype color,
				const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	int retval = 0;
	char buffer[80];
	vsnprintf(buffer, 80, format, argp);

	window->setDefaultForeground(color);
	window->print(x, y, buffer);

	return retval;
}

int _wputstring(_wintype *window, int y, int x, _colortype color,
				std::string str, bool reverse)
{
	if (reverse)
	{
		window->setDefaultBackground(WHITE);
		window->setBackgroundFlag(TCOD_BKGND_SET);
		_wputcstr(window, y, x, BLACK, str.c_str());
		window->setDefaultBackground(BLACK);
	}
	else
	{
		_wputcstr(window, y, x, color, str.c_str());
	}
	
	return 0;
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
	window->flush();
}

void _delwin(_wintype* window)
{
	delete window;
}

_wintype* _derwin(_wintype* superwindow, int nlines, int ncols,
	int begin_y, int begin_x)
{
	TCODConsole* window = new TCODConsole(ncols, nlines);
	winXmap[window] = winXmap[superwindow] + begin_x;
	winYmap[window] = winYmap[superwindow] + begin_y;
	return window;
}

_wintype* _dupwin(_wintype* window)
{
	TCODConsole* newWindow = new TCODConsole(_getwidth(window),
		_getheight(window));
	TCODConsole::blit(window, 0, 0, 0, 0, newWindow, 0, 0);
	winXmap[newWindow] = winXmap[window];
	winYmap[newWindow] = winYmap[window];
	return newWindow;
}

void show_window(_wintype* window)
{
	TCODConsole::blit(window, 0, 0, 0, 0, mainWindow, winXmap[window],
		winYmap[window]);
}

void cbox(_wintype *window, _colortype color)
{
	_setColor(window, color);
	window->printFrame(0, 0, _getwidth(window),
		_getheight(window));
}

void divide_window_vert(_wintype *window, _wintype*& lwindow,
	_wintype*& rwindow,	int middle)
{
	cbox(window, BORDER_COLOR);

	_setColor(window, BORDER_COLOR);
	window->vline(middle, 1, _getheight(window) - 2);
	_wputchar(window, 0, middle, TCOD_CHAR_TEES, BORDER_COLOR);
	_wputchar(window, _getheight(window) - 1, middle, TCOD_CHAR_TEEN,
		BORDER_COLOR);

	lwindow = _derwin(window, _getheight(window) - 2, middle - 1, 1, 1);
	rwindow = _derwin(window, _getheight(window) - 2,
		_getwidth(window) - middle - 2, 1, middle + 1);
}

// This hasn't been implemented yet for libtcod.
void show_enemy_pad(int y)
{
	//prefresh(enemyPad, y, 0, 0, 18, LEFT_HEIGHT - 1, RIGHT_UL_X - 2);

#ifdef _CURSES_
	copywin(enemyPad, mainWindow, y, 0, 0, 20, LEFT_HEIGHT - 1,
		RIGHT_UL_X - 2, false);

	refresh();
#endif
}

void prepare_combat_output()
{
	_werase(leftWindow);

	battleQWindow = _derwin(leftWindow, LEFT_HEIGHT - 1, 17, 1, 1);
	//enemyWindow = derwin(leftWindow, LEFT_HEIGHT - 1, 21, 0, 18);

	//I think this will need to used something like the enemypad - set Output_curses.cpp
	_setColor(leftWindow, BORDER_COLOR);
	leftWindow->vline(19, 0, LEFT_HEIGHT);

	_wputchar(mainWindow, 21, 19, ACS_COMBINE_BTEE, BORDER_COLOR);

	show_window(leftWindow);
}
