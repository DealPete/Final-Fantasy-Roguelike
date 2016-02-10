#include <stdlib.h>

#include "Map.h"
#include "Misc.h"
#include "Message.h"
#include "Output.h"

#include <string>

CAbility* CAbility::get_ability(std::string ability_name)
{
	if (!abilitymap[ability_name])
		error("ERROR: Cannot find ability " + ability_name + ".");
	return abilitymap[ability_name];
}

// ni stands for "next int".
int ni(buffer& buf)
{
	int i = atoi(buf.front().c_str());
	if (i == 0)
		for(char ch : buf.front())
			if (!isdigit(ch))
				error("Expected number, found \"" +
					buf.front() + "\".");
	buf.pop_front();
	return i;
}

// nch stands of "next char".
char nch(buffer& buf)
{
	char ch = (buf.front()[0]);
	buf.pop_front();
	return ch;
}

// ns stands for "next std::string".
std::string ns(buffer& buf)
{
	std::string s = buf.front();
	buf.pop_front();
	return s;
}

int dv(int dividend, int divisor)
{
	if (divisor == 0)
		error("Division by zero!");
	int quotient = dividend / divisor;
	if (dividend % divisor) quotient++;
	return quotient;
}

int dv_percentage(int dividend, int divisor)
{
	if (divisor == 0)
		error("Division by zero!");
	if (dividend == 0)
		return -1;
	if (dividend < 0)
		return dividend * divisor;
	int quotient = dividend / divisor;
	return quotient;
}

void quit_game(int err_code)
{
	end_display();
	exit(err_code);
}

buffer status_to_buffer(unsigned long status, int t, int b, int c)
{
	buffer buf;
	for(std::string it : Status)
		if (status & statusmap[it])
		{
			if (it == "Calcify" && c > 0)
				buf.push_back(it + "[" + to_string(c) + "]");
			else if (it == "Count" && t > 0)
				buf.push_back(it + "[" + to_string(t) + "]");
			else if (it == "Blink" && b > 1)
				buf.push_back(it + "[" + to_string(b) + "]");
			else
				buf.push_back(it);
		}

	return buf;
}

buffer element_to_buffer(unsigned char element)
{
	buffer buf;
	for(std::string it : Element)
		if (element & elementmap[it])
			buf.push_back(it);

	return buf;
}

buffer race_to_buffer(unsigned char race)
{
	buffer buf;
	for(std::string it : Race)
		if (race & racemap[it])
			buf.push_back(it);

	return buf;
}

std::string buffer_to_string(const buffer& buf)
{
	std::string str;
	for(const std::string& it : buf)
		str += " " + it;

	if (str == "")
		return "";

	return str.substr(1);
}

std::string comma_list(const buffer& buf)
{
	std::string str;
	if (buf.size() == 0)
		str = "";
	else if (buf.size() == 1)
		str = buf.front();
	else if (buf.size() == 2)
		str = buf.front() + " and " + buf.back();
	else
	{
		buffer::const_iterator it = buf.begin();
		str = "and " + *(it++);

		for (;it != buf.end(); it++)
			str = *it + ", " + str;
	}
	return str;
}

char Suits[] = { 6, 5, 4, 3 };

char Ranks[] = { 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};

std::deque<CAbility> Ability;
std::map<std::string, CAbility*> abilitymap;
std::set<std::string> Element;
std::map<std::string, unsigned char> elementmap;
std::vector<std::string> Race;
std::map<std::string, unsigned char> racemap;
std::set<std::string> Status;
std::map<std::string, unsigned long> statusmap;

unsigned char debug_flags = 0;
unsigned char config_flags = 0;
