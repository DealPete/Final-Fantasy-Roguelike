#include <string>

#include "Armor.h"
#include "Combat.h"
#include "Input.h"
#include "Items.h"
#include "Jobs.h"
#include "Message.h"
#include "Misc.h"
#include "Monsters.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Random.h"
#include "Spells.h"
#include "Weapons.h"

CPlayer::CPlayer(int j, std::string n)
{
	serial = Party.Player.size() - 1;
	job = &Job[j];
	name = n;
	sex = SEX_MALE;

	Level = 1;

	bStr = job->Str;
	bAgi = job->Agi;
	bVit = job->Vit;
	bInt = job->Int;
	bLuc = job->Luc;
	bonusMaxHP = 0;
	bonusMaxMP = 0;

	tStr = tAgi = tVit = tInt = tLuc = tAtk = tHit = tEva = tMP = 0;
	tresists = timmune = ELEM_NONE;

	for(CAbility* it : job->Ability)
		bAbil.insert(it);

	for(std::string it : job->WeaponSkill)
	{
		if (!itemsetmap[it])
			error("ERROR: No item set " + it);
	
		WeaponSkill.insert(itemsetmap[it]);
	}

	for(std::string it : job->ArmorSkill)
	{
		if (!itemsetmap[it])
			error("ERROR: No item set " + it);

		ArmorSkill.insert(itemsetmap[it]);
	}

	JP = 4;
	XP = 0;
	TNL = 40;

	for (int type = ITEM_WEAPON; type <= ITEM_HEAD; type++)
		Equip[type] = noneItem[type];
	
	backRow = false;

	cal_stats();

	HP = MaxHP;
	MP = MaxMP;

	status = always | starts;

	battle_select_cursor = 3;
}

void CPlayer::cal_stats(bool side_effects, CWeapon* alternate_weapon,
						item_type main_weapon)
{
	IEquip equip;

	Abil = bAbil;
	Spell = bSpell;

	for(int i = ITEM_WEAPON; i <= ITEM_HEAD; i++)
	{
		IEquip& ref = dynamic_cast<IEquip&>(*Equip[i]);

		if (Equip[i]->type == ITEM_WEAPON)
			equip = equip < ref;
		else
			equip = equip + ref;

		if (ref.grantsA)
			Abil.insert(ref.grantsA);
		if (ref.grantsS)
			Spell.insert(ref.grantsS);
	}

	if (alternate_weapon)
		equip = equip + dynamic_cast<IEquip&>(*alternate_weapon);
	else
		equip = equip << dynamic_cast<IEquip&>(*Equip[main_weapon]);

	Str = bStr + tStr + equip.Str;
	Agi = bAgi + tAgi + equip.Agi;
	Vit = bVit + tVit + equip.Vit;
	Int = bInt + tInt + equip.Int;
	Luc = bLuc + tLuc + equip.Luc;
	weight = equip.weight;

	MaxHP = bonusMaxHP + equip.HP + (Level + 1) * (Vit + 5);
	MaxMP = bonusMaxMP + equip.MP + (Level + 1) * (Int + 5);

	if (Equip[main_weapon]->Set->name == "Gun")
		Atk = tAtk + equip.Atk;
	else
		Atk = dv(Str, 2) + tAtk + equip.Atk;

	Hit = dv(Str, 6) + tHit + equip.Hit;
	Eva = dv(Luc - 1, 3) + tEva + equip.Eva;

	// final Str must be known by here.
	if (burden() + equip.weight > Str)
		Agi -= burden() + equip.weight - Str;

	if (Abil.count(abilitymap["Martial Arts"]))
	{
		if (Equip[ITEM_HEAD]->name == "None" &&
			Equip[ITEM_BODY]->name == "None")
		{
			MaxHP += 8 * Level;
			MaxMP += 8 * Level;
		}

		if (Equip[ITEM_WEAPON]->name == "None" &&
			Equip[ITEM_ACCESSORY]->name == "None")
		{
			Atk += Level * 2;
			Hit += dv(Level, 2);
		}
	}
	
	if (Abil.count(abilitymap["Double Grip"]) &&
		Equip[ITEM_ACCESSORY]->name == "None"
		&& ((CWeapon*)Equip[ITEM_WEAPON])->twoHanded == false
		&& Equip[ITEM_WEAPON]->name != "None")
		Hit++;

	MEv = dv(Int + 2, 3) + equip.MEv;

	hurts = equip.hurts;
	element = equip.element;
	weak = ELEM_NONE;
	resists = equip.resists | tresists;
	immune = equip.immune | timmune;
	absorbs = ELEM_NONE;
	strengthens = equip.strengthens;
	adds = equip.adds;
	always = equip.always;
	cancels = equip.cancels;
	starts = equip.starts;
	seals = equip.seals;
	
	if (side_effects)
	{
		if (MaxHP < 1)
			MaxHP = 1;
		if (MaxMP < 1)
			MaxMP = 1;
		if (HP > MaxHP)
			HP = MaxHP;
		if (MP > MaxMP)
			MP = MaxMP;
	}
}

std::string CPlayer::action(int action)
{
	return "Attack";
}

int CPlayer::curHP()
{
	return HP;
}

int CPlayer::gainHP(int gain)
{
	if (HP + gain > MaxHP)
		gain = MaxHP - HP;

	HP += gain;
	return gain;
}

bool CPlayer::gain_stat(std::string stat, int bonus)
{
	if (stat == "Agi")
		tAgi += bonus;
	else if (stat == "Vit")
		tVit += bonus;
	else if (stat == "Hit")
		tHit += bonus;
	else if (stat == "Eva")
		tEva += bonus;
	else if (stat == "Atk")
		tAtk += bonus;
	else if (stat == "Int")
		tInt += bonus;
	else
		return false;

	cal_stats();

	if (stat == "Agi") {
		if (bonus > 0)
			init->add_cards(bonus);
		else if (bonus < 0)
			init->cut_highest(-bonus);
	}

	return true;
}

int CPlayer::gainMP(int gain)
{
	if (MP + gain > MaxMP)
		gain = MaxMP - MP;

	MP += gain;
	return gain;
}

bool CPlayer::has_ability(std::string ability)
{
	return Abil.count(abilitymap[ability]) != 0;
}

void CPlayer::heal_to_full()
{
	HP = MaxHP;
}

bool CPlayer::is_critical()
{
	if (curHP() <= dv(MaxHP, 4))
		return true;

	return false;
}

bool CPlayer::is_player()
{
	return true;
}

// Return value:  Is the player still alive?
bool CPlayer::loseHP(int loss)
{
	HP -= loss;
	if (HP <= 0)
		HP = 0;

	if (HP == 0)
		return false;

	return true;
}

// Return value:  The amount of MP actually lost.
int CPlayer::loseMP(int loss)
{
	MP -= loss;
	if (MP < 0)
	{
		loss += MP;
		MP = 0;
	}

	return loss;
}

int CPlayer::Lv()
{
	return Level;
}

CMonsterRace* CPlayer::race()
{
	return monstermap["None"];
}

void CPlayer::setHP(int value)
{
	HP = value;
}

int CPlayer::burden()
{
	int burden = 0;
	int ammo = 0;

	for(CItem* it : Inventory)
		if (it->Set->name == "Ammunition")
			ammo++;
		else
			burden++;

	if (ammo > Str)
		burden += ammo - Str;

	return burden;
}

item_type CPlayer::remove_item(CItem* item)
{
	std::list<CItem*>::iterator it = Inventory.begin();
	
	while (it != Inventory.end())
	{
		if (*it == item)
		{
			Inventory.erase(it);
			cal_stats();
			return ITEM_NONE;
		}
		it++;
	}

	for(int type = ITEM_WEAPON; type <= ITEM_HEAD; type++)
		if (Equip[type] == item)
		{
			Equip[type] = noneItem[type];
			cal_stats();
			return (item_type)type;
		}

	return ITEM_NONE;
}

bool CPlayer::has_equipped(CItem* item)
{
	for(int i = ITEM_WEAPON; i <= ITEM_HEAD; i++)
		if (item == Equip[i])
			return true;

	return false;
}

void CPlayer::put_on(CItem* item)
{
	if (item->Set->name == "Quest")
		Party.EquippedKeyItem.insert(item);
	else
		remove_item(item);
}

void CPlayer::take_off(CItem* item)
{
	if (item->name != "None") {
		if (item->Set->name == "Quest")
			Party.EquippedKeyItem.erase(item);
		else
			Inventory.push_back(item);
	}
}

bool CPlayer::equip(CItem* item, item_type slot)
{
	if (!slot)
	{
		message("error: must specify equip slot.");
		return false;
	}

	CItem* remove;
	std::string verb;
	
	if (!(WeaponSkill.count(item->Set) || ArmorSkill.count(item->Set)
		|| item->name == "None" || item->Set->name == "Quest"
		|| item->Set->name == "Ammunition"))
	{
		message("%s cannot equip the %s.", name.c_str(),
			item->name.c_str());
		return false;
	}

	if (slot == ITEM_ACCESSORY && ((CWeapon*)Equip[ITEM_WEAPON])->twoHanded
		&& item->name != "None")
	{
		message("%s needs both hands to use the %s.", name.c_str(),
			Equip[ITEM_WEAPON]->name.c_str());
		return false;
	}

	remove = Equip[slot];
	Equip[slot] = item;
	verb = item->type == ITEM_WEAPON ? "readies" : "dons";

	if (item->name == "None")
	{
		if (remove->name == "None")
		{
			message("%s already has nothing equipped there.", name.c_str());
			return false;
		}
		message("%s removes the %s.", name.c_str(), remove->name.c_str());
	}
	else if (remove->name != "None")
	{
		if (slot == ITEM_WEAPON && ((CWeapon*)item)->twoHanded
			&& Equip[ITEM_ACCESSORY]->name != "None")
		{
			message("%s takes off the %s and the %s and %s the %s.",
				name.c_str(), remove->name.c_str(),
				Equip[ITEM_ACCESSORY]->name.c_str(),
				verb.c_str(), item->name.c_str());
			take_off(Equip[ITEM_ACCESSORY]);
			Equip[ITEM_ACCESSORY] = noneItem[ITEM_ACCESSORY];
		}
		else
		{
			message("%s removes the %s and %s %s %s.",
				name.c_str(), remove->name.c_str(), verb.c_str(),
				remove->name == item->name ? "another" : "the",
				item->name.c_str());
		}
	}
	else
		if (item->type == ITEM_WEAPON && ((CWeapon*)item)->twoHanded
			&& Equip[ITEM_ACCESSORY]->name != "None")
		{
			message("%s takes off the %s and %s the %s.",
				name.c_str(), Equip[ITEM_ACCESSORY]->name.c_str(),
				verb.c_str(), item->name.c_str());
			take_off(Equip[ITEM_ACCESSORY]);
			Equip[ITEM_ACCESSORY] = &Armor[0];
		}
		else
			message("%s %s the %s.", name.c_str(), verb.c_str(), 
				item->name.c_str());


	take_off(remove);
	put_on(item);
	cal_stats();

	return true;
}

void CPlayer::level_up()
{
	draw_explore_screen();

	int add[5] = {0, 0, 0, 0, 0}; 
	bool done = false;
	int ch;
	int pos = 0;
	
	int oldAtk, oldHit, oldEva, oldMEv, oldMHP, oldMMP;
	int newAtk, newHit, newEva, newMEv, newMHP, newMMP;

	oldAtk = dv(bStr, 2);
	oldHit = dv(bStr, 6);
	oldEva = dv(bLuc - 1, 3);
	oldMEv = dv(bInt + 2, 3);

	oldMHP = bonusMaxHP + (Level + 1) * (bVit + 5);
	oldMMP = bonusMaxMP + (Level + 1) * (bInt + 5);

	XP -= TNL;
	++Level;
	TNL += 40 * Level;
	int statup = CCard().suit + 1;

	do
	{
		_werase(levelupWindow);
		cbox(levelupWindow, BORDER_COLOR);

		format_print(levelupWindow, 2, "@ %s reaches level %d! ",
			name.c_str(), Level);
		format_print(levelupWindow, 3, "   Choose %d stats to increase.",
			statup);

		for(int i = 0; i < 5; i++)
		{
			switch (i)
			{
			case 0:
				_wputstring(levelupWindow, 5, 4, TITLE_COLOR, "Strength:");
				_wputstring(levelupWindow, 5, 20, TEXT_COLOR,
					std::to_string(bStr), i == pos);
				if (add[0])
				{
					_wputchar(levelupWindow, 5, 23, RIGHT_ARROW, LIGHTGRAY);
					_wputcstr(levelupWindow, 5, 25, TEXT_COLOR, "%d",
						bStr + add[0]);
				}
				break;

			case 1:
				_wputstring(levelupWindow, 6, 4, TITLE_COLOR, "Agility:");
				_wputstring(levelupWindow, 6, 20, TEXT_COLOR,
					std::to_string(bAgi), i == pos);
				if (add[1])
				{
					_wputchar(levelupWindow, 6, 23, RIGHT_ARROW, LIGHTGRAY);
					_wputcstr(levelupWindow, 6, 25, TEXT_COLOR, "%d",
						bAgi + add[1]);
				}
				break;

			case 2:
				_wputstring(levelupWindow, 7, 4, TITLE_COLOR, "Vitality:");
				_wputstring(levelupWindow, 7, 20, TEXT_COLOR,
					std::to_string(bVit), i == pos);
				if (add[2])
				{
					_wputchar(levelupWindow, 7, 23, RIGHT_ARROW, LIGHTGRAY);
					_wputcstr(levelupWindow, 7, 25, TEXT_COLOR, "%d",
						bVit + add[2]);
				}
				break;
				
			case 3:
				_wputstring(levelupWindow, 8, 4, TITLE_COLOR, "Intelligence:");
				_wputstring(levelupWindow, 8, 20, TEXT_COLOR,
					std::to_string(bInt), i == pos);
				if (add[3])
				{
					_wputchar(levelupWindow, 8, 23, RIGHT_ARROW, LIGHTGRAY);
					_wputcstr(levelupWindow, 8, 25, TEXT_COLOR, "%d",
						bInt + add[3]);
				}
				break;
				
			case 4:
				_wputstring(levelupWindow, 9, 4, TITLE_COLOR, "Luck:");
				_wputstring(levelupWindow, 9, 20, TEXT_COLOR,
					std::to_string(bLuc), i == pos);
				if (add[4])
				{
					_wputchar(levelupWindow, 9, 23, RIGHT_ARROW, LIGHTGRAY);
					_wputcstr(levelupWindow, 9, 25, TEXT_COLOR, "%d",
						bLuc + add[4]);
				}
				break;
			}
		}

		newAtk = dv(bStr + add[0], 2);
		newHit = dv(bStr + add[0], 6);
		newEva = dv(bLuc + add[4] - 1, 3);
		newMEv = dv(bInt + add[3] + 2, 3);

		newMHP = bonusMaxHP + (Level + 1) * ((bVit + add[2]) + 5);
		newMMP = bonusMaxMP + (Level + 1) * ((bInt + add[3]) + 5);


		_wputstring(levelupWindow, 11, 4, TITLE_COLOR, "Attack:");
		_wputcstr(levelupWindow, 11, 19, TEXT_COLOR, "%d", oldAtk);
		if (oldAtk != newAtk)
		{
			_wputchar(levelupWindow, 11, 23, RIGHT_ARROW, LIGHTGRAY);
			_wputcstr(levelupWindow, 11, 25, TEXT_COLOR, "%d", newAtk);
		}

		_wputstring(levelupWindow, 12, 4, TITLE_COLOR, "Hit:");
		_wputcstr(levelupWindow, 12, 19, TEXT_COLOR, "%d", oldHit);

		if (oldHit != newHit)
		{
			_wputchar(levelupWindow, 12, 23, RIGHT_ARROW, LIGHTGRAY);
			_wputcstr(levelupWindow, 12, 25, TEXT_COLOR, "%d", newHit);
		}

		_wputstring(levelupWindow, 13, 4, TITLE_COLOR, "Evade:");
		_wputcstr(levelupWindow, 13, 19, TEXT_COLOR, "%d", oldEva);
		if (oldEva != newEva)
		{
			_wputchar(levelupWindow, 13, 23, RIGHT_ARROW, LIGHTGRAY);
			_wputcstr(levelupWindow, 13, 25, TEXT_COLOR, "%d", newEva);
		}

		_wputstring(levelupWindow, 14, 4, TITLE_COLOR, "Max HP:");
		_wputcstr(levelupWindow, 14, 19, TEXT_COLOR, "%d", oldMHP);

		if (oldMHP != newMHP)
		{
			_wputchar(levelupWindow, 14, 23, RIGHT_ARROW, LIGHTGRAY);
			_wputcstr(levelupWindow, 14, 25, TEXT_COLOR, "%d", newMHP);
		}

		_wputstring(levelupWindow, 15, 4, TITLE_COLOR, "Max MP:");
		_wputcstr(levelupWindow, 15, 19, TEXT_COLOR, "%d", oldMMP);

		if (oldMMP != newMMP)
		{
			_wputchar(levelupWindow, 15, 23, RIGHT_ARROW, LIGHTGRAY);
			_wputcstr(levelupWindow, 15, 25, TEXT_COLOR, "%d", newMMP);
		}

		_wputstring(levelupWindow, 16, 4, TITLE_COLOR, "Magic Evade:");
		_wputcstr(levelupWindow, 16, 19, TEXT_COLOR, "%d", oldMEv);

		if (oldMEv != newMEv)
		{
			_wputchar(levelupWindow, 16, 23, RIGHT_ARROW, LIGHTGRAY);
			_wputcstr(levelupWindow, 16, 25, TEXT_COLOR, "%d", newMEv);
		}

		show_window(levelupWindow);

		switch (ch = _getch())
		{
		case _KEY_UP:
			pos--;
			if (pos < 0)
				pos = 4;
			break;

		case _KEY_LEFT:
			if (add[pos] > 0)
			{
				add[pos]--;
				statup++;
			}
			break;

		case _KEY_RIGHT:
			if (add[pos] < 1 && statup > 0)
			{
				add[pos]++;
				statup--;
			}
			break;

		case _KEY_DOWN:
			pos++;
			if (pos > 4)
				pos = 0;
			break;

		case KEY_ACCEPT:
			if (!statup)
				done = true;
			break;
			
		}

	} while (!done);

	bStr += add[0];
	bAgi += add[1];
	bVit += add[2];
	bInt += add[3];
	bLuc += add[4];

	oldMMP = MaxMP;
	oldMHP = MaxHP;
	
	cal_stats();

	MP += MaxMP - oldMMP;
	HP += MaxHP - oldMHP;
}

void print_stat_change(_wintype* window, int y, int x, int valA, int valB,
					   int digits, bool can_equip)
{
	_wputcstr(window, y, x, LIGHTGRAY, "%d", valA);
	
	if (valA == valB)
		return;

	_wputchar(window, y, x+ digits + 1, RIGHT_ARROW, LIGHTGRAY);

	_colortype stat_color = (valB > valA) ? LIGHTGREEN : LIGHTRED;

	if (!can_equip)
		stat_color = DARKGRAY;

	_wputcstr(window, y, x + digits + 3, stat_color, "%d", valB);
}

void CPlayer::print_stats(_wintype* window, CItem* equip, item_type slot, int y)
{
	IEquip nstats;
	
	if (equip)
	{
		if (!slot)
			slot = equip->type;

		CItem* remove = Equip[slot];

		Equip[slot] = equip;

		if (equip->type == ITEM_WEAPON && ((CWeapon*)equip)->twoHanded
			&& slot == ITEM_WEAPON)
		{
			CItem* remove_accessory = Equip[ITEM_ACCESSORY];
			Equip[ITEM_ACCESSORY] = noneItem[ITEM_ACCESSORY];
			cal_stats(false);
			nstats = IEquip(this);

			Equip[slot] = remove;
			Equip[ITEM_ACCESSORY] = remove_accessory;
			cal_stats(false);
		}
		else
		{
			cal_stats(false);
			nstats = IEquip(this);

			Equip[slot] = remove;
			cal_stats(false);
		}
	}
	else
	{
		nstats = IEquip(this);
		equip = noneItem[ITEM_WEAPON];
	}

	bool can_equip = ArmorSkill.count(equip->Set) ||
		WeaponSkill.count(equip->Set) || equip->name == "None" ||
		equip->Set->name == "Quest";

	_wputstring(window, y, 1, TITLE_COLOR, "Attack:");

	if (equip->Set->name == "Ammunition")
		print_stat_change(window, y, 11, ((CWeapon*)equip)->Atk,
			((CWeapon*)equip)->Atk, 3, can_equip);
	else
		print_stat_change(window, y, 11, Atk, nstats.Atk, 3, can_equip);

	_wputstring(window, y + 1, 1, TITLE_COLOR, "Hit:");

	if (equip->Set->name == "Ammunition")
		print_stat_change(window, y + 1, 11, ((CWeapon*)equip)->Hit,
			((CWeapon*)equip)->Hit, 3, can_equip);
	else
		print_stat_change(window, y + 1, 11, Hit, nstats.Hit, 3, can_equip);

	_wputstring(window, y + 2, 1, TITLE_COLOR, "Evade:");
	print_stat_change(window, y + 2, 11, Eva, nstats.Eva, 3, can_equip);

	_wputstring(window, y + 3, 1, TITLE_COLOR, "Max HP:");
	print_stat_change(window, y + 3, 11, MaxHP,	
		nstats.HP > 0 ? nstats.HP : 0, 3, can_equip);

	_wputstring(window, y + 4, 1, TITLE_COLOR, "Max MP:");
	print_stat_change(window, y + 4, 11, MaxMP,
		nstats.MP > 0 ? nstats.MP : 0, 3, can_equip);

	_wputstring(window, y, 26, TITLE_COLOR, "Strength:");
	print_stat_change(window, y, 42, Str, nstats.Str, 2, can_equip);

	_wputstring(window, y + 1, 26, TITLE_COLOR, "Agility:");
	print_stat_change(window, y + 1, 42, Agi, nstats.Agi, 2, can_equip);

	_wputstring(window, y + 2, 26, TITLE_COLOR, "Vitality:");
	print_stat_change(window, y + 2, 42, Vit, nstats.Vit, 2, can_equip);

	_wputstring(window, y + 3, 26, TITLE_COLOR, "Intelligence:");
	print_stat_change(window, y + 3, 42, Int, nstats.Int, 2, can_equip);

	_wputstring(window, y + 4, 26, TITLE_COLOR, "Luck:");
	print_stat_change(window, y + 4, 42, Luc, nstats.Luc, 2, can_equip);
}
