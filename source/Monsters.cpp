#include <map>
#include <deque>

#include "Combat.h"
#include "Message.h"
#include "Monsters.h"
#include "Random.h"

CMonster::CMonster(CMonsterRace* EnemyRace, unsigned long flags)
{
	serial = mon_serial++;
	monster_race = EnemyRace;
	name = monster_race->name;
	
	Vit = monster_race->Vit;
	Agi = monster_race->Agi;
	Eva = monster_race->Eva;
	Hit = monster_race->Hit;
	Atk = monster_race->Atk;
	Int = monster_race->Int;

	sex = monster_race->sex;

	immune = monster_race->immune;
	resists = monster_race->resists;
	weak = monster_race->weak;

	absorbs = monster_race->absorbs;
	adds = monster_race->adds;
	always = monster_race->always;
	starts = monster_race->starts;

	hurts = RACE_NONE;
	element = strengthens = ELEM_NONE;

	if (monster_race->boss)
		cancels = ST_BAD;
	else
		cancels = ST_NONE;

	status = always | starts;

	has_item = true;
	form = monster_race->starting_form;

	cflag = START_CF_MONSTER | flags;
	focus = 0;

	for (int i = 0; i < Vit; i++)
		HPCards.push_back(CCard(true));
}

std::string CMonster::action(int action)
{
	return form->Action[action];
}

void CMonster::change_form(std::string change_to)
{
	std::string changeStr = change_to;

	if (change_to == "" && monster_race->name == "Chaos")
	{
		do
		{
			switch(random(4))
			{
			case 0:
				changeStr = "Earth";
				break;

			case 1:
				changeStr = "Air";
				break;

			case 2:
				changeStr = "Fire";
				break;

			case 3:
				changeStr = "Water";
				break;
			}
		} while (form->name == changeStr);
	}

	changeStr = change_to == "" ? form->changes_to : change_to;

	CMonsterForm* changes_to = monster_race->formMap[changeStr];

	if (form != changes_to)
	{
		add_message("%s changes to %s form.", name.c_str(),
			changes_to->name.c_str());
		form = changes_to;
	}
}

int CMonster::curHP()
{
	int total = 0;

	for(CCard& it : HPCards)
		total += it.rank + 1;

	return total;
}

// return value:  The amount of HP gained.
int CMonster::gainHP(int gain)
{
	int recover = 12 - HPCards.back().rank;
	HPCards.back().face_down = false;

	if (recover < gain)
	{
		HPCards.back().rank = 12;

		if (HPCards.size() == eVit())
			return recover;

		HPCards.push_back(CCard(0));
		return recover + gainHP(gain - recover - 1);
	}

	HPCards.back().rank += gain;
	return gain;
}

int CMonster::gainMP(int gain)
{
	return 0;
}

bool CMonster::has_ability(std::string ability)
{
	return false;
}

void CMonster::heal_to_full()
{
	while((int)HPCards.size() < eVit())
		HPCards.push_back(CCard(true));
}

bool CMonster::is_critical()
{
	if ((int)HPCards.size() <= dv(eVit(), 4))
		return true;

	return false;
}

bool CMonster::is_player()
{
	return false;
}

// return value:  Is the monster still alive?
bool CMonster::loseHP(int loss)
{
	if (HPCards.size() == 0)
	{
		return false;
	}

	if (loss == 0)
		return true;

	if (loss <= HPCards.back().rank)
	{
		HPCards.back().rank -= loss;
		HPCards.back().face_down = false;
	}
	else
	{
		loss -= HPCards.back().rank + 1;
		HPCards.pop_back();
		if (!loseHP(loss))
			return false;
	}

	return true;
}

int CMonster::loseMP(int loss)
{
	return 0;
}

bool CMonster::gain_stat(std::string stat, int bonus)
{
	if (stat == "Agi")
		Agi += bonus;
	else if (stat == "Vit")
		Vit += bonus;
	else if (stat == "Hit")
		Hit += bonus;
	else if (stat == "Eva")
		Eva += bonus;
	else if (stat == "Atk")
		Atk += bonus;
	else if (stat == "Int")
		Int += bonus;
	else
		return false;

	if (stat == "Agi")
		if (bonus > 0)
			init->add_cards(bonus);
		else if (bonus < 0 && init->size() > 0)
			init->cut_highest(-bonus);
	
	if (stat == "Vit")
		if ((int)HPCards.size() > Vit)
			if (Vit > 0)
				while ((int)HPCards.size() > Vit)
					HPCards.pop_back();
			else
			{
				CCard lastcard(HPCards.front());
				lastcard.face_down = false;
				HPCards.clear();
				for (int i = 0; i < (1 - Vit); i++)
				{
					CCard tmpcard;
					if (tmpcard < lastcard)
						lastcard = tmpcard;
				}
				HPCards.push_back(lastcard);
			}

	return true;
}

int CMonster::Lv()
{
	return monster_race->Lv;
}

CMonsterRace* CMonster::race()
{
	return monster_race;
}

// This function looks like it needs work.
void CMonster::setHP(int value)
{
	HPCards.clear();
	if (value > 0)
		HPCards.push_back(CCard(value - 1));
}

std::deque<CMonsterRace> MonsterRace;
std::map<std::string, CMonsterRace*> monstermap;

int CMonster::mon_serial = 100;