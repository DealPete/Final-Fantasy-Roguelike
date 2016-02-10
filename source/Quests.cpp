#include <deque>

#include "Items.h"
#include "Map.h"
#include "Message.h"
#include "Monsters.h"
#include "Party.h"
#include "Quests.h"

void CQuestFunctor::describe(_wintype* window)
{
	_werase(window);

	CQuest& q = *Item[pos];
	_wputstring(window, 1, 1, TITLE_COLOR, "Difficulty:");
	_wputcstr(window, 1, 15, LIGHTGRAY, "%d", q.Lv);
	
	_wputstring(window, 2, 1, TITLE_COLOR, "Enemy:");
	if (q.num_bosses == 1)
		_wputstring(window, 2, 15, LIGHTGRAY, q.boss->name);
	else
		_wputcstr(window, 2, 15, LIGHTGRAY, "%d %s",
		q.num_bosses, q.boss->plural.c_str());
	
	_wputstring(window, 3, 1, TITLE_COLOR, "Gold:");
	_wputcstr(window, 3, 15, LIGHTGRAY, "%d", q.Gil);

	_wputstring(window, 4, 1, TITLE_COLOR, "Item:");
	_wputstring(window, 4, 15, LIGHTGRAY, q.reward->name);
	
	if (q.reward->name == "None")
		format_print(window, 6,
		"You will transported to town upon completing this quest.");
}

void CQuestFunctor::empty_menu(_wintype* window)
{
	format_print(window, 1, 
		"The land is at peace, for you have completed all the quests.");
}

CQuestFunctor::CQuestFunctor(_wintype* w, int m)
		: CSelectFunctor(w, false, true, m, INPUT_NORMAL)
{
	Menu.push_back(buffer());

	int avglv = 0;
	for(CPlayer& it : Party.Player)
		avglv += it.Lv();
	avglv /= Party.Player.size();

	pos = 0;
	int posLv = 1;
	int i = 0;
	for(CQuest& it : Quest)
	{
		if (!(Party.quests_complete & 1 << i))
		{
/*			if (it.Lv > posLv &&
				it.Lv <= (avglv + (int)Party.Player.size() - 2))
			{
				posLv = it.Lv;
				pos = Menu[0].size();
			}*/
			Menu[0].push_back(it.name);
			Item.push_back(&it);
		}
		i++;
	}
}

void CQuestItemFunctor::select()
{
	if (Item[pos]->name == "Airship")
		if (Party.flying)
			if (Party.tile().terrain == TRN_MOUNTAIN ||
				(Party.tile().terrain == TRN_WATER &&
				!Party.KeyItem.count(itemmap["Ship"])))
				message("It would be too dangerous to disembark here.");
			else
			{
				Party.flying = false;
				message("You land the airship.");
			}
		else
		{
			if (Party.domain().name == "Overworld")
			{
				Party.flying = true;
				message("Stirring music plays as you lift into the skies.");
			}
			else
				message("The airship is waiting for you outside.");
		}
	else if (Item[pos]->name == "Key")
		if (Party.domain().name == "Overworld")
			message("The key cannot be used outside.");
		else if (Party.map().key_used)
			message("You have already used the key on this floor.");
		else
		{
			stack_window();
			draw_explore_screen();
			prompt("Open a passage in which direction?");
			char exit = EXIT_NONE;
			move_type move;

			while (exit == EXIT_NONE)
				switch (_getch())
				{
				case _KEY_UP:
					exit = EXIT_NORTH;
					move = MV_NORTH;
					break;

				case _KEY_RIGHT:
					exit = EXIT_EAST;
					move = MV_EAST;
					break;

				case _KEY_DOWN:
					exit = EXIT_SOUTH;
					move = MV_SOUTH;
					break;

				case _KEY_LEFT:
					exit = EXIT_WEST;
					move = MV_WEST;
					break;

				case _KEY_CANCEL:
					exit = EXIT_ALL;
					break;
				}

			if (exit != EXIT_ALL)
				if (exit & Party.tile().exits)
					message("There is already an exit in that direction.");
				else
				{
					CMapTile& opposite_tile = Party.map().tile(Party.L.y
						+ move_vec[move][0], Party.L.x + move_vec[move][1]);
					if (Party.map().unexplored_tile.empty() &&
						!opposite_tile.explored)
						message(
			"There is nothing left to explore on this floor.");
					else
					{
						message("The key shines and a passage opens.");
						Party.tile().exits |= exit;
						opposite_tile.exits |= (1 << (move + 2) % 4);
						Party.map().key_used = true;
					}
				}
			unstack_window();
		}
	else
		message("You fiddle with the " + Item[pos]->name + ".");
}

void CQuestItemFunctor::describe(_wintype* window)
{
	if (_getwidth(window) < 39)
	{
		_wputstring(window, 1, 1, ERROR_COLOR, "Error: window not wide \
enough!");
		return;
	}
	
	Item[pos]->describe(window);
}

CQuestItemFunctor::CQuestItemFunctor(_wintype* w)
		: CSelectFunctor(w, false, true)
{
	Menu.push_back(buffer());

	for(CItem* it : Party.KeyItem)
	{
		Menu[0].push_back(it->name);
		Item.push_back(it);
	}
}

void CQuestItemFunctor::empty_menu(_wintype* window)
{

	format_print(window, 1,
		"You have yet to collect any key items.");
}

void quest_item_menu()
{
	(void)CQuestItemFunctor(upperWindow)();
}

std::deque<CQuest> Quest;
