#include <deque>
#include <map>
#include <string>
#include <vector>

#include "Armor.h"
#include "Combat.h"
#include "Input.h"
#include "Items.h"
#include "Message.h"
#include "Misc.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Random.h"
#include "Spells.h"
#include "Target.h"
#include "Weapons.h"

CItem* CItem::get_item(std::string name)
{
	if (!itemmap[name])
		error("ERROR: Cannot find item " + name + ".");
	return itemmap[name];
}

void CItem::describe(_wintype* window, int y)
{
	y = format_print(window, y, description);

	if (price)
	{
		_wputstring(window, y + 1, 1, TITLE_COLOR, "Price:");
		_wputcstr(window, y + 1, 11, LIGHTGRAY, "%d", price);
	}
}

CShopFunctor::CShopFunctor(_wintype* w, item_type i) : CItemFunctor(w, i,
						ITEM_MODE_BUY, true, true)
{
	for(CItemSet& it : ItemSet)
		if (it.name != "None" && it.name != "Quest")
		{
			switch(it.type)
			{
			case ITEM_WEAPON:
				if (i == it.type)
					SuperMenu.push_back(&it);
				break;

			case ITEM_BODY:
			case ITEM_ACCESSORY:
			case ITEM_HEAD:
				if (i == ITEM_BODY)
					SuperMenu.push_back(&it);
				break;
			}
		}

	Menu.push_back(buffer());
	Item.push_back(std::vector<CItem*>());

	for(CItem* it : *SuperMenu[0])
	{
		if (it->Lv <= Party.QL + 4 - (int)Party.Player.size())
		{
			_colortype color = LIGHTGRAY;

			if (it->price > Party.Gil)
				color = DARKGRAY;

			Menu[0].push(it->name, color);
			Item[0].push_back(it);
		}
	}
}

CEquipFunctor::CEquipFunctor(_wintype* w, item_type i, item_mode_type m,
							 bool p) : CItemFunctor(w, i, m, p),
							 equipped_something(false)
{}

CItemFunctor::CItemFunctor(_wintype* w, item_type i, item_mode_type m,
						   bool p, bool shared_menu) : type(i), mode(m),
						   CSelectFunctor(w, p, true)
{
	if (!shared_menu)
		for(CPlayer& it : Party.Player)
		{
			Menu.push_back(buffer());
			Item.push_back(std::vector<CItem*>());
		}
}

void CEquipFunctor::describe(_wintype* window)
{
	if (party_view)
		player = &Party.Player[menu];

	if (_getwidth(window) < 39)
	{
		_wputstring(window, 1, 1, ERROR_COLOR, "Error: window not wide \
enough!");
		depth--;
		return;
	}

	std::vector<CItem*> selection;

	for(int i = ITEM_WEAPON; i <= ITEM_HEAD; i++)
		selection.push_back(player->Equip[i]);

	_werase(window);

	_wputstring(window, 1, 1, TITLE_COLOR, "Weapon:");
	_wputstring(window, 2, 1, TITLE_COLOR, "Accessory:");
	_wputstring(window, 1, 32, TITLE_COLOR, "Body:");
	_wputstring(window, 2, 32, TITLE_COLOR, "Head:");

	for(int i = ITEM_WEAPON; i <= ITEM_HEAD; i++)
	{
		_colortype color = TEXT_COLOR;
		int xprint, yprint;

		if (i < ITEM_BODY)
		{
			xprint = 13;
			yprint = i;
		}
		else
		{
			xprint = 39;
			yprint = i - 2;
		}
		
		_wputstring(window, yprint, xprint, color, selection[i - 1]->name,
			i == suppos + 1 && depth == 1);
	}
	
	if (depth == 1)
	{
		player->print_stats(window, selection[suppos],
			(item_type)(suppos + 1), 4);
		selection[suppos]->describe(window, 10);
	}
	else
	{
		player->print_stats(window, Item[menu][pos],
			(item_type)(suppos + 1), 4);
		Item[menu][pos]->describe(window, 10);
	}
}

void CEquipFunctor::supermenu()
{
	describe(rWindow);
	show_window(window);	
	show_window(lWindow);

	int ch;
	menu_move = HOVER;

	ch = _getch();

	int temppos = suppos;

	switch(ch)
	{
	case _KEY_UP:
		if (!(suppos % 2))
			suppos++;
		else
			suppos--;
		break;

	case _KEY_DOWN:
		if (suppos % 2 == 1)
			suppos--;
		else
			suppos++;
		break;

	case _KEY_LEFT:
		if (suppos < 2)
			suppos += 2;
		else
			suppos -= 2;
		break;

	case _KEY_RIGHT:
		if (suppos > 1)
			suppos -= 2;
		else
			suppos += 2;
		break;

	case KEY_PGUP:
		if (menu == 0)
				menu = Party.Player.size() - 1;
			else
				menu--;
		break;

	case KEY_PGDN:
		if (menu == Party.Player.size() - 1)
			menu = 0;
		else
			menu++;
		break;

	case KEY_ACCEPT:
		menu_move = DIVE;
		break;

	case _KEY_CANCEL:
		menu_move = CLIMB;
		break;
	}

	if (temppos != suppos)
	{
		switch(suppos)
		{
		case 0:
			type = ITEM_WEAPON;
			break;

		case 1:
			type = ITEM_ACCESSORY;
			break;

		case 2:
			type = ITEM_BODY;
			break;

		case 3:
			type = ITEM_HEAD;
			break;
		}

		build_menus();
	}
}

void CShopFunctor::supermenu()
{
	int Ss = (int)SuperMenu.size();
	int ch;
	menu_move = HOVER;

	_werase(rWindow);

	//_wputstring(rWindow, 1, 2, BORDER_COLOR, "Select a set");
	for(int i = 0; i < Ss; i++)
	{
		_colortype color = LIGHTGRAY;
		
		if (!Party.Player[menu].WeaponSkill.count(SuperMenu[i])
			&& !Party.Player[menu].ArmorSkill.count(SuperMenu[i])
			&& SuperMenu[i]->name != "Ammunition" && suppos != i)
			color = DARKGRAY;

		_wputstring(rWindow, (i / 3) + 1, (i % 3) * 17 + 2, color,
			SuperMenu[i]->name, suppos == i);
	}

	_wputstring(rWindow, _getheight(rWindow) - 2, 1, TITLE_COLOR,
		"Party Gold:");
	_wputcstr(rWindow, _getheight(rWindow) - 2, 13, LIGHTGRAY, "%d",
		Party.Gil);
	_wputstring(rWindow, _getheight(rWindow) - 2, _getwidth(rWindow) - 18,
		TITLE_COLOR, "Capacity:");
	_wputcstr(rWindow, _getheight(rWindow) - 2, _getwidth(rWindow) - 7,
		LIGHTGRAY, "%d/%d", Party.Player[menu].burden(), Party.Player[menu].Str);
	
	show_window(window);
	show_window(lWindow);
	show_window(rWindow);

	ch = _getch();

	int temppos = suppos;

	switch(ch)
	{
	case _KEY_UP:
		suppos -= 3;
		break;

	case _KEY_DOWN:
		suppos += 3;
		break;

	case _KEY_LEFT:
		suppos--;
		break;

	case _KEY_RIGHT:
		suppos++;
		break;

	case KEY_PGUP:
		if (menu == 0)
				menu = Party.Player.size() - 1;
			else
				menu--;
		break;

	case KEY_PGDN:
		if (menu == Party.Player.size() - 1)
			menu = 0;
		else
			menu++;
		break;

	case KEY_ACCEPT:
		menu_move = DIVE;
		break;

	case _KEY_CANCEL:
		menu_move = CLIMB;
		break;
	}

	if (suppos >= Ss)
		suppos -= 3 * (Ss / 3);

	if (suppos < 0)
		suppos += 3 * (Ss / 3);

	if (temppos != suppos)
	{
		Menu[0].clear();
		Item[0].clear();

		build_menus();
	}
}

bool CItemFunctor::build_menus(bool include_equipped)
{
	if (mode == ITEM_MODE_BUY)
		return false;

	bool menus_nonempty = false;
	int i = 0;

	for(CPlayer& it : Party.Player)
		if (party_view || &it == player)
		{
			Item[i].clear();
			Menu[i].clear();

			for(CItem* item : it.Inventory)
			{
				if (type == ITEM_ITEM || item->type == type)
				{
					Menu[i].push_back(item->name);
					Item[i].push_back(item);
				}

				if (type == ITEM_ACCESSORY && item->type == ITEM_WEAPON &&
					it.has_ability("Ambidexterity") &&
					item->Lv <= it.Lv())
				{
					Menu[i].push_back(item->name);
					Item[i].push_back(item);
				}
			}

			if (include_equipped)
				for(int j = ITEM_WEAPON; j <= ITEM_HEAD; j++)
				{
					CItem* item = it.Equip[j];
					if ((type == ITEM_ITEM || item->type == type)
						&& item->name != "None")
					{
						Menu[i].push_back(item->name);
						Item[i].push_back(item);
					}
				}

			if (mode == ITEM_MODE_EQUIP)
			{
//				if (player->Equip[type]->name != "None")
//				{
					Menu[i].push_back(noneItem[type]->name);
					Item[i].push_back(noneItem[type]);
//				}

				for(CItem* item : Party.KeyItem)
					if (item->type == type &&
						!Party.EquippedKeyItem.count(item))
					{
						Menu[i].push_back(item->name);
						Item[i].push_back(item);
					}
			}

			if (Menu[i++].size())
				menus_nonempty = true;
		}
	if (pos > (int)Item[menu].size() - 1)
	pos = Item[menu].size() - 1;
	if (pos < 0)
		pos = 0;

	return menus_nonempty;
}

bool CShopFunctor::build_menus()
{
	Item[0].clear();
	Menu[0].clear();

	for(CItem* it : *SuperMenu[suppos])
	{
		if (it->Lv <= Party.QL + 4 - (int)Party.Player.size() ||
			it->Set->name == "Ammunition")
		{
			_colortype color = LIGHTGRAY;

			if (it->price > Party.Gil)
				color = DARKGRAY;

			Menu[0].push(it->name, color);
			Item[0].push_back(it);
		}
	}

	if (!Menu[0].size())
		return false;
	else
		return true;
}

void CItemFunctor::describe(_wintype* window)
{
	if (_getwidth(window) < 39)
	{
		_wputstring(window, 1, 1, ERROR_COLOR, "Error: window not wide \
enough!");
		return;
	}

	unsigned int eff_menu = menu;
	if (eff_menu >= Menu.size())
		eff_menu = Menu.size() - 1;

	_wputstring(window, _getheight(window) - 2, 1, TITLE_COLOR,
		"Party Gold:");
	_wputcstr(window, _getheight(window) - 2, 13, LIGHTGRAY, "%d",
		Party.Gil);
	_wputstring(window, _getheight(window) - 2, _getwidth(window) - 18,
		TITLE_COLOR, "Capacity:");
	_wputcstr(window, _getheight(window) - 2, _getwidth(window) - 7,
		LIGHTGRAY, "%d/%d", Party.Player[menu].burden(), Party.Player[menu].Str);

	if (Item[eff_menu][pos]->type == ITEM_ITEM)
		Item[eff_menu][pos]->describe(window);
	else
	{
		if (player)
			player->print_stats(window, Item[eff_menu][pos]);
		else
			Party.Player[menu].print_stats(window, Item[eff_menu][pos]);
		Item[eff_menu][pos]->describe(window, 7);
	}
}

void CItemFunctor::empty_menu(_wintype* window)
{
	if (_getwidth(window) < 39)
	{
		_wputstring(window, 1, 1, ERROR_COLOR, "Error: window not wide \
enough!");
		return;
	}
	
	_wputstring(window, _getheight(window) - 2, 1, TITLE_COLOR,
		"Party Gold:");
	_wputcstr(window, _getheight(window) - 2, 13, LIGHTGRAY, "%d",
		Party.Gil);
	_wputstring(window, _getheight(window) - 2, _getwidth(window) - 18,
		TITLE_COLOR, "Capacity:");
	_wputcstr(window, _getheight(window) - 2, _getwidth(window) - 7,
		LIGHTGRAY, "%d/%d", Party.Player[menu].burden(), Party.Player[menu].Str);

	std::string item_string;

	switch (type)
	{
	case ITEM_ITEM:
		item_string = "items";
		break;

	case ITEM_WEAPON:
		item_string = "weapons";
		break;

	case ITEM_BODY:
		item_string = "armor";
		break;

	case ITEM_HEAD:
		item_string = "headgear";
		break;

	case ITEM_ACCESSORY:
		item_string = "accessories";
		break;
	}

	if (mode == ITEM_MODE_BUY)
		cformat_print(window, INFO_COLOR, 1, " %s can not afford any %s.",
			Party.Player[menu].name.c_str(), item_string.c_str());
	else
		cformat_print(window, INFO_COLOR, 1, " %s is not carrying any %s.",
			Party.Player[menu].name.c_str(), item_string.c_str());
}

void CItemFunctor::select()
{
	if (party_view)
		player = &Party.Player[menu];

	switch (mode)
	{
	case ITEM_MODE_BUY:
		{
			unsigned int eff_menu = menu;
			if (eff_menu >= Menu.size())
				eff_menu = Menu.size() - 1;

			if (Item[eff_menu][pos]->price <= Party.Gil)
			{
				message(player->name + " buys the " +
					Item[eff_menu][pos]->name + ".");
				Party.Gil -= Item[eff_menu][pos]->price;
				player->Inventory.push_back(Item[eff_menu][pos]);
				player->cal_stats();
				(*this).build_menus();
			}
			else
				message("You cannot afford the " +
					Item[eff_menu][pos]->name + ".");
		}
		break;

	case ITEM_MODE_SELL:
		if (Item[menu][pos]->price == 0)
			message("The %s is too precious to sell.",
				Item[menu][pos]->name.c_str());
		else
		{
			message("%s sells the %s.", player->name.c_str(),
				Item[menu][pos]->name.c_str());
			Party.Gil += dv(Item[menu][pos]->price, 2);

			player->remove_item(Item[menu][pos]);
			build_menus();
		}
		break;

	case ITEM_MODE_USE:
		{
			buffer buf;

			if (Item[menu][pos]->type == ITEM_ITEM)
				buf.push_back("Use");

			if (Item[menu][pos]->Set->name != "Quest")
			{
				if (Party.Player.size() > 1)
					buf.push_back("Trade");
				buf.push_back("Drop");
			}

			int select = menu_select(4, 1, buf);
			if (select == 0)
				return;

			std::string choice;
			buffer::iterator it = buf.begin();

			for(int i = 1; i < select; i++, it++)
				;

			choice = *it;

			if (choice == "Use")
			{
				if (player->use(Item[menu][pos], rWindow))
				{
					player->remove_item(Item[menu][pos]);
					build_menus();
				}
				break;
			}

			if (choice == "Drop")
			{
				message("%s discards the %s.",
					player->name.c_str(),
					Item[menu][pos]->name.c_str());
				player->remove_item(Item[menu][pos]);
				build_menus();
				break;
			}

			if (choice == "Trade")
			{
				buffer buf;
				std::vector<int> Plyr;

				for(int i = 0; i < (int)Party.Player.size(); i++)
					if (player != &Party.Player[i])
					{
						buf.push_back(Party.Player[i].name);
						Plyr.push_back(i);
					}

				int select = menu_select(4, 1, buf);

				if (select--)
				{
					message("%s hands the %s to %s.",
						player->name.c_str(),
						Item[menu][pos]->name.c_str(),
						Party.Player[Plyr[select]].name.c_str());
					Party.Player[Plyr[select]].
						Inventory.push_back(Item[menu][pos]);
					Menu[Plyr[select]].
						push_back(Item[menu][pos]->name.c_str());
					Item[Plyr[select]].push_back(Item[menu][pos]);
					player->remove_item(Item[menu][pos]);
					build_menus();
				}
				break;
			}

			break;
		}

	case ITEM_MODE_EQUIP:
		{
			int always = player->always;
			if (player->equip(Item[menu][pos], (item_type)(suppos + 1)))
			{
				((CEquipFunctor*)this)->equipped_something = true;
				menu_move = CLIMB;
			}
			build_menus();
			
			// party_view will only be true outside of combat.  Changing
			// the players' status will be handled by CActionEquip in combat.
			if (party_view)
			{
				unsigned long loss = (always & ~player->always) |
					(player->status & player->cancels);
				unsigned long gain = (player->always & ~always) &
					~player->cancels;

				if (loss)
					message("%s loses %s.", player->name.c_str(),
						comma_list(status_to_buffer(loss)).c_str());
				if (gain)
					message("%s gains %s.", player->name.c_str(),
						comma_list(status_to_buffer(gain)).c_str());
				player->status &= ~loss;
				player->status |= gain;
			}

			break;
		}

	case ITEM_MODE_SELECT:
		menu_move = CLIMB;
		break;

	}
}

void CPlayer::recoverHP(int recovery)
{
	if (Equip[ITEM_HEAD]->name == "Crown")
		recovery *= 2;
	int gain = gainHP(recovery);
	message("%s recovers %d HP.", name.c_str(), gain);
}

bool CPlayer::use(CItem* item, _wintype* window)
{
	if (item->type != ITEM_ITEM)
	{
		message("The %s cannot be used (try equipping it).",
			item->name.c_str());
		return false;
	}

	if (item->name == "Cabin" || item->name == "Tent" ||
		item->name == "Sleeping Bag")
	{
		if (Party.battle)
		{
			message("This is no time for resting.");
			return false;
		}
		else if (Party.domain().name == "Overworld" || (Party.tile().Feature
			&& Party.tile().Feature->type == FEA_SAVE_POINT))
		{
			if (!(item->name == "Sleeping Bag"))
			{
				message("The party sleeps soundly in the %s.",
					item->name.c_str());
				for(CPlayer& it : Party.Player)
					if (!it.defeated())
						if (item->name == "Tent")
						{
							it.gainHP(150);
							it.gainMP(150);
						}
						else
						{
							it.heal_to_full();
							it.MP = it.MaxMP;
						}
				return true;
			}
		}
		else
		{
			message("There is not enough room here.  Try another spot.");
			return false;
		}
	}

	CPlayer* target = NULL;
	int select;

	buffer buf;
	for(CPlayer& it : Party.Player)
		buf.push_back(it.name);

	if (window)
		Party.draw(window);

	if (item->name == "Unicorn Horn" || item->name == "Megalixir")
	{
		if (Party.battle)
		{
			CTarget tgt(Party.Player);
			if (!tgt(*this, TRGT_ALL))
				return false;
		}

		if (item->name == "Unicorn Horn")
		{
			bool anyEffect = false;
			for(CPlayer& player : Party.Player)
				if (Party.battle)
				{
					if (Party.battle->lose_status(player, ST_BAD & ~ST_KO ))
						anyEffect = true;
				}
				else if 
					(target->status & ST_BAD & ~ST_KO & ~target->always)
				{
					message("%s is cured.", target->name.c_str());
					*target -= ~ST_KO & ST_BAD & ~target->always;
					anyEffect = true;
				}

			if (anyEffect == false)
				message("The Unicorn Horn is wasted.");

			return true;
		}

		// Item is Megalixir.
		for(CPlayer& player : Party.Player)
			if (!player.defeated())
			{
				if (player == ST_UNDEAD)
				{
					player.gainMP(9999);
					Party.battle->heal(player, 
						player.MaxHP - player.curHP());
				}
				else
				{
					int HPgain = player.gainHP(9999);
					int MPgain = player.gainMP(9999);
					message("%s recovers %d HP and %d MP.",
						target->name.c_str(), HPgain, MPgain);
				}
			}

		return true;
	}

	if (Party.battle)
	{
		CTarget tgt(Party.Player);

		if (tgt(*this, TRGT_SINGLE))
			target = (CPlayer*)*tgt.begin();
	}
	else if (select = menu_select(4, 1, buf))
		target = &Party.Player[--select];

	if (target == NULL)
		return false;


	if (item->name == "Phoenix Down")
	{
		if (*target == ST_KO)
		{
			if (Party.battle)
				(void)Party.battle->lose_status(*target, ST_KO);
			else
				*target -= ST_KO;
			target->gainHP(target->eVit());
			message("%s returns to life.", target->name.c_str());
		}
		else
		{
			message("%s will live a while yet.", target->name.c_str());
			return false;
		}
	}

	if (*target == ST_KO)
	{
		message("%s is beyond the help of %s %s.",
			target->name.c_str(), a_or_an(item->name).c_str(),
			item->name.c_str());
		return false;
	}

	if (item->name == "Remedy")
	{
		if (Party.battle)
		{
			if (!Party.battle->lose_status(*target, ST_BAD & ~ST_KO ))
				message("The Remedy is wasted.");
		}
		else
			if (target->status & ST_BAD & ~ST_KO & ~target->always)
			{
				message("%s is cured.", target->name.c_str());
				*target -= ~ST_KO & ST_BAD & ~target->always;
			}
			else
			{
				message("%s has no need for a Remedy.",
					target->name.c_str());
				return false;
			}
	}
		
	if (target->defeated())
	{
		message("What %s needs is a Remedy.", target->name.c_str());
		return false;
	}

	if (item->name == "Sleeping Bag")
	{
		message("%s curls up in the sleeping bag.", target->name.c_str());
		target->recoverHP(150);
		int gain = target->gainMP(150);
		message("%s recovers %d MP.", target->name.c_str(), gain);
	}

	if (item->name == "Potion")
	{
		if (*target == ST_UNDEAD)
		{
			message("%s would be ill advised to drink that.",
				target->name.c_str());
			return false;
		}
		target->recoverHP(30);
	}

	if (item->name == "Ether")
	{
		int gain = target->gainMP(20);
		message("%s recovers %d MP.", target->name.c_str(), gain);
	}
		
	if (item->name == "Hi Potion")
	{
		if (*target == ST_UNDEAD)
		{
			message("%s would be ill advised to drink that.",
				target->name.c_str());
			return false;
		}
			
		target->recoverHP(70);
	}

		
	if (item->name == "Dry Ether")
	{
		int gain = target->gainMP(60);
		message("%s recovers %d MP.", target->name.c_str(), gain);
	}

	if (item->name == "X Potion")
	{
		if (*target == ST_UNDEAD)
		{
			message("%s would be ill advised to drink that.",
				target->name.c_str());
			return false;
		}
			
		target->recoverHP(150);
	}

	if (item->name == "X Ether")
	{
		int gain = target->gainMP(70);
		message("%s recovers %d MP.", target->name.c_str(), gain);
	}

	if (item->name == "Elixir")
	{
		if (*target == ST_UNDEAD)
		{
			message("%s would be ill advised to drink that.",
				target->name.c_str());
			return false;
		}

		int HPgain = target->gainHP(9999);
		int MPgain = target->gainMP(9999);
		message("%s recovers %d HP and %d MP.", target->name.c_str(),
			HPgain, MPgain);
	}

	if (item->name == "Golden Apple")
	{
		target->bonusMaxHP += 50;
		target->MaxHP += 50;
		message("%s's maximum HP are increased by 50.",
			target->name.c_str());
	}

	if (item->name == "Soma Drop")
	{
		target->bonusMaxMP += 50;
		target->MaxMP += 50;
		message("%s's maximum MP are increased by 50.",
			target->name.c_str());
	}			
	return true;
}

void item_menu(item_mode_type mode)
{
	CItemFunctor functor(upperWindow, ITEM_ITEM, mode);
	functor.build_menus();
	(void)functor();
}

CItemSet* random_set()
{
	CItemSet* chosenSet;

	do
	{
		chosenSet = &ItemSet[random(ItemSet.size())];
	} while (chosenSet->name == "Quest" || 
		chosenSet->name == "None" || chosenSet->name == "Item");

	return chosenSet;
}

std::deque<CItemSet> ItemSet;
std::map<std::string, CItem*> itemmap;
std::map<std::string, CItemSet*> itemsetmap;
std::map<int, CItem*> noneItem;
std::deque<CItem> Item;
std::vector<CItem> ComplexItem;
