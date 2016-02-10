#ifndef __ACTION_H_
#define __ACTION_H_

#include "Misc.h"

#include <string>

class CActor;
class CBattle;
class CSpell;
class CTarget;
class CWeapon;

enum action_result_type
{
	AR_NONE,
	AR_NORMAL,
	AR_COND_FAIL,
	AR_FLED,
	AR_ERROR
};

class CActionFunctor
{
public:
	CActor& actor;

	CActionFunctor(CActor& a) : actor(a)
	{}

	CActionFunctor(CActor& a, buffer e) : actor(a), effect(e)
	{}

	virtual ~CActionFunctor() {}

protected:
	CActor* target;
	buffer effect;
	std::string token;

	// To scan through an action's script, set effect
	// equal to the buffer containing the script, then
	// call next() repeatedly to iterate through it.
	std::string next();
};

class CActionAttack : public CActionFunctor
{
public:
	CActionAttack(CActor& a, buffer e);

	action_result_type operator()(CTarget& Target);

private:
	action_result_type attack(CActor& target);
	void parse();

	std::string attack_msg;
	unsigned char attack_hurts, attack_element;
	unsigned long attack_adds;
	int adjusted_Hit;
	int adjusted_Atk;
	int adjusted_Eva;
	bool ignore_evasion, ignore_row, jumping, throwing;
	bool attack_drainHP, attack_drainMP, meatbone_slashing;
	int modifier, num_attacks;
	CWeapon* attack_weapon;
};

class CActionDefend : public CActionFunctor
{
public:
	CActionDefend(CActor& a) :
	  CActionFunctor(a) {}

	action_result_type operator()();
};

class CActionFlee : public CActionFunctor
{
public:
	CActionFlee(CActor& a) :
	  CActionFunctor(a) {}

	action_result_type operator()();

};

class CActionChangeRow : public CActionFunctor
{
public:
	CActionChangeRow(CActor& a) :
	  CActionFunctor(a) {}

	action_result_type operator()();
};

class CActionEquip : public CActionFunctor
{
public:
	CActionEquip(CActor& a) :
	  CActionFunctor(a) {}

	action_result_type operator()();
};

class CActionItem : public CActionFunctor
{
public:

	CActionItem(CActor& a) :
	  CActionFunctor(a) {}
	
	action_result_type operator()();
};

class CActionCastSpell : public CActionFunctor
{
public:
	CActionCastSpell(CActor& a, CSpell* s) :
	  CActionFunctor(a), spell(s) {}

	action_result_type operator()(CTarget& Target);

	virtual action_result_type special(CTarget&) = 0;
	virtual action_result_type add() = 0;
	virtual action_result_type remove() = 0;
	virtual action_result_type toggle() = 0;

protected:
	std::map<CActor*, CActor*> reflectmap;
	action_result_type final_result;
	int total_damage;
	CSpell* spell;
	bool apply_to_caster;

	// Is called by both exit()'s.
	void warp_outside();

	// buffer contains the spell effect. Before calling
	// a subroutine that needs to scan the effect, it
	// sets CActionFunction::effect equal to buffer.
	action_result_type parse(buffer, CTarget);
	action_result_type party();
//	void search_for_endif();
		
	action_result_type condition(CActor*);
	action_result_type damage(CTarget);
	action_result_type drain(CTarget);
	action_result_type escape();
	action_result_type gainMP(CTarget);
	action_result_type heal(CTarget);
	action_result_type loseHP();
	action_result_type stat();

	// return value: The targets which passed the check.
	CTarget chance(CTarget);
};

class CActionCastSpellBattle : public CActionCastSpell
{
public:
	CActionCastSpellBattle(CActor& a, CSpell* s) :
	  CActionCastSpell(a, s) {}

	action_result_type add();
	action_result_type remove();
	action_result_type special(CTarget&);
	action_result_type toggle();
};

class CCastSpellMap : public CActionCastSpell
{
public:
	CCastSpellMap(CActor& a, CSpell* s) :
	  CActionCastSpell(a, s) {}
	
	action_result_type add();
	action_result_type remove();
	action_result_type special(CTarget&);
	action_result_type toggle();
};

class CActionCommand : public CActionFunctor
{
public:
	CActionCommand(CActor& a, CAbility* ab) :
	  CActionFunctor(a), ability(ab) {}

	bool operator()(CTarget&);

private:
	CAbility* ability;
};

#endif
