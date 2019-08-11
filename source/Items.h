#ifndef __ITEMS_H_
#define __ITEMS_H_

#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Misc.h"
#include "Output.h"
#include "Player.h"
#include "Window.h"

enum item_mode_type
{
	ITEM_MODE_SELECT, 
	ITEM_MODE_BUY,		// Invoked when using CShopWindow
						// to buy armor and weapons.
	ITEM_MODE_SELL,
	ITEM_MODE_USE,
	ITEM_MODE_EQUIP
};

class CAbility;
class CBattle;
class CItemSet;
class CSpell;

class CItem
{
public:
	CItemSet* Set;

	// complex items have special state information saved with them.
	bool complex;

	int serial;
	item_type type;

	std::string name;
	std::string description;

	int Lv, price;

	static CItem* get_item(std::string item_name);
	virtual void describe(_wintype*, int y = 1);
	virtual ~CItem() {}
};

class IEquip
{
public:
	int HP, MP, weight;
	char Int, Str, Agi, Vit, Luc;
	char Atk, Hit, Eva, MEv;

	unsigned long adds;
	unsigned long always;
	unsigned long cancels;
	unsigned long starts;
	unsigned char element;
	unsigned char hurts;
	unsigned char immune;
	unsigned char resists;
	unsigned char strengthens;

	CAbility *grantsA, *seals;
	CSpell* grantsS;

	IEquip(): HP(0), MP(0), weight(0), Int(0), Str(0), Agi(0),
		Vit(0), Luc(0), Atk(0), Hit(0), Eva(0), MEv(0),
		adds(ST_NONE), always(ST_NONE), cancels(ST_NONE),
		element(ELEM_NONE), hurts(0), immune(ELEM_NONE),
		resists(ELEM_NONE), starts(ELEM_NONE), seals(NULL),
		strengthens(ELEM_NONE), grantsA(NULL), grantsS(NULL)
	{}

	IEquip(const CPlayer* player)
	{
		HP = player->MaxHP;
		MP = player->MaxMP;
		Int = player->Int;
		Str = player->Str;
		Agi = player->Agi;
		Vit = player->Vit;
		Luc = player->Luc;
		Atk = player->Atk;
		Hit = player->Hit;
		Eva = player->Eva;
		weight = 0;
	}

	const IEquip operator+(const IEquip& rhs) const
	{
		IEquip equip(*this);
		
		equip.HP += rhs.HP;
		equip.MP += rhs.MP;
		equip.Int += rhs.Int;
		equip.Str += rhs.Str;
		equip.Agi += rhs.Agi;
		equip.Vit += rhs.Vit;
		equip.Luc += rhs.Luc;
		equip.Atk += rhs.Atk;
		equip.Hit += rhs.Hit;
		equip.Eva += rhs.Eva;
		equip.weight += rhs.weight;

		equip.adds |= rhs.adds;
		equip.always |= rhs.always;
		equip.cancels |= rhs.cancels;
		equip.element |= rhs.element;
		equip.hurts |= rhs.hurts;
		equip.immune |= rhs.immune;
		equip.resists |= rhs.resists;
		equip.starts |= rhs.starts;
		equip.strengthens |= rhs.strengthens;
		if (rhs.seals)
			equip.seals = rhs.seals;

		return equip;
	}
	
	// Add the passive abilities of the rhs item.
	const IEquip operator<(const IEquip& rhs) const
	{
		IEquip equip(*this);

		equip.HP += rhs.HP;
		equip.MP += rhs.MP;
		equip.Int += rhs.Int;
		equip.Str += rhs.Str;
		equip.Agi += rhs.Agi;
		equip.Vit += rhs.Vit;
		equip.Luc += rhs.Luc;
		equip.Eva += rhs.Eva;
		equip.weight += rhs.weight;

		equip.always |= rhs.always;
		equip.starts |= rhs.starts;
		equip.cancels |= rhs.cancels;
		equip.strengthens |= rhs.strengthens;
		if (rhs.seals)
			equip.seals = rhs.seals;

		return equip;
	}

	// Add the active abilities of the rhs item.
	const IEquip operator<<(const IEquip& rhs) const
	{
		IEquip equip(*this);

		equip.Atk += rhs.Atk;
		equip.Hit += rhs.Hit;

		equip.adds |= rhs.adds;
		equip.element |= rhs.element;
		equip.hurts |= rhs.hurts;

		return equip;
	}
};

class CItemSet : public std::set<CItem*, sort_serial<CItem> >
{
public:
	int serial;
	std::string name;
	item_type type;
};

class CItemWindow : public CSelectWindow
{
public:
	item_mode_type mode;
	item_type type;
	std::vector<std::vector<CItem*> > Item;

	// if shared_menu is true, all players select from the same
	// buffer (used by CShopWindow).
	CItemWindow(_wintype*, item_type, item_mode_type m = ITEM_MODE_BUY,
				 bool party_view = true, bool shared_menu = false);

	// return value:  Whether any of the menus built were nonempty.
	virtual bool build_menus(bool include_equipped = false);
	void describe(_wintype*);
	void empty_menu(_wintype*);
	void select();
};

class CEquipWindow : public CItemWindow
{
public:
	bool equipped_something;

	CEquipWindow(_wintype*, item_type, item_mode_type m,
		bool party_view = true);
	void supermenu();
	void describe(_wintype*);
};

class CShopWindow : public CItemWindow
{
public:
	std::vector<CItemSet*> SuperMenu;

	CShopWindow(_wintype*, item_type);
	bool build_menus();
	void supermenu();
};

void item_menu(item_mode_type);

extern std::deque<CItemSet> ItemSet;
extern std::map<std::string, CItem*> itemmap;
extern std::map<std::string, CItemSet*> itemsetmap;
extern std::map<int, CItem*> noneItem;
extern std::deque<CItem> Item;
extern std::vector<CItem> ComplexItem;

CItemSet* random_set();

#endif
