#include "Output.h"
#include "Party.h"
#include "Window.h"

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

std::pair<int, int> CSelectWindow::operator()()
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
				_wputchar(window, 1, i * 19 + 2, i + 0x31, WHITE);
				_wputstring(window, 1, i * 19 + 4, TEXT_COLOR, it.name,
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
			case '1':
			case '2':
			case '3':
			case '4':
			{
				int player_num = ch - 0x31;
				if (player_num < Party.Player.size())
					menu = player_num;
				break;
			}

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

			case '\t':
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

