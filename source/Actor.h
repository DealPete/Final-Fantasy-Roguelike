#ifndef __ACTOR_H_
#define __ACTOR_H_

#include <string>

#include "Misc.h"

class CDeal;
class CMonsterRace;

enum sex_type
{
	SEX_NONE,
	SEX_MALE,
	SEX_FEMALE
};

// An "Actor" is any entity that can be involved in combat.
// This is the parent class of both CMonster and CPlayer.
class CActor
{
public:
	std::string name;
	sex_type sex;	

	CDeal* init;
	unsigned long status;

	// These variables keep track of combat state.
	long cflag;
	unsigned char calcify_counters, time_counters, blink_num;
	unsigned char focus;

	CActor* control;
	
	// For sorting them in combat.
	int serial;

	// These Stats are common to both players and monsters.
	int Vit, Agi, Int;

	// Derived Stats.
	int Atk, Hit, Eva;

	CActor() {
		cflag = 0;
	}

	// effective stats, which may 0 due to status.
	int eVit(), eAgi(), eInt(), eAtk(), eHit(), eEva();

	unsigned char hurts, element;
	unsigned char weak, resists, immune, strengthens, absorbs;
	unsigned long adds, always, cancels, starts;

	bool defeated();
	
	unsigned long operator+=(unsigned long flag);
	long operator+=(long flag);
	unsigned long operator-=(unsigned long flag);
	long operator-=(long flag);

	// returns true if every bit of flag matches actor.
	bool operator==(unsigned long flag);
	bool operator==(long flag);

	// returns true if no bit of flag matches actor.
	bool operator!=(unsigned long flag);
	bool operator!=(long flag);

	unsigned long operator^=(unsigned long flag);
	long operator^=(long flag);
	unsigned long operator&=(unsigned long flag);
	long operator&=(long flag);
	bool operator&(unsigned long flag);
	bool operator&(long flag);

	virtual	std::string action(int action) = 0;
	virtual int curHP() = 0;
	virtual int gainHP(int) = 0;
	virtual int gainMP(int) = 0;
	virtual bool gain_stat(std::string stat, int bonus) = 0;
	virtual bool has_ability(std::string) = 0;
	virtual void heal_to_full() = 0;
	virtual bool is_critical() = 0;
	virtual bool is_player() = 0;
	virtual bool loseHP(int) = 0;

	// Return value:  The amount of MP actually lost.
	virtual int loseMP(int) = 0;
	virtual int Lv() = 0;
	virtual CMonsterRace* race() = 0;
	virtual void setHP(int) = 0;

	std::string objp();
	std::string subp();
	std::string posp();
	std::string refp();
	virtual ~CActor()
	{}
};

#endif
