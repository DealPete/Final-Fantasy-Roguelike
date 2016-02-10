#include <assert.h>

#include <string>

#include "Message.h"
#include "Input.h"
#include "Output.h"

static bool more_line = false;
static int unseen_lines = 0;

buffer help_buffer;
buffer message_buffer;

std::string centre_string(const std::string& con_str, int width)
{
	std::string str = "";
	str.insert(str.begin(), (width - con_str.length()) / 2, ' ');
	return str + con_str;
}

//	Splits up the string if it doesn't fit in "width"; centres strings
//  beginning with '@'.
void format_string(buffer& buf, std::string str, unsigned int width)
{
	std::string whitespace = " \n\r\t";
	bool centre = false;

	if (str[0] == '@')
	{
		str.erase(0, 1);
		centre = true;
	}

	while (str.length() > width)
	{
		int i, j;
		for (i = width - 1;
			whitespace.find(str[i]) == std::string::npos; i--);
		for (j = i; whitespace.find(str[j]) != std::string::npos; j--);
		buf.push_back(str.substr(0, j + 1));
		str = str.substr(i + 1);
	}

	buf.push_back(str);

	if (centre)
	{
		for(std::string& it : buf)
			it = centre_string(it, width);
	}
}

// draws the current message buffer on the screen.  Lines the user has
// seen are in DARKGRAY; lines he hasn't seen are in WHITE.
void draw_message_window()
{
	_werase(messageWindow);
	buffer::const_iterator it = message_buffer.begin();

	for(unsigned int i = 0; (i < MSG_HEIGHT) &&
		(i < message_buffer.size()); i++, it++)
	{
		_wputstring(messageWindow, MSG_HEIGHT - i - 1, 0,
			unseen_lines == 0 ? DARKGRAY : WHITE, *it);

		if (unseen_lines > 0) unseen_lines--;
	}
	show_window(messageWindow);
}

void error(const std::string msg)
{
	message(msg);
	flush_message_buf();
	_getch(INPUT_ERROR);
	quit_game(1);
}

void error(const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	char buf[1024];

	vsprintf(buf, format, argp);

	va_end(argp);

	error(std::string(buf));
}

void more_top_line()
{
	std::string prev_str = message_buffer.front();
	message_buffer.pop_front();
	unseen_lines--;

	message(prev_str + " --More--");
	while (_getch(INPUT_MORE) != KEY_ACCEPT);

	// take the more prompt out of the message buffer.
	message_buffer.front().erase(message_buffer.front().find("--More--"), 8);
	if (message_buffer.front().empty())
		message_buffer.pop_front();
}

// processes game messages to be sent to the message window.
void message(const std::string& msg)
{
	if (more_line)
	{
		more_line = false;
		more_top_line();
	}

	buffer buf;

	assert(message_buffer.size() <= MESSAGE_BUFFER_SIZE);

	format_string(buf, msg, MSG_BUF_WIDTH - 2);

	for(std::string& it : buf)
	{
		if (message_buffer.size() == MESSAGE_BUFFER_SIZE)
			message_buffer.pop_back();
		message_buffer.push_front(it);
		unseen_lines++;
	}
}

// converts messages formatted C style to C++ style.
void message(const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	char buf[1024];

	vsprintf(buf, format, argp);

	va_end(argp);

	message(std::string(buf));
}

void more(const std::string& msg)
{
	message(msg);
	more_line = true;
}

void more(const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	char buf[1024];

	vsprintf(buf, format, argp);

	va_end(argp);

	more(std::string(buf));
}

// tries to add "msg" to the end of the next message to be
// displayed.  If there are no unseen lines, it just adds "msg"
// to the message buffer.
void add_message(const std::string& msg)
{
	if (!unseen_lines)
		message(msg);
	else
	{
		std::string str = message_buffer.front();
		message_buffer.pop_front();
		unseen_lines--;

		bool m = more_line;
		more_line = false;
		message(str + "  " + msg);
		more_line = m;
	}
}

void add_message(const char* format, ...)
{
	va_list argp;
	va_start(argp, format);

	char buf[1024];

	vsprintf(buf, format, argp);

	va_end(argp);

	add_message(std::string(buf));
}

void flush_message_buf(bool more_prompt)
{
	more_line = false;

	if (unseen_lines)
	{
		if (more_prompt)
			more_top_line();
			
		draw_message_window();
	}
}

void prompt(const std::string& msg)
{
	flush_message_buf(true);

	// the message window should always contain the most recently
	// displayed messages, so create a temporary duplicate.
	_wintype* tmpWindow = _dupwin(messageWindow);

	_werase(tmpWindow);
	_wputstring(tmpWindow, 1, 0, WHITE, msg);

	show_window(tmpWindow);
}

void prompt_input(const std::string& msg, std::string& input)
{
	flush_message_buf(true);
	char tmp[16];
	prompt(msg);
	_mvgetnstr(MSG_UL_Y + 1, msg.length(), tmp, 15);
	input = std::string(tmp);
}

bool yesno()
{
	buffer buf;
	buf.push_back("Yes");
	buf.push_back("No");
				
	int select = menu_select(14, 5, buf);

	return (select == 1);
}
