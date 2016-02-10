#include <list>
#include <string>

#include "Message.h"
#include "Party.h"
#include "Player.h"
#include "Quests.h"
#include "Town.h"
#include "Weapons.h"

CTown::CTown()
{
	name = *(it++);
	type = FEA_TOWN;
}

CTown::CTown(const CTown& town)
{
	name = town.name;
	type = FEA_TOWN;
}

void desc_town(int choice)
{
    switch (choice)
    {
        case 0:
        prompt("Restore HP and MP.");
        break;

        case 1:
        prompt("Buy weapons.");
        break;

        case 2:
        prompt("Buy armor.");
        break;

        case 3:
        prompt("Buy items.");
        break;

        case 4:
        prompt("Sell items for half their price.");
        break;

        case 5:
        prompt("Receive a quest.");
        break;
    }
}

void CTown::enter()
{
	buffer town_buffer;
	town_buffer.push_back("Inn");
	town_buffer.push_back("Weaponsmith");
	town_buffer.push_back("Armorer");
	town_buffer.push_back("Apothecary");
	town_buffer.push_back("Market");
	town_buffer.push_back("Wise Man");

	int select;
    do
    {
        select = menu_select(12, 5, town_buffer, desc_town);
		draw_map(leftWindow, Party.L);

        switch (select)
		{
		case 0:
			if (Party.quest)
				message("You depart upon your quest.");
			else
				message("You leave to explore the countryside.");
			break;

		case 1:
            message(
"A pleasant night's rest restores the party to full health.");
            message(std::string(
                "\"Hold RESET while you turn POWER off!!\" warns ") +
                "the innkeeper as you leave.");

            for(CPlayer& it : Party.Player)
            {
                it.HP = it.MaxHP;
                it.MP = it.MaxMP;
                it.status = it.always;
            }

            break;

        case 2:
			{
				message("You enter the Weaponry.");
				CShopFunctor functor(upperWindow, ITEM_WEAPON);
				//CItemFunctor functor(upperWindow, ITEM_WEAPON);
				//int i = 0;
				//foreach(CPlayer& it, Party.Player)
				//{				
				//	foreach(CItemSet* wset, it.WeaponSkill)
				//		foreach(CItem* weapon, *wset)
				//		{
				//			functor.Menu[i].push_back(weapon->name);
				//			functor.Item[i].push_back(weapon);
				//		}
				//	++i;
				//}

				(void)functor();
				break;
			}

		case 3:
			{
				message("You enter the Armory.");
				CShopFunctor functor(upperWindow, ITEM_BODY);
				//CItemFunctor functor(upperWindow, ITEM_BODY);
				//int i = 0;
				//foreach(CPlayer& it, Party.Player)
				//{
				//	foreach(CItemSet* aset, it.ArmorSkill)
				//		foreach(CItem* armor, *aset)
				//		{
				//			functor.Menu[i].push_back(armor->name);
				//			functor.Item[i].push_back(armor);
				//		}
				//	++i;
				//}

				(void)functor();
				break;
			}

		case 4:
			{
				message("You enter the Apothecary.");
				CItemFunctor functor(upperWindow, ITEM_ITEM);
				int i = 0;
				for(CPlayer& it : Party.Player)
				{
					for(CItem& item : Item)
						if (item.price != 0)
						{
							functor.Menu[i].push_back(item.name);
							functor.Item[i].push_back(&item);
						}
					++i;
				}

				(void)functor();
				break;
			}

		case 5:
			{
				message("You wander the market.");
				CItemFunctor functor(upperWindow, ITEM_ITEM, ITEM_MODE_SELL);

				int i = 0;
				for(CPlayer& it : Party.Player)
				{
					for(CItem* item : it.Inventory)
					{
						functor.Menu[i].push_back(item->name);
						functor.Item[i].push_back(item);
					}
					++i;
				}

				(void)functor();
				break;
			}

		case 6:
			{
				message("The wise man tells of the dangers facing "
					+ name + ".");
				CQuestFunctor functor(upperWindow);
				std::pair<int, int> result = functor();
				if (result.second--)
				{
					Party.quest = functor.Item[result.second];
					Party.QL = Party.quest->Lv;
					Dungeon[0].quest = Party.quest;
					Party.QuestTown = Party.L;

					more("You bravely vow to %s.",
						Party.quest->name.c_str());
				}
			}
        }
        Party.draw(rightWindow);
    } while (select);
}

std::list<std::string>::iterator CTown::it;
std::list<std::string> CTown::namelist;
