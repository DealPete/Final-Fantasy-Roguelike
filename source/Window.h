#ifndef __MENU_H_
#define __MENU_H_

#include "Input.h"
#include "Output.h"

// returns 0 for cancel, nonzero for position in buffer.
int menu_select(int y, int x, const buffer&, void (*desc)(int) = NULL);

class CPlayer;

class CSelectWindow
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

	virtual void supermenu()
	{}

	virtual ~CSelectWindow() {}
};

#endif // __MENU_H_
