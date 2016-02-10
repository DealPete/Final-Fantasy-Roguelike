#ifndef __PLAYER_H_
#define __PLAYER_H_

#include "Actor.h"
#include "Misc.h"

#include <list>
#include <map>
#include <set>
#include <string>

class CBattle;
class CItem;
class CItemSet;
class CJob;
class CSpell;
class CMonsterRace;
class CWeapon;

class CPlayer: public CActor
{
public:
	int Level, JP, XP, TNL;
	std::list<CItem*> Inventory;

	std::set<CSpell*> bSpell;
	std::set<CSpell*> Spell;

	std::set<CAbility*, sort_serial<CAbility> > bAbil;
	std::set<CAbility*, sort_serial<CAbility> > Abil;
	std::set<CItemSet*> WeaponSkill;
	std::set<CItemSet*> ArmorSkill;

	bool backRow;
	int weight;

	CJob* job;

	CAbility* seals;

	std::map<int, CItem*> Equip;

	// These Stats are unique to the players.
	int MEv, Str, Luc, HP, MaxHP, MP, MaxMP;

	// Only players have Base Stats.
	int bStr, bAgi, bVit, bInt, bLuc, bonusMaxHP, bonusMaxMP;

	// Temporary boost to stats during battle.
	int tStr, tAgi, tVit, tInt, tLuc, tAtk, tHit, tEva, tMP;

	// Temporary boost to resistances.
	unsigned char tresists, timmune;

	int battle_select_cursor;
	std::set<CSpell*> BlueMemory;

	CPlayer()
	{
	}

	// both constructors call cal_stats() to determine initial values.
	CPlayer(int j, std::string n);
	CPlayer(std::ifstream& savefile);

	void cal_stats(bool side_effects = true, CWeapon* alternate_weapon = NULL,
		item_type main_weapon =	ITEM_WEAPON);
	
	std::string action(int action);
	int burden();
	int curHP();

	// return value:  HP gained.
	int gainHP(int);
	
	// return value:  Is the player still alive?
	bool loseHP(int);

	int gainMP(int);
	bool gain_stat(std::string stat, int bonus);
	bool has_ability(std::string);
	void heal_to_full();
	bool is_critical();
	bool is_player();

	// Return value:  The amount of MP actually lost.
	int loseMP(int);
	int Lv();
	CMonsterRace* race();
	void setHP(int);

	// return value:  Which equip slot was it removed from?
	// ITEM_NONE means from inventory.
	item_type remove_item(CItem*);

	bool has_equipped(CItem*);
	bool equip(CItem*, item_type slot);
	bool use(CItem*, _wintype* window = NULL);
	void level_up();

	void print_stats(_wintype*, CItem* equip = NULL,
		item_type slot = ITEM_NONE, int y = 1);
	void out(std::ofstream& savefile);

private:
	void recoverHP(int);
	void put_on(CItem*);
	void take_off(CItem*);
};

#endif
