#ifndef __COMBAT_H_
#define __COMBAT_H_

#include <vector>
#include <list>
#include <string>

#include "Misc.h"
#include "Monsters.h"

class CActor;
class CDeal;
class CMonster;
class CSpell;
class CTarget;

// Must be long, and not unsigned long, lest the overloaded
// CActor operators be confused with those for status.
const long CF_NONE =			0x00000000;
const long CF_TARGETABLE =		0x00000001;
const long CF_BACK_ROW =		0x00000002;
const long CF_ON_PLAYER_SIDE =	0x00000004;
const long CF_DEFENDING =		0x00000008;
const long CF_FLED =			0x00000010;
const long CF_AIMING =			0x00004000;
const long CF_CHARGING =		0x00008000;
const long CF_JUMPING =			0x00010000;

const long LOSE_WHEN_DEFEATED_CF =
	CF_AIMING | CF_CHARGING | CF_JUMPING | CF_DEFENDING;

const long START_CF_PLAYER =
	CF_ON_PLAYER_SIDE | CF_TARGETABLE;

const long START_CF_MONSTER =
	CF_TARGETABLE;

enum combat_act_type
{
	SEL_TOGGLE_CV = -1,
	SEL_ITEM,
	SEL_RUN,
	SEL_ROW,
	SEL_ATTACK,
	SEL_DEFEND,
	SEL_EQUIP,
	SEL_ABIL1,
	SEL_ABIL2,
	SEL_ABIL3,
};

class CBattle
{
public:
	std::list<CMonster*> Enemy;
	std::vector<CActor*> Actor;
	bool fighting_boss;
	bool the_battle_rages;

	CBattle(bool fighting_boss);
	~CBattle();

	void add_actors();
	bool cast_spell(CPlayer&, CSpell*);
	void conduct();
	void draw_monsters(CTarget& cursor, CTarget* Target = NULL);
	void draw_monsters(CActor*);
	void draw_monsters();
	void draw_BattleQ();
	void escape(CMonster*);
	void form_BattleQ();
	void draw();
	void die(CActor&);
	int heal(CActor&, int total, bool HPrecovery = true);
	void loseHP(CActor& target, int total);
	int damage(CActor* source, CActor& target, int base_damage,
		unsigned char element, int multiplier, std::string,
		bool mult_includes_base = false, bool drainHP = false,
		bool drainMP = false);
	bool lose_status(CActor&, unsigned long status);
	void summon(CMonsterRace*, int number);
	
	// Return value:  Was any message printed?
	bool gain_status(CActor&, unsigned long status);

private:
	std::list<CActor*> BattleQ;
	std::list<CDeal> QuickList;

	bool party_died, quick_turn;
	
	void begin_round();
	void control_monster(CActor&);
	void end_round();
	void monster_combat(CActor&);
	bool one_side_dead();
	void player_combat(CActor&);
	void take_turn(CActor&);
	bool use_ability(CAbility&, CActor&);
};

void the_boss_attacks();

void monsters_attack();

#endif
