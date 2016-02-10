#ifndef __OUTPUT_H_
#define __OUTPUT_H_

#include <cstdarg>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "Defines.h"
#include "Input.h"
#include _MEDIA_HEADER_

#define RIGHT_ARROW 175
#define ACS_DBL_HLINE 205
#define ACS_DBL_VLINE 186
#define ACS_DBL_TTEE 203
#define ACS_DBL_BTEE 202
#define ACS_DBL_LTEE 204
#define ACS_DBL_RTEE 185
#define ACS_COMBINE_BTEE 207

extern std::string font_file;
extern std::string font_orient;

#ifdef _LIBTCOD_

const TCODColor BLACK = TCODColor::black;
const TCODColor BLUE = TCODColor::blue;
const TCODColor GREEN = TCODColor::green;
const TCODColor CYAN = TCODColor::cyan;
const TCODColor RED = TCODColor::red;
const TCODColor MAGENTA = TCODColor::magenta;
const TCODColor YELLOW = TCODColor::yellow;
const TCODColor LIGHTGRAY = TCODColor::lightGrey;
const TCODColor DARKGRAY = TCODColor::darkGrey;
const TCODColor LIGHTBLUE = TCODColor::lightBlue;
const TCODColor LIGHTGREEN = TCODColor::lightGreen;
const TCODColor LIGHTCYAN = TCODColor::lightCyan;
const TCODColor LIGHTRED = TCODColor::lightRed;
const TCODColor LIGHTMAGENTA = TCODColor::lightMagenta;
const TCODColor LIGHTYELLOW = TCODColor::lightYellow;
const TCODColor WHITE = TCODColor::white;

#define ACS_LTEE TCOD_CHAR_TEEE
#define ACS_RTEE TCOD_CHAR_TEEW
#define ACS_TTEE TCOD_CHAR_TEEN
#define ACS_BTEE TCOD_CHAR_TEES

#define _getheight(WND) WND->getHeight() 
#define _getwidth(WND) WND->getWidth()
#define _setColor(WND,CH) WND->setDefaultForeground(CH)
#define _werase(WND) WND->clear()

typedef TCODConsole _wintype;
typedef TCODColor _colortype;
typedef int _chtype;

extern std::map<_wintype*, int> winXmap;
extern std::map<_wintype*, int> winYmap;

#endif // _LIBTCOD_

#ifdef _CURSES_

//#undef clear
//#undef erase
//#undef move

const chtype BLACK = COLOR_PAIR(8);
const chtype BLUE = COLOR_PAIR(1);
const chtype GREEN = COLOR_PAIR(2);
const chtype CYAN = COLOR_PAIR(3);
const chtype RED = COLOR_PAIR(4);
const chtype MAGENTA = COLOR_PAIR(5);
const chtype YELLOW = COLOR_PAIR(6);
const chtype LIGHTGRAY = COLOR_PAIR(7);
const chtype DARKGRAY = COLOR_PAIR(8) | A_BOLD;
const chtype LIGHTBLUE = COLOR_PAIR(1) | A_BOLD;
const chtype LIGHTGREEN = COLOR_PAIR(2) | A_BOLD;
const chtype LIGHTCYAN = COLOR_PAIR(3) | A_BOLD;
const chtype LIGHTRED = COLOR_PAIR(4) | A_BOLD;
const chtype LIGHTMAGENTA = COLOR_PAIR(5) | A_BOLD;
const chtype LIGHTYELLOW = COLOR_PAIR(6) | A_BOLD;
const chtype WHITE = COLOR_PAIR(7) | A_BOLD;

#define _clear() clear()
// getmaxy and getmaxx are badly named.
#define _getheight(WND) getmaxy(WND)
#define _getwidth(WND) getmaxx(WND)
#define _newwin(Y1,X1,Y2,X2) newwin(Y1,X1,Y2,X2)
#define _setColor(WND, C) wattron(WND, C)
#define _werase(WND) werase(WND)

typedef WINDOW _wintype;
typedef chtype _colortype;
typedef chtype _chtype;

#endif // _CURSES_

const _colortype BORDER_COLOR = YELLOW;
const _colortype ERROR_COLOR = WHITE;
const _colortype INFO_COLOR = CYAN;
const _colortype TEXT_COLOR = LIGHTGRAY;
const _colortype TITLE_COLOR = YELLOW;

extern std::map<std::string, _colortype> colormap;

class color_string : public std::string
{
public:
	_colortype color;

	color_string(std::string s, _colortype c) : std::string(s), color(c)
	{}

	color_string() : color(LIGHTGRAY)
	{}

	color_string(const std::string& s) : std::string(s)
	{
		color = LIGHTGRAY;
	}

	color_string(const char* c) : std::string(c)
	{
		color = LIGHTGRAY;
	}
};

class buffer : public std::list<color_string>
{
private:
	int width;

public:
	buffer(int w = 80) : width(w)
	{}

	// more robust than push_back
	void push(std::string text, _colortype color = LIGHTGRAY);
};

void init_color();
void init_display();
void end_display();
int format_print(_wintype*, int y, std::string);
int format_print(_wintype*, int y, const char*, ...);
int cformat_print(_wintype*, _colortype color, int y, std::string);
int cformat_print(_wintype*, _colortype color, int y, const char*, ...);

int _wputchar(_wintype*, int y, int x, _chtype, _colortype, bool reverse =
	false);
int _wputcstr_reverse(_wintype*, int y, int x, _colortype,
	const char*, ...);
int _wputcstr(_wintype*, int y, int x, _colortype, const char*, ...);
int _wputstring(_wintype*, int y, int x, _colortype, std::string, bool
	reverse = false);

/*
int _wprintw(_wintype*, std::string);
int _wprintw(_wintype*, const char*, ...);
int wcprintw(_wintype*, _colortype color, std::string);
int wcprintw(_wintype*, _colortype color, const char*, ...);
*/

void wcaddch(_wintype*, _chtype ch, _colortype color);
void _wrefresh(_wintype*);

void _delwin(_wintype*);
_wintype* _derwin(_wintype*, int nlines, int ncols, int begin_y,
                 int begin_x);
_wintype* _dupwin(_wintype*);
void show_window(_wintype* window);
void draw_explore_screen();
void stack_window();
void unstack_window();
void cbox(_wintype* window, _colortype color);

void browse_buffer(_wintype*, buffer*);
void draw_buffer(_wintype*, buffer*, bool in_box);

// returns 0 for cancel, nonzero for position in buffer.
int menu_select(int y, int x, const buffer&, void (*desc)(int) = NULL);

void show_enemy_pad(int y);
void prepare_combat_output();
int get_player_battle_command(const std::vector<std::string>&,
							  int pos);

class CPlayer;

class CSelectFunctor
{
public:
	_wintype *window, *bWindow, *lWindow, *rWindow;
	CPlayer* player;

	bool party_view,	// Seperate menu for each player.
		selection;		// false if you just want to display
						// information (like the status screen).
	
	int depth, menu, pos, middle;
	std::vector<buffer> Menu;
	input_type state;
	int suppos;

	enum {
		CLIMB = -1,
		HOVER = 0,
		DIVE = 1
	} menu_move;

	CSelectFunctor(_wintype* w, bool p = true, bool s = true, int m = 18,
		input_type i = INPUT_NORMAL)
		: player(NULL), menu(0), pos(0), suppos(0), window(w),
		party_view(p), selection(s), middle(m), state(i)
	{}

	std::pair<int, int> operator()();
	virtual void describe(_wintype*) = 0;
	virtual void empty_menu(_wintype*) = 0;
	
	virtual void select()
	{
		menu_move = CLIMB;
	}

	virtual void supermenu()
	{}

	virtual ~CSelectFunctor() {}
};

extern _wintype* battleQWindow;
extern _wintype* bigWindow;
extern _wintype* enemyPad;
extern _wintype* helpWindow;
extern _wintype* leftWindow;
extern _wintype* mainWindow;
extern _wintype* messageWindow;
extern _wintype* rightWindow;
extern _wintype* upperWindow;
extern _wintype* levelupWindow;

const unsigned int LEFT_UL_Y = 0;
const unsigned int LEFT_UL_X = 0;
const unsigned int LEFT_HEIGHT = 21;
const unsigned int LEFT_WIDTH = 41;

const unsigned int RIGHT_UL_Y = 0;
const unsigned int RIGHT_UL_X = 42;
const unsigned int RIGHT_HEIGHT = 21;
const unsigned int RIGHT_WIDTH = 38;

const unsigned int HELP_UL_Y = 1;
const unsigned int HELP_UL_X = 8;
const unsigned int HELP_HEIGHT = 17;
const unsigned int HELP_WIDTH = 65;

const unsigned int UPPER_UL_Y = 0;
const unsigned int UPPER_UL_X = 0;
const unsigned int UPPER_HEIGHT = 22;
const unsigned int UPPER_WIDTH = 80;

const unsigned int BIG_UL_Y = 0;
const unsigned int BIG_UL_X = 0;
const unsigned int BIG_HEIGHT = 25;
const unsigned int BIG_WIDTH = 80;

const unsigned int LVUP_UL_Y = 3;
const unsigned int LVUP_UL_X = 24;
const unsigned int LVUP_HEIGHT = 19;
const unsigned int LVUP_WIDTH = 35;

const unsigned int MSG_UL_Y = 22;
const unsigned int MSG_UL_X = 0;
const unsigned int MSG_HEIGHT = 3;
const unsigned int MSG_WIDTH = 80;

const unsigned int ENEMY_PAD_SIZE = 100;
const unsigned int MSG_BUF_WIDTH = 78;
/*
const int MSG_UL_Y = 22;
const int MSG_UL_X = 0;
const int MSG_HEIGHT = 3;
const int MSG_WIDTH = 80;

const int MSG_UL_Y = 22;
const int MSG_UL_X = 0;
const int MSG_HEIGHT = 3;
const int MSG_WIDTH = 80; */

#endif // __OUTPUT_H_
