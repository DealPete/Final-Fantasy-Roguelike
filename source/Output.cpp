#include "Defines.h"
#include "Message.h"
#include "Output.h"
#include "Input.h"
#include "Party.h"
#include "Player.h"

#include <stack>

FILE* logfile;

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

#ifdef _CURSES_

#ifdef _WIN32
#include <Windows.h>
#include <Wincon.h>
#endif

#endif // _CURSES_

_wintype* battleQWindow;
_wintype* bigWindow;
_wintype* enemyPad;
_wintype* helpWindow;
_wintype* leftWindow;
_wintype* mainWindow;
_wintype* messageWindow;
_wintype* rightWindow;
_wintype* upperWindow;
_wintype* levelupWindow;

static std::stack<_wintype*> windowStack;

#ifdef _LIBTCOD_
std::map<_wintype*, int> winXmap;
std::map<_wintype*, int> winYmap;
#endif

std::map<std::string, _colortype> colormap;

void init_color()
{
#ifdef _CURSES_
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_CYAN, COLOR_BLACK);
	init_pair(4, COLOR_RED, COLOR_BLACK);
	init_pair(6, COLOR_YELLOW, COLOR_BLACK);
	init_pair(7, COLOR_WHITE, COLOR_BLACK);
	init_pair(8, COLOR_BLACK, COLOR_BLACK);
#endif // _CURSES_

	colormap["Black"] = BLACK;
	colormap["Blue"] = BLUE;
	colormap["Green"] = GREEN;
	colormap["Cyan"] = CYAN;
	colormap["Red"] = RED;
	colormap["Magenta"] = MAGENTA;
	colormap["Yellow"] = YELLOW;
	colormap["Lightgray"] = LIGHTGRAY;
	colormap["Darkgray"] = DARKGRAY;
	colormap["Lightblue"] = LIGHTBLUE;
	colormap["Lightgreen"] = LIGHTGREEN;
	colormap["Lightcyan"] = LIGHTCYAN;
	colormap["Lightred"] = LIGHTRED;
	colormap["Lightmagenta"] = LIGHTMAGENTA;
	colormap["Lightyellow"] = LIGHTYELLOW;
	colormap["White"] = WHITE;
}

void draw_screen_borders()
{
#ifdef _CURSES_
	attron(BORDER_COLOR);
	mvhline(21,0,ACS_DBL_HLINE,80);
	mvvline(0,41,ACS_DBL_VLINE,21);
	mvaddch(21,41,ACS_DBL_BTEE);
	attroff(BORDER_COLOR);
#endif

#ifdef _LIBTCOD_
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
#endif

}

void init_display()
{
	logfile = fopen("logfile.txt","w");
		
#ifdef _CURSES_
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

#endif // _CURSES_

#ifdef _LIBTCOD_
	
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
#endif

	init_color();
	draw_screen_borders();
}

void end_display()
{
	fclose(logfile);
	
#ifdef _CURSES_
	endwin();
#endif
}

// This function formats "str" to fit in "window", with a margin of
// one square on each side, then prints it spanning as many lines as
// necessary.
// Return value:  the y co-ordinate below the last line printed.
int format_print(_wintype* window, int y, std::string str)
{
	y = cformat_print(window, TEXT_COLOR, y, str);

	return y;
}

int format_print(_wintype* window, int y, const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	char buf[1024];

	vsprintf(buf, format, argp);

	va_end(argp);

	return format_print(window, y, std::string(buf));
}

int cformat_print(_wintype* window, _colortype color, int y,
				  std::string str)
{
	buffer buf;
	format_string(buf, str, _getwidth(window) - 2);

	for(std::string& it : buf)
	{
		_wputstring(window, y++, 1, color, it);
	}

	return y;
}

int cformat_print(_wintype* window, _colortype color, int y,
				  const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	char buf[1024];

	vsprintf(buf, format, argp);

	va_end(argp);

	return cformat_print(window, color, y, std::string(buf));
}

int _wputchar(_wintype *window, int y, int x, _chtype ch,
	_colortype color, bool reverse)
{
#ifdef _CURSES_
	if (reverse) ch |= A_REVERSE;
	wmove(window, y, x);
#ifdef __linux__
   	return waddch(window, ch | color);
#else
	return waddch(window, ch | color | A_ALTCHARSET);
#endif
#endif

#ifdef _LIBTCOD_
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
#endif
}

int _wputcstr_reverse(_wintype *window, int y, int x, _colortype color,
	                  const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	int retval = 0;
#ifdef _CURSES_
	wmove(window, y, x);
	
	wattron(window, color | A_REVERSE);
	retval = vwprintw(window, format, argp);
	wattroff(window, color | A_REVERSE);
#endif

#ifdef _LIBTCOD_
	window->setDefaultBackground(color);
	window->setBackgroundFlag(TCOD_BKGND_SET);
	window->print(x, y, format, argp);
	window->setDefaultBackground(TCODColor::black);
#endif

	return retval;
}

int _wputcstr(_wintype *window, int y, int x, _colortype color,
				const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	int retval = 0;
#ifdef _CURSES_
	wmove(window, y, x);
	
	wattron(window, color);
	retval = vwprintw(window, format, argp);
	wattroff(window, color);
#endif

#ifdef _LIBTCOD_
	char buffer[80];
	vsnprintf(buffer, 80, format, argp);

	window->setDefaultForeground(color);
	window->print(x, y, buffer);
#endif

	return retval;
}

int _wputstring(_wintype *window, int y, int x, _colortype color,
				std::string str, bool reverse)
{
#ifdef _CURSES_
	if (reverse) color |= A_REVERSE;
	return _wputcstr(window, y, x, color, str.c_str());
#endif

#ifdef _LIBTCOD_
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
#endif
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
#ifdef _CURSES_
	wrefresh(window);
	clearok(mainWindow, false);
#endif
#ifdef _LIBTCOD_
	window->flush();
#endif
}

void _delwin(_wintype* window)
{
#ifdef _CURSES_
	delwin(window);
#endif
#ifdef _LIBTCOD_
	delete window;
#endif
}

_wintype* _derwin(_wintype* superwindow, int nlines, int ncols,
	int begin_y, int begin_x)
{
#ifdef _CURSES_
	return derwin(superwindow, nlines, ncols, begin_y, begin_x);
#endif
#ifdef _LIBTCOD_
	TCODConsole* window = new TCODConsole(ncols, nlines);
	winXmap[window] = winXmap[superwindow] + begin_x;
	winYmap[window] = winYmap[superwindow] + begin_y;
	return window;
#endif
}

_wintype* _dupwin(_wintype* window)
{
#ifdef _CURSES_
	return dupwin(window);
#endif
#ifdef _LIBTCOD_
	TCODConsole* newWindow = new TCODConsole(_getwidth(window),
		_getheight(window));
	TCODConsole::blit(window, 0, 0, 0, 0, newWindow, 0, 0);
	winXmap[newWindow] = winXmap[window];
	winYmap[newWindow] = winYmap[window];
	return newWindow;
#endif
}

void show_window(_wintype* window)
{
#ifdef _CURSES_
	overwrite(window, mainWindow);
#endif
#ifdef _LIBTCOD_
	TCODConsole::blit(window, 0, 0, 0, 0, mainWindow, winXmap[window],
		winYmap[window]);
#endif
}

void draw_explore_screen()
{
	draw_screen_borders();
	draw_map(leftWindow, Party.L);
	Party.draw(rightWindow);
	flush_message_buf();
}

void stack_window()
{
	windowStack.push(_dupwin(mainWindow));
}

void unstack_window()
{
	show_window(windowStack.top());
	_delwin(windowStack.top());
	windowStack.pop();
}

void cbox(_wintype *window, _colortype color)
{
#ifdef _CURSES_
	wattron(window, color);
	box(window, 0, 0);
	wattroff(window, color);
#endif
#ifdef _LIBTCOD_
	_setColor(window, color);
	window->printFrame(0, 0, _getwidth(window),
		_getheight(window));
#endif
}

void divide_window_vert(_wintype *window, _wintype*& lwindow,
	_wintype*& rwindow,	int middle)
{
	cbox(window, BORDER_COLOR);

#ifdef _CURSES_
	wattron(window, BORDER_COLOR);
	mvwvline(window, 1, middle, ACS_VLINE, _getheight(window) - 2);
	mvwaddch(window, 0, middle, ACS_TTEE);
	mvwaddch(window, _getheight(window) - 1, middle, ACS_BTEE);
	wattroff(window, BORDER_COLOR);
#endif

#ifdef _LIBTCOD_
	_setColor(window, BORDER_COLOR);
	window->vline(middle, 1, _getheight(window) - 2);
	_wputchar(window, 0, middle, TCOD_CHAR_TEES, BORDER_COLOR);
	_wputchar(window, _getheight(window) - 1, middle, TCOD_CHAR_TEEN,
		BORDER_COLOR);
#endif

	lwindow = _derwin(window, _getheight(window) - 2, middle - 1, 1, 1);
	rwindow = _derwin(window, _getheight(window) - 2,
		_getwidth(window) - middle - 2, 1, middle + 1);
}

void browse_buffer(_wintype* window, buffer* buf)
{
	unsigned int pos = 0;
	unsigned int i;
	int ch;
	unsigned int lines = _getheight(window) - 2;

	buffer::iterator it;

	stack_window();

	do
	{
		_werase(window);
		cbox(window, BORDER_COLOR);

		it = buf->begin();

		for(i = 0; (i < lines + pos) && (i < buf->size()); i++, it++)
		{
			if (i >= pos)
			{
				_wputstring(window, (lines + pos) - i, 1, TEXT_COLOR, *it);
			}
		}

		show_window(window);

		switch(ch = _getch(INPUT_STATUS))
		{
		case _KEY_UP:
			if ((buf->size() > lines) && (pos < buf->size() - lines))
				pos++;
			break;

		case _KEY_DOWN:
			if (pos > 0)
				pos--;
			break;
		}

	} while (ch != _KEY_CANCEL);

	unstack_window();
}

void draw_buffer(_wintype* window, buffer* buf, bool in_box)
{
	_werase(window);
	if (in_box)
		cbox(window, BORDER_COLOR);

	int i = in_box ? 1 : 0;

	buffer::iterator it = buf->begin();

	for( ; i < (in_box ? _getheight(window) - 2 : _getheight(window) - 1)
		&& it != buf->end(); it++, i++)
		_wputstring(window, i, in_box ? 1 : 0, it->color, *it);
}

int menu_select(int y, int x, const buffer& selection, void (*desc)(int))
{
	int ch;
	unsigned int pos = 0;

	stack_window();

#ifdef _CURSES_
	_wintype* window = newwin(selection.size() + 2, 17, y, x);
#endif
#ifdef _LIBTCOD_
	_wintype* window = new TCODConsole(17, selection.size() + 2);
	winXmap[window] = x;
	winYmap[window] = y;
#endif
	
	cbox(window, BORDER_COLOR);

	do
	{
		unsigned int i = 0;

		for(const color_string& it : selection)
		{
			_wputstring(window, i + 1, 1, it.color, it, i == pos);
			i++;
		}

		if (desc)
			(void)desc(pos);

		show_window(window);

		ch = _getch();

		switch(ch)
		{
		case _KEY_UP:
			if (pos == 0)
				pos = selection.size() - 1;
			else
				pos--;
			break;

		case _KEY_DOWN:
			if (pos == selection.size() - 1)
				pos = 0;
			else
				pos++;
			break;
		}

	} while (ch != KEY_ACCEPT && ch != _KEY_CANCEL);

	unstack_window();

	if (ch == KEY_ACCEPT)
		return pos + 1;
	else
		return 0;
}

std::pair<int, int> CSelectFunctor::operator()()
{
	int ch = _KEY_CANCEL;
	depth = 1;  menu_move = DIVE;

	stack_window();

	_werase(window);

	if (!party_view)
		divide_window_vert(window, lWindow, rWindow, middle);
	else
	{
		bWindow = _derwin(window, _getheight(window) - 2,
			_getwidth(window), 2, 0);
		cbox(window, BORDER_COLOR);

		if (selection)
			divide_window_vert(bWindow, lWindow, rWindow, middle);
		else
		{
			rWindow = _derwin(bWindow, _getheight(bWindow) - 2,
				_getwidth(bWindow) - 2, 1, 1);
			cbox(bWindow, BORDER_COLOR);
		}

		_wputchar(bWindow, 0, 0, ACS_LTEE, BORDER_COLOR);
		_wputchar(bWindow, 0, _getwidth(window) - 1, ACS_RTEE,
			BORDER_COLOR);

	}

	int lCentre, lAboveSpan, lBelowSpan;
	int lMax;

	if (selection)
	{
		lMax = _getheight(lWindow);

		if (lMax & 1)
			lCentre = (lMax - 1) / 2;
		else
			lCentre = lMax / 2;

		lAboveSpan = lCentre;
		lBelowSpan = lMax - lCentre - 1;
	}

	do
	{
		_werase(rWindow);

		int i = 0;

		if (party_view)
			for(CPlayer& it : Party.Player)
			{
				_wputstring(window, 1, i * 17 + 2, TEXT_COLOR, it.name,
					i == menu);
				i++;
			}

		i = 0;

		unsigned int eff_menu = menu;
		if (eff_menu > Menu.size() - 1)
			eff_menu = Menu.size() - 1;

		if (selection)
		{
			_werase(lWindow);
			for(const color_string& it : Menu[eff_menu])
			{
				int moveY, mSize = Menu[eff_menu].size();

				if (pos < lAboveSpan || mSize <= lMax - 2)
					moveY = i + 1;
				else if (mSize - (pos + 1) < lBelowSpan)
					moveY = lMax - (mSize - i + 1);
				else
					moveY = lCentre - (pos - i);
				
				if (moveY >= 0 && moveY < lMax)
				{
					if (i == pos && depth == 2)
						_wputstring(lWindow, moveY, 1, WHITE, it, true);
					else if	(moveY == 0 || moveY == (lMax - 1))
						_wputstring(lWindow, moveY, 1, DARKGRAY, it);
					else
						_wputstring(lWindow, moveY, 1, it.color, it);
				}
				i++;
			}
		}

		if (depth == 1)
		{
			supermenu();
		}
		else
		{
			menu_move = HOVER;

			int eff_menu = menu;
			if (eff_menu > (int)Menu.size() - 1)
				eff_menu = Menu.size() - 1;

			if (selection && !Menu[eff_menu].size())
				empty_menu(rWindow);
			else
				describe(rWindow);

			show_window(window);

#ifdef _LIBTCOD_
			if (selection)
				show_window(lWindow);

			if (party_view)
				show_window(bWindow);

			show_window(rWindow);
#endif

			ch = _getch(state);

			if (selection && !Menu[eff_menu].size() && (ch == KEY_ACCEPT))
				ch = _KEY_RIGHT;

			switch(ch)
			{
			case _KEY_UP:
				if (selection && pos == 0)
					pos = Menu[eff_menu].size() - 1;
				else
					pos--;
				break;

			case _KEY_DOWN:
				pos++;
				break;

			case _KEY_LEFT:
			case KEY_PGUP:
				if (menu == 0)
						menu = Party.Player.size() - 1;
					else
						menu--;
				break;

			case _KEY_RIGHT:
			case KEY_PGDN:
				if (menu == Party.Player.size() - 1)
					menu = 0;
				else
					menu++;
				break;

			case KEY_ACCEPT:
				select();
				break;

			case _KEY_CANCEL:
				menu_move = CLIMB;
				break;
			}
		}

		if (!party_view)
			menu = 0;

		eff_menu = menu;
		if (eff_menu > Menu.size() - 1)
			eff_menu = Menu.size() - 1;

		if (selection)
		{
			if (pos > (int)Menu[eff_menu].size() - 1)
				pos = 0;
			if (pos < 0)
				pos = 0;
		}

		depth += menu_move;

	} while(depth);

	_delwin(rWindow);
	
	if (selection)
		_delwin(lWindow);

	if (party_view)
		_delwin(bWindow);

	unstack_window();
	show_window(messageWindow);

	if (ch == KEY_ACCEPT)
		return std::pair <int, int>(menu, pos + 1);
	else
		return std::pair <int, int>(0, 0);
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

#ifdef _CURSES_
	wattron(leftWindow, BORDER_COLOR);
	mvwvline(leftWindow, 0, 19, ACS_VLINE, LEFT_HEIGHT);
	wattroff(leftWindow, BORDER_COLOR);
#endif

#ifdef _LIBTCOD_
	_setColor(leftWindow, BORDER_COLOR);
	leftWindow->vline(19, 0, LEFT_HEIGHT);
#endif

	_wputchar(mainWindow, 21, 19, ACS_COMBINE_BTEE, BORDER_COLOR);

	show_window(leftWindow);
}

int get_player_battle_command(const std::vector<std::string>& selection,
							  int pos)
{
	if (pos > (int)selection.size() - 1)
			pos = 3;

	int ch;

	_wintype* selectWindow = _derwin(rightWindow, 5, 30, 15, 3);

	cbox(selectWindow, BORDER_COLOR);

	do
	{
		for (unsigned int i = 0; i < selection.size(); i++)
		{
			_colortype color = LIGHTGRAY;

			int printx, printy;
			
			if (i < 3)
			{
				printx = 1;
				printy = i + 1;
			}
			else if (i < 6)
			{
				printx = 7;
				printy = i % 3 + 1;
			}
			else
			{
				printx = 15;
				printy = i % 3 + 1;
			}

			_wputstring(selectWindow, printy, printx, color, selection[i],
				(i == pos));
		}

		show_window(selectWindow);

		ch = _getch();

		switch(ch)
		{
		case _KEY_UP:
			if (!(pos % 3))
				pos += 2;
			else
				pos--;
			break;

		case _KEY_DOWN:
			if (pos % 3 == 2)
				pos -= 2;
			else
				pos++;
			break;

		case _KEY_LEFT:
			if (pos < 3)
				pos += 6;
			else
				pos -= 3;
			break;

		case _KEY_RIGHT:
			if (pos > 5)
				pos -= 6;
			else
				pos += 3;
			break;
		}

		if (pos > (int)selection.size() - 1)
			pos = selection.size() - 1;

	} while ((ch != KEY_ACCEPT) && (ch != 'C'));

	_delwin(selectWindow);

	if (ch == KEY_ACCEPT)
		return pos;
	else
		return -1;
}
