#ifndef __MENU_H_
#define __MENU_H_

#include "Input.h"
#include "Output.h"

// returns 0 for cancel, nonzero for position in buffer.
int menu_select(int y, int x, const buffer&, void (*desc)(int) = NULL);

class CPlayer;

// CSelectWindow - This class represents a full-screen window used to manage party members.
// e.g. Status Window, Shop Window, Equip Window, Job Window.
//
// If "party_view" is true, the top of the window has a tab for each party member.
// If the menu is only for one player (such as when it's his turn in battle),
// set "player" to that player.
// If "selection" is true, then the window is divided into a menu window on the left,
// filled by the "Menu" vector, and a description on the right.
//
// The virtual function "describe" is called to fill the right hand window with text.
// The member variable "menu" gives the index of the menu item selected.
// If the menu is empty, the virtual "empty_menu" function is called instead.
//
// After creating a window object, use the () operator (i.e. call it like a function) to display it.
class CSelectWindow
{
public:
	_wintype *window, *bWindow, *lWindow, *rWindow;
	CPlayer* player;

	bool party_view,    // Tab bar with party members.
	     selection;     // false if you just want to display
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

	CSelectWindow(_wintype* w, bool p = true, bool s = true, int m = 18,
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

	// Used for a menu in the large window at bottom right.
	// e.g. select equip set when shopping or select equip slot in Equip Window.
	virtual void supermenu()
	{}

	virtual ~CSelectWindow() {}
};

#endif // __MENU_H_
