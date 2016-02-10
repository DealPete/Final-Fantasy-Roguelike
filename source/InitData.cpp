#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "Armor.h"
#include "InitData.h"
#include "Items.h"
#include "Jobs.h"
#include "Map.h"
#include "Message.h"
#include "Misc.h"
#include "Monsters.h"
#include "Output.h"
#include "Quests.h"
#include "Spells.h"
#include "Target.h"
#include "Town.h"
#include "Weapons.h"

#ifdef _DEBUG
#include "Parser/Parse.h"
#endif

static std::string working_file;

bool strip_asterisk(std::string& str)
{
	if (str == "")
		return false;

	std::string::iterator it = str.end();
	
	if (*(--it) == '*')
	{
		str.erase(it);
		return true;
	}

	return false;
}

void tokenize(std::string filename, buffer& token)
{
	std::string whitespace = " \n\r\t:";
	std::string tmp;
	char ch;
	bool new_token = true;
	std::ifstream file(filename.c_str(), std::ios::in);

	if (!file.is_open()) error("Cannot open file " + working_file + "!");

	file.get(ch);

	while(!file.eof())
	{
		if (ch == '#')
		{
			getline(file, tmp);
			new_token = true;
		}
		else if (ch == '"')
		{
			char prev_ch = '#';
			token.push_back("");
			do
			{
				file.get(ch);
				if (file.eof())
					error("Opening \" not matched with closing \" in " +
						working_file + ".");
				if (ch == '\n' || ch == '\r')
				{
					if (whitespace.find(prev_ch) == std::string::npos)
						token.back() += ' ';
				}
				else if (ch != '"')
					token.back() += ch;
				prev_ch = ch;
			} while (ch != '"');
			new_token = true;
		}
		else if (whitespace.find(ch) == std::string::npos)
		{
			if (new_token)
			{
				token.push_back("");
			}
			if (ch == '.')
				token.back() += " ";
			else
				token.back() += ch;
			new_token = false;
		}
		else new_token = true;
		file.get(ch);
	}
	file.close();
}

unsigned char races_parse(std::string races_list)
{
	unsigned char ch = RACE_NONE;
	for(unsigned int i = 0;i < races_list.length();i++)
	{
		switch (races_list[i])
		{
		case 'C':
			ch |= RACE_CONSTRUCT;
			break;

		case 'M':
			ch |= RACE_MAGE;
			break;

		case 'G':
			ch |= RACE_GIANT;
			break;

		case 'D':
			ch |= RACE_DRAGON;
			break;
		}
	}
	return ch;
}

unsigned char element_parse(std::string element_list)
{
	unsigned char ch = ELEM_NONE;
	for(unsigned int i = 0;i < element_list.length();i++)
	{
		switch (element_list[i])
		{
		case 'W':
			ch |= ELEM_WATER;
			break;

		case 'I':
			ch |= ELEM_ICE;
			break;

		case 'H':
			ch |= ELEM_HOLY;
			break;

		case 'E':
			ch |= ELEM_EARTH;
			break;

		case 'D':
			ch |= ELEM_DARK;
			break;

		case 'B':
			ch |= ELEM_BOLT;
			break;

		case 'A':
			ch |= ELEM_AIR;
			break;

		case 'F':
			ch |= ELEM_FIRE;
			break;
		}
	}
	return ch;
}

void load_abilities(buffer& category)
{
	std::string atype;

	while(!category.empty())
	{
		atype = ns(category);
		while (category.front() != "~")
		{
			Ability.push_back(CAbility());
			CAbility& ref = Ability.back();

			ref.serial = Ability.size() - 1;

			ref.type = atype;
			ref.name = ns(category);

			if (atype == "Command")
				ref.target = targetmap[ns(category)];
			else
				ref.target = TRGT_NONE;

			abilitymap[ref.name] = &Ability.back();
		}
		category.pop_front();
	}
}

void load_armor(buffer& category)
{
	std::string itemset = "error";
	while(!category.empty())
	{
		if (category.front() == "$")
		{
			category.pop_front();
			itemset = ns(category);
			ItemSet.push_back(CItemSet());
			ItemSet.back().serial = ItemSet.size() - 1;
			ItemSet.back().name = itemset;
			ItemSet.back().type = ITEM_BODY;
			itemsetmap[itemset] = &ItemSet.back();
		}
		else
		{
			Armor.push_back(CArmor());
			CArmor& ref = Armor.back();

			ref.Set = &ItemSet.back();
			
			ref.serial = Armor.size() - 1;

			ItemSet.back().insert(&ref);

			ref.Lv = ni(category);
			ref.name = ns(category);
			ref.HP = ni(category);
			ref.MP = ni(category);

			std::string type = ns(category);
			if (type == "Body")
				ref.type = ITEM_BODY;
			else if (type == "Head")
				ref.type = ITEM_HEAD;
			else
				ref.type = ITEM_ACCESSORY;

			ref.price = ni(category);

			while (category.front() != "~")
			{
				char code = nch(category);
				switch (code)
				{
				case 'a':
					ref.Agi = ni(category);
					break;

				case 'A':
					ref.always |= statusmap[ns(category)];
					break;

				case 'B':
					ref.starts |= statusmap[ns(category)];
					break;

				case 'C':
					ref.cancels |= statusmap[ns(category)];
					break;

				case 'e':
					ref.Eva = ni(category);
					break;

				case 'G':
					ref.grantsS = CSpell::get_spell(ns(category));
					break;

				case 'h':
					ref.Hit = ni(category);
					break;

				case 'I':
					ref.immune = element_parse(ns(category));
					break;

				case 'i':
					ref.Int = ni(category);
					break;

				case 'k':
					ref.Atk = ni(category);
					break;

				case 'l':
					ref.Luc = ni(category);
					break;

				case 'm':
					ref.MEv = ni(category);
					break;

				case 'R':
					ref.resists = element_parse(ns(category));
					break;

				case 'P':
					ref.seals = CAbility::get_ability(ns(category));
					break;

				case 's':
					ref.Str = ni(category);
					break;

				case 'S':
					ref.strengthens = element_parse(ns(category));
					break;

				case 'v':
					ref.Vit = ni(category);
					break;

				case 'w':
					ref.weight = ni(category);
					break;

				default:
					error("Didn't expect armor code %c for %s", code,
						ref.name.c_str());
				}
			}
		
			if (strip_asterisk(ref.name))
				ref.complex = true;
			else
				ref.complex = false;

			itemmap[ref.name] = &ref;
			
			if (ref.name == "None")
				noneItem[ref.type] = &ref;

			category.pop_front();
		}
	}
}

void load_domains(buffer& category)
{
	int domain_number;

	domain_number = ni(category);

	for(int i = 0;i < domain_number;i++)
	{
		int j = Domain.size();
		Domain.push_back(CDomain());
		Domain[j].serial = Domain.size() - 1;
		Domain[j].name = ns(category);
		Domain[j].entrance = ni(category);
		Domain[j].wall = ni(category);
		Domain[j].color = colormap[ns(category)];
		for(int k = 0;k < 13;k++)
		{
			if (ni(category) != (k + 1))
				error("ERROR: Expected level %d for domain %s.", k + 1,
					Domain[j].name.c_str());
			for(int l = 0;l < 4;l++)
			{
				std::string monster = ns(category);

				if (!monstermap[monster])
					error(
"Could not find monster %s, found in domain %s, level %d.", monster.c_str(),
						Domain[j].name.c_str(), k + 1);
				
				Domain[j].encounter.push_back(monstermap[monster]);
			}
		}
	}
}

void load_items(buffer& category)
{
	ItemSet.push_back(CItemSet());
	ItemSet.back().serial = ItemSet.size() - 1;
	ItemSet.back().name = "Item";
	ItemSet.back().type = ITEM_ITEM;
	itemsetmap["Item"] = &ItemSet.back();

	while(!category.empty())
	{
		Item.push_back(CItem());
		CItem& ref = Item.back();
		ref.type = ITEM_ITEM;

		ref.serial = Item.size() - 1;
		ref.Lv = ni(category);
		ref.name = ns(category);
		ref.price = ni(category);
		ref.description = ns(category);

		ref.Set = itemsetmap["Item"];

		if (strip_asterisk(ref.name))
			ref.complex = true;
		else
			ref.complex = false;

		itemmap[ref.name] = &ref;
	}
}

void load_jobs(buffer& category)
{
	while(!category.empty())
	{
		Job.push_back(CJob());
		CJob& ref = Job.back();

		ref.serial = Job.size() - 1;

		ref.title = ns(category);
		ref.Str = ni(category);
		ref.Agi = ni(category);
		ref.Vit = ni(category);
		ref.Int = ni(category);
		ref.Luc = ni(category);

		ref.description = ns(category);

		while (category.front() != "~")
		{
			switch(nch(category))
			{
			case 'a':
				ref.Ability.insert(CAbility::get_ability(ns(category)));
				break;

			case 'W':
				ref.WeaponSkill.insert(ns(category));
				break;

			case 'A':
				ref.ArmorSkill.insert(ns(category));
				break;
			}
		}
		category.pop_front();
	}
}

void load_monsters(buffer& category, bool bosses)
{
	while (!category.empty())
	{
		MonsterRace.push_back(CMonsterRace());
		CMonsterRace& ref = MonsterRace.back();
		ref.formMap["Default"] = new CMonsterForm("Default");
		ref.Lv = ni(category);
		ref.name = ns(category);
		
		ref.boss = bosses;
		
		ref.Vit = ni(category);
		ref.Agi = ni(category);
		ref.Eva = ni(category);
		ref.Hit = ni(category);
		ref.Atk = ni(category);
		ref.Int = ni(category);

		switch (nch(category))
		{
		case 'M':
			ref.sex = SEX_MALE;
			break;

		case 'F':
			ref.sex = SEX_FEMALE;
			break;

		case 'N':
			ref.sex = SEX_NONE;
			break;
		}

		if (!ref.boss)
		{
			ref.common_item = CItem::get_item(ns(category));
			ref.rare_item = CItem::get_item(ns(category));
		}

		while (category.front() != "~")
		{
			char code = nch(category);
			switch (code)
			{
			case 'a':
				ref.always |= statusmap[ns(category)];
				break;

			case 'A':
				ref.absorbs |= element_parse(ns(category));
				break;
				
			case 'B':
				ref.starts |= statusmap[ns(category)];
				break;

			case 'c':
				ref.starts_by_casting = ns(category);
				break;

			case 'C':
				ref.formMap["Default"]->Action[SUIT_CLUB] = ns(category);
				break;

			case 'D':
				ref.formMap["Default"]->Action[SUIT_DIAMOND] = ns(category);
				break;

			case 'H':
				ref.formMap["Default"]->Action[SUIT_HEART] = ns(category);
				break;

			case 'I':
				ref.immune = element_parse(ns(category));
				break;

			case 'P':
				ref.plural = ns(category);
				break;

			case 'r':
				ref.races |= races_parse(ns(category));
				break;

			case 'R':
				ref.resists = element_parse(ns(category));
				break;

			case 'S':
				ref.formMap["Default"]->Action[SUIT_SPADE] = ns(category);
				break;

			case 'W':
				ref.weak = element_parse(ns(category));
				break;

			case '+':
				ref.adds = statusmap[ns(category)];
				break;

			default:
				error("Didn't expect monster code %c for %s", code,
					ref.name.c_str());
			}

		}

		if (ref.plural == "") ref.plural = ref.name + 's';
		monstermap[ref.name] = &ref;

		if (ref.boss)
		{
			(void)ns(category);

			ref.greeting = ns(category);

			while (category.front() != "~")
			{
				std::string form = ns(category);
				if (form != "[")
				{
					ref.formMap[form] = new CMonsterForm(form);
					if (!ref.starting_form)
						ref.starting_form = ref.formMap[form];
					if (ns(category) != "[")
						error("Expected opening \"[\" for form %s of %s.",
							form.c_str(), ref.name.c_str());
				}
				else
				{
					form = "Default";
					ref.starting_form = ref.formMap[form];
				}

				int i = SUIT_SPADE;
				while (category.front() != "]")
				{
					if ((ref.formMap[form]->Action[i] = ns(category)) == "Form")
						ref.formMap[form]->changes_to = ns(category);

					if (i++ > SUIT_HEART)
						error("Expected closing \"]\" for form %s of %s.",
							form.c_str(), ref.name.c_str());
				}

				(void)ns(category);
			}
		}
		else
			ref.starting_form = ref.formMap["Default"];

		(void)ns(category);
	}
}

void load_quests(buffer& category)
{
	//Quest.push_back(CQuest());
	//Quest.back().name = "No Quest";
	
	int i = 0;

	while (!category.empty())
	{
		
		Quest.push_back(CQuest());
		CQuest& ref = Quest.back();

		ref.serial = Quest.size() - 1;

		ref.Lv = ++i;
		ref.num_bosses = 1;
		ref.boss = monstermap[ns(category)];
		ref.Gil = 400 * (ref.Lv * ref.Lv + 2 * ref.Lv + 1);
		ref.reward = itemmap["None"];
		ref.name = "Defeat " + ref.boss->name;

	/*	ref.Lv = ni(category);
		ref.name = ns(category);
		ref.num_bosses = ni(category);
		ref.boss = monstermap[ns(category)];
		ref.Gil = ni(category);
		ref.reward = itemmap[ns(category)];*/
	}
}

void load_spells(buffer& category)
{
	std::string spellset = "error";

	while (!category.empty())
	{
		if (category.front() == "$")
		{
			category.pop_front();
			spellset = ns(category);
			spellsetmap[spellset] = std::set<CSpell*, sort_serial<CSpell> >();
		}

		if (spellset == "error")
			error("Expected '$' in SPELLS block in " + working_file);

		Spell.push_back(CSpell());
		CSpell& ref = Spell.back();

		ref.Set = spellset;
		ref.serial = Spell.size() - 1;

		ref.Lv = ni(category);

		spellsetmap[spellset].insert(&ref);

		ref.name = ns(category);
		spellmap[ref.name] = &ref;

		ref.JP = ref.Lv * ref.Lv;
		ref.MP = (ref.Lv * ref.Lv + ref.Lv * 3) / 2 + 1;

		ref.Map = (ns(category) == "Y");

		ref.target = targetmap[ns(category)];

		std::string ai_type = ns(category);

		if (ai_type == "ALLY")
			ref.AI = AI_ALLY;
		else if (ai_type == "ENEMY")
			ref.AI = AI_ENEMY;
		else if (ai_type == "KOALLY")
			ref.AI = AI_KOALLY;
		else
			error("Unexpected spell AI type for %s.", ref.name.c_str());

		ref.effect.push_back(buffer());

#ifdef _DEBUG
		std::string spell_desc;
#endif

		if (category.front() == "consume")
		{
			category.pop_front();
			ref.consumes = ns(category);
		}

		while (category.front() != "~")
		{
			if (category.front() == "&")
			{
#ifdef _DEBUG
				if (!parse_spell(spell_desc))
					error("Spell %s has incorrectly formed effect.",
						ref.name.c_str());
				spell_desc = "";
#endif
				ref.effect.back().push_back("eoe");
				ref.effect.push_back(buffer());
				category.pop_front();
			}
			ref.effect.back().push_back(ns(category));

#ifdef _DEBUG
			spell_desc += ref.effect.back().back() + "/";
#endif
		}
		ref.effect.back().push_back("eoe");
		category.pop_front();

#ifdef _DEBUG
		if (!parse_spell(spell_desc))
			error("Spell " + ref.name + " has incorrectly formed effect.");
#endif

		ref.desc = ns(category);
	}
}

void CTown::load_towns(buffer& category)
{
	while (!category.empty())
	{
		namelist.push_back(ns(category));
	}
	it = namelist.begin();
}

void load_weapons(buffer& category)
{
	std::string itemset = "error";

	while(!category.empty())
	{
		if (category.front() == "$")
		{
			category.pop_front();
			itemset = ns(category);
			ItemSet.push_back(CItemSet());
			ItemSet.back().serial = ItemSet.size() - 1;
			ItemSet.back().name = itemset;
			ItemSet.back().type = ITEM_WEAPON;
			itemsetmap[itemset] = &ItemSet.back();
		}

		Weapon.push_back(CWeapon());
		CWeapon& ref = Weapon.back();
		ref.type = ITEM_WEAPON;

		ref.Set = &ItemSet.back();

		ref.serial = Weapon.size() - 1;
		ItemSet.back().insert(&ref);

		ref.Lv = ni(category);
		ref.name = ns(category);
		ref.Atk = ni(category);
		ref.Hit = ni(category);
		ref.range = ni(category);
		ref.twoHanded = (ns(category) == "Y");
		ref.price = ni(category);

		while (category.front() != "~")
		{
			char code = nch(category);
			switch(code)
			{
			case '+':
				ref.adds |= statusmap[ns(category)];
				break;

			case 'a':
				ref.Agi = ni(category);
				break;

			case 'A':
				ref.always |= statusmap[ns(category)];
				break;

			case 'B':
				ref.starts |= statusmap[ns(category)];
				break;

			case 'c':
				ref.casts = CSpell::get_spell(ns(category));
				break;

			case 'C':
				ref.cancels |= statusmap[ns(category)];
				break;

			case 'D':
			{
				std::string drainType = ns(category);
				if (drainType == "HP")
					ref.drainHP = true;
				else if (drainType == "MP")
					ref.drainMP = true;
				else
					error("For " + ref.name +
					", expected a drain type of HP or MP.");
			}
				break;

			case 'e':
				ref.Eva = ni(category);
				break;

			case 'E':
				ref.element = element_parse(ns(category));
				break;

			case 'g':
				ref.grantsA = CAbility::get_ability(ns(category));
				break;

			case 'G':
				ref.grantsS = CSpell::get_spell(ns(category));
				break;

			case 'H':
				ref.hurts = races_parse(ns(category));
				break;

			case 'i':
				ref.Int = ni(category);
				break;

			case 'l':
				ref.Luc = ni(category);
				break;

			case 'm':
				ref.MEv = ni(category);
				break;

			case 'R':
				ref.resists = element_parse(ns(category));
				break;
				
			case 's':
				ref.Str = ni(category);
				break;

			case 'S':
				ref.strengthens = element_parse(ns(category));
				break;
				
			case 'v':
				ref.Vit = ni(category);
				break;

			case 'w':
				ref.weight = ni(category);
				break;

			default:
				error("Didn't expect weapon code %c for %s", code,
					ref.name.c_str());
			}
		}

		if (strip_asterisk(ref.name))
			ref.complex = true;
		else
			ref.complex = false;

		itemmap[ref.name] = &ref;
		if (ref.name == "None")
			noneItem[ref.type] = &ref;

		category.pop_front();
	}
}

bool get_next_category(buffer& token, buffer& category)
{
	if (token.empty()) return false;

	while (token.front() != "{")
	{
		category.push_back(token.front());
		token.pop_front();
		if (token.empty())
			error("Did not find opening '{' in " + working_file + "!");
	}

	// pop the opening '{'.
	token.pop_front();

	do
	{
		if (token.empty())
			error("Expected '}' at end of data file " + working_file + "!");
		category.push_back(token.front());
		token.pop_front();
	} while (category.back() != "}");

	// pop the closing '}'.
	category.pop_back();

	return true;
}

void create_buffer(buffer& buf, std::string filename, int buffer_width)
{
	std::string tmp;
	buffer tmpbuf;
	working_file = filename;
	std::ifstream file(filename.c_str(), std::ios::in);

	if (!file.is_open()) error("Cannot open file " + working_file + "!");

	do
	{
		getline(file, tmp);
		format_string(tmpbuf, tmp, buffer_width);
		buf.splice(buf.end(), tmpbuf);
	} while (!file.eof());

	buf.reverse();
}

void load_data(std::string datafile)
{
	working_file = datafile;
	buffer token;
	buffer category;
	tokenize(datafile, token);

	while (get_next_category(token, category))
	{
		std::string type = category.front();
		category.pop_front();

		if (type == "ABILITY")
			load_abilities(category);

		else if (type == "ARMOR")
			load_armor(category);

		else if (type == "BUFFER")
		{
			std::string type = category.front();
			category.pop_front();
			if (type == "HELP")
				create_buffer(help_buffer, "Data/" + category.front() + ".txt",
					HELP_WIDTH - 2);
			else
				error("Unknown buffer type: " + type);
			working_file = datafile;
		}

		else if (type == "CONFIG")
		{
			while (!category.empty())
			{
				std::string option = ns(category);
				if (option == "Ironman")
					config_flags ^= CONFIG_IRONMAN;
				else if (option == "Chaos")
					config_flags ^= CONFIG_CHAOS;
				else if (option == "Luck")
					config_flags ^= CONFIG_LUCK;
			}
		}

		else if (type == "DOMAINS")
			load_domains(category);

		else if (type == "ELEMENT")
		{
			elementmap["neutral"] = ELEM_NONE;
			for (unsigned char i = 1; !category.empty(); i <<= 1)
			{
				std::string element = ns(category);
				Element.insert(element);
				elementmap[element] = i;
			}
		}
		
		else if (type == "FONT")
		{
			while (!category.empty())
			{
				std::string ff = ns(category) + ".png";
				std::string fo = ns(category);
				if (font_file == "terminal.png")
				{
					font_file = ff;
					font_orient = fo;
				}
			}
		}

		else if (type == "INCLUDE")
		{
			for(std::string it : category)
				load_data("Data/" + it + ".txt");
			working_file = datafile;
		}

		else if (type == "ITEMS")
			load_items(category);

		else if (type == "JOBS")
			load_jobs(category);

		else if (type == "MONSTERS")
			load_monsters(category, false);

		else if (type == "BOSSES")
			load_monsters(category, true);

		else if (type == "QUESTS")
			load_quests(category);

		else if (type == "RACE")
			for (unsigned char i = 1; !category.empty(); i <<= 1)
			{
				Race.push_back(ns(category));
				racemap[Race.back()] = i;
			}

		else if (type == "SPELLS")
			load_spells(category);

		else if (type == "STATUS")
			for (unsigned long i = 1; !category.empty(); i <<= 1)
			{
				std::string s = ns(category);
				Status.insert(s);
				statusmap[s] = i;
			}

		else if (type == "TARGET_TYPE")
			for (int i = 0; !category.empty(); i++)
				targetmap[ns(category)] = (target_type)i;

		else if (type == "TOWN")
			CTown::load_towns(category);

		else if (type == "WEAPONS")
			load_weapons(category);

		else
			error("Unknown data category type: " + type);

		category.clear();
	}
}

