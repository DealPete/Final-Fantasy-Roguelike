#include "Message.h"
#include "Output.h"
#include "Party.h"

#include <stack>

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
