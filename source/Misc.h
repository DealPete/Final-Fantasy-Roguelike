#ifndef __MISC_H_
#define __MISC_H_

#include <sstream>
#include <stdlib.h>

#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Output.h"

const unsigned char	RACE_NONE =			0x00;
const unsigned char RACE_CONSTRUCT =	0x01;
const unsigned char RACE_MAGE =			0x02;
const unsigned char RACE_GIANT =		0x04;
const unsigned char RACE_DRAGON =		0x08;

const unsigned char ELEM_NONE =		0x00;
const unsigned char ELEM_WATER =	0x01;
const unsigned char ELEM_ICE =		0x02;
const unsigned char ELEM_HOLY =		0x04;
const unsigned char ELEM_EARTH =	0x08;
const unsigned char ELEM_DARK =		0x10;
const unsigned char ELEM_BOLT =		0x20;
const unsigned char ELEM_AIR =		0x40;
const unsigned char ELEM_FIRE =		0x80;

const unsigned long ST_NONE =		0x00000000;

// resistable
const unsigned long ST_SLEEP =		0x00000001;
const unsigned long ST_STOP =		0x00000002;
const unsigned long ST_STUN =		0x00000004;

const unsigned long ST_BERSERK =	0x00000008;
const unsigned long ST_BLIND =		0x00000010;
const unsigned long ST_BLINK =		0x00000020;
const unsigned long ST_CALCIFY =	0x00000040;
const unsigned long ST_CHARM =		0x00000080;
const unsigned long ST_CONFUSE =	0x00000100;
const unsigned long ST_COUNT =		0x00000200;
const unsigned long ST_CRITICAL =	0x00000400;
const unsigned long ST_FATAL =		0x00000800;
const unsigned long ST_FLOAT =		0x00001000;
const unsigned long ST_FROG =		0x00002000;
const unsigned long ST_HALT =		0x00004000;
const unsigned long ST_HASTE =		0x00008000;
const unsigned long ST_INVITE =		0x00010000;
const unsigned long ST_KO =			0x00020000;
const unsigned long ST_LOCK =		0x00040000;
const unsigned long ST_MUTE =		0x00080000;
const unsigned long ST_PETRIFY =	0x00100000;
const unsigned long ST_POISON =		0x00200000;
const unsigned long ST_PROTECT =	0x00400000;
const unsigned long ST_QUICK =		0x00800000;
const unsigned long ST_REFLECT =	0x01000000;
const unsigned long ST_REGEN =		0x02000000;
const unsigned long ST_RERAISE =	0x04000000;
const unsigned long ST_SHELL =		0x08000000;
const unsigned long ST_SLOW =		0x10000000;
const unsigned long ST_UNDEAD =		0x20000000;
const unsigned long ST_BLINKx2 =	0x40000000;

const unsigned long ST_REMAIN = ST_FLOAT | ST_KO | ST_PETRIFY; 
const unsigned long ST_DEFEATING = ST_KO | ST_PETRIFY | ST_INVITE;
const unsigned long ST_INCAPACITATING = ST_KO | ST_PETRIFY |
						ST_STOP | ST_SLEEP | ST_STUN;
const unsigned long ST_DISTRACTING = ST_CHARM | ST_CONFUSE | ST_BERSERK;
const unsigned long ST_BAD = ST_BERSERK | ST_BLIND | ST_CALCIFY |
						ST_CHARM | ST_CONFUSE | ST_COUNT | ST_CRITICAL |
						ST_FATAL | ST_FROG | ST_HALT | ST_INVITE | ST_KO |
						ST_LOCK | ST_MUTE | ST_PETRIFY | ST_POISON |
						ST_SLEEP | ST_SLOW | ST_STOP | ST_STUN |
						ST_UNDEAD;

const unsigned char DEBUG_NO_COMBAT = 0x01;

const unsigned char CONFIG_IRONMAN = 0x01;
const unsigned char CONFIG_CHAOS = 0x02;

const unsigned char CONFIG_LUCK = 0x10;

extern unsigned char debug_flags;
extern unsigned char config_flags;

enum suit_type
{
	SUIT_SPADE,
	SUIT_CLUB,
	SUIT_DIAMOND,
	SUIT_HEART
};

enum target_type
{
	TRGT_NONE,
	TRGT_ANY,
	TRGT_SINGLE,
	TRGT_GROUP,
	TRGT_SELF,
	TRGT_OTHER,
	TRGT_RANDOM,
	TRGT_RANDOM_GROUP,
	TRGT_ALL
};

enum item_type
{
	ITEM_NONE,
	ITEM_WEAPON,
	ITEM_ACCESSORY,
	ITEM_BODY,
	ITEM_HEAD,
	ITEM_ITEM,
};

enum AI_type
{
	AI_ENEMY,
	AI_ALLY,
	AI_KOALLY
};


extern char Suits[];
extern char Ranks[];

class CAbility
{
public:
	static CAbility* get_ability(std::string ability_name);

	int serial;
	std::string name, type;
	target_type target;
};

template <class T>
inline std::string to_string (const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

inline std::string a_or_an(const std::string& str)
{
	std::string vowels = "aeiouAEIOU";
	if (vowels.find(str[0]) == std::string::npos)
		return "a";
	else
		return "an";
}

template <class C>
struct sort_serial
{
	bool operator() (C* lhs, C* rhs) const
	{
		return lhs->serial < rhs->serial;
	}
};

int ni(buffer& buf);
char nch(buffer& buf);
std::string ns(buffer& buf);
int dv(int dividend, int divisor);
int dv_percentage(int dividend, int divisor);
void quit_game(int err_code);
buffer status_to_buffer(unsigned long, int time_counters = 0,
	int num_blink = 0, int calcify_counters = 0);
buffer element_to_buffer(unsigned char);
buffer race_to_buffer(unsigned char);
std::string buffer_to_string(const buffer&);
std::string comma_list(const buffer&);

extern std::deque<CAbility> Ability;
extern std::map<std::string, CAbility*> abilitymap;
extern std::set<std::string> Element;
extern std::map<std::string, unsigned char> elementmap;
extern std::vector<std::string> Race;
extern std::map<std::string, unsigned char> racemap;
extern std::set<std::string> Status;
extern std::map<std::string, unsigned long> statusmap;

#endif
