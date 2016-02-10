#ifndef __MONSTERS_H_
#define __MONSTERS_H_

#include <bitset>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "Actor.h"
#include "Misc.h"

class CCard;
class CItem;
class CSpell;

class CMonsterForm
{
public:
	std::string name;
	std::map<int, std::string> Action;
	std::string changes_to;

	CMonsterForm(std::string name): name(name), changes_to("Default")
	{
		Action[SUIT_SPADE] = Action[SUIT_CLUB] = Action[SUIT_DIAMOND] =
			Action[SUIT_HEART] = "Attack";
	}
};

class CMonsterRace
{
public:
	std::string name;
	std::string plural;
	std::string greeting;
	sex_type sex;
	int Lv, Vit, Agi, Eva, Hit, Atk, Int;
	std::map<std::string, CMonsterForm*> formMap;
	CMonsterForm* starting_form;
	CItem* rare_item;
	CItem* common_item;
	unsigned char races, weak, resists, immune, absorbs;
	int adds, always, starts;
	std::string starts_by_casting;
	bool boss;

	CMonsterRace(): name(""), plural(""), greeting(""), starts_by_casting(""),
		races(RACE_NONE), weak(ELEM_NONE), resists(ELEM_NONE),
		immune(ELEM_NONE), absorbs(ELEM_NONE), starting_form(NULL),
		adds(ST_NONE), always(ST_NONE),	starts(ST_NONE)
	{}
};

class CMonster: public CActor
{
private:
	static int mon_serial;
	CMonsterRace* monster_race;
	
public:
	bool has_item;
	CMonsterForm* form;
	std::vector<CCard> HPCards;

	CMonster(CMonsterRace* EnemyRace, unsigned long flags);

	void change_form(std::string = "");
	std::string action(int action);
	int curHP();
	int gainHP(int);
	int gainMP(int);
	bool gain_stat(std::string stat, int bonus);
	bool has_ability(std::string);
	void heal_to_full();
	bool is_critical();
	bool is_player();
	bool loseHP(int);
	int loseMP(int);
	int Lv();
	CMonsterRace* race();
	void setHP(int);
};

extern std::deque<CMonsterRace> MonsterRace;
extern std::map<std::string, CMonsterRace*> monstermap;

#endif
