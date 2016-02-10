#ifndef __WEAPONS_H_
#define __WEAPONS_H_

#include <map>
#include <deque>

#include "Items.h"

class CAbility;
class CSpell;

class CWeapon: public CItem, public IEquip
{
public:
	void describe(_wintype*, int y);

	char range;
	bool twoHanded;
	bool drainHP;
	bool drainMP;

	CSpell* casts;

	CWeapon(): casts(NULL), drainHP(false), drainMP(false)
	{}
};

extern std::deque<CWeapon> Weapon;

#endif
