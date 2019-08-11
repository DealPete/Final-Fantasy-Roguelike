#ifndef __PARSE_H_
#define __PARSE_H_

#include <stdlib.h>
#include <string>

#include "Misc.h"

#include <fstream>
std::ofstream file("Parser/debug.txt", std::ios::out);
#define BOOST_SPIRIT_DEBUG_OUT file

#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/classic_core.hpp>

using namespace BOOST_SPIRIT_CLASSIC_NS;

bool parse_spell(std::string& spell_desc)
{
	rule<> race = str_p("Constructs/") | "Mages/" | "Giants/" | "Dragons/";

	rule<> status = str_p("Berserk/") | "Blind/" | "Berserk/" |	"Blind/"
		| "Blink/" | "Calcify/" | "Charm/" | "Confuse/" | "Count/" | "Critical/"
		| "Fatal/" | "Float/" | "Frog/" | "Halt/" | "Haste/" | "Invite/"
		| "KO/"	| "Lock/" | "Mini/" | "Mute/" | "Petrify/" | "Pig/" | "Poison/"
		| "Protect/" | "Quick/" | "Reflect/" | "Regen/" | "Reraise/" | "Shell/"
		| "Sleep/" | "Slow/" | "Stop/" | "Stun/" | "Undead/" | "Blinkx2/";

	rule<> status_list = *(status);
	
	rule<> degree = str_p("min/") | "low/" | "med/" | "high/" | "max/";
	rule<> element = str_p("fire/") | "water/" | "ice/" | "bolt/" | "earth/" |
		"dark/" | "holy/" | "air/" | "neutral/";
	rule<> stat = str_p("Str/") | "Int/" | "Vit/" | "Agi/" | "Eva/" |
		"Hit/" | "Atk/";
	rule<> amount = str_p("caster_HP/") | "all/" | "lost/" | "half_lost/";

	rule<> special
		=	str_p("change_row/")
		|	"charge/"
		|	"dark_magic/"
			>>	status
		|	"draw/"
			>> int_p >> "/"
		|	"exchange_weapon/"
		|	"exit/"
		|	"junction/"
		|	"pep_up/"
		|	"red_feast/"
		|	"scan/"
		|	"steal/"
			>>	(str_p("gil/") | "item/")
		|	"warp/"
		;

	rule<> special_attack
		=	str_p("add/")
			>> status
		|	"drainHP/"
		|	"drainMP/"
		|	element
		|	"exchange weapon/"
		|	"ignore_evasion/"
		|	"ignore_row/"
		|	"may_change_row/"
		|	"meatbone_slash/"
		|	"modifier_minus_one/"
		|	"twice/"
		|	"d4_times/"
		;

	rule<> condition
		=	status
		|	"monster/"
		;
	
	rule<> effect
		=	"add/"
			>> !str_p("random_of/")
			>> status_list
		|	"attack/"
			>> !element
			>> !("hurts/" >> +race)
			>> !(+special_attack)
		|	"cancel/"
			>> status_list
		|	"caster/"
		|	"damage/"
			>> degree
			>> element
		|	"drain/"
			>> degree
		|	"escape/"
		|	"gainMP/"
			>> degree
		|	"give_resist/"
			>> +element
		|	"give_weakness/"
			>> +element
		|	"heal/"
			>> (degree | amount)
		|	"loseHP/"
			>>	amount
			>>	!str_p("both/")
		|	"remove/"
			>> status_list
		|	str_p("special/")
			>> special
		|	"stat/"
			>> stat
			>> int_p >> "/"
		|	"toggle/"
			>> status_list
		;
		
	rule<> effect_list
		=	"chance/"
			>> degree
			>> +effect
		|	"party/"
			>> effect_list
		|	str_p("if/")
			>> condition
			>> +effect_list
		|	+effect
			>> !effect_list
		;

	rule<> spell = effect_list >> *("&/" >> effect_list)
					| "none/";
    if (parse(spell_desc.c_str(), spell).full)
		return true;

    return false;
}

#endif	// __PARSE_H_
