#ifndef __ARMOR_H_
#define __ARMOR_H_

#include <map>
#include <deque>

#include "Items.h"
#include "Misc.h"

class CArmor: public CItem, public IEquip
{
public:
	void describe(_wintype*, int y);
};

extern std::deque<CArmor> Armor;

#endif
