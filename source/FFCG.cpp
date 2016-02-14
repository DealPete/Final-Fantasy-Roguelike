#include <iostream>

#include "Misc.h"
#include "Init.h"
#include "Input.h"
#include "Items.h"
#include "Map.h"
#include "Message.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Quests.h"
#include "Savefile.h"
#include "Spells.h"
#include "Town.h"

int main(int argc, char* argv[])
{
	font_orient = "COL";
	
	if (argc > 1)
	{
		font_file = std::string(argv[1]);
		if (argc > 2)
			font_orient = std::string(argv[2]);
		else
			font_orient = "ROW";
	}
	else
		font_file = "terminal.png";
		
	int ch;

	start_game();

	for(;;)
	{
		switch (ch = _getch())
		{
		case _KEY_UP:
			Party.move(MV_NORTH);
			break;

		case _KEY_RIGHT:
			Party.move(MV_EAST);
			break;

		case _KEY_DOWN:
			Party.move(MV_SOUTH);
			break;

		case _KEY_LEFT:
			Party.move(MV_WEST);
			break;

		case 'c':
			select_spell();
			break;

		case 'e':
			{
				CEquipFunctor functor(upperWindow, ITEM_WEAPON, 
					ITEM_MODE_EQUIP);
				functor.build_menus();
				(void)functor();
				if (functor.equipped_something)
					Party.draw(rightWindow);
				break;
			}

		case 'i':
			item_menu(ITEM_MODE_USE);
			Party.draw(rightWindow);
			break;

		case 'I':
			quest_item_menu();
			draw_map(leftWindow, Party.L);
			break;

		case 'J':
			learn_spells();
			break;

		case 'm':
			browse_map(bigWindow, Party.L);
			break;

		case 'r':
			Party.change_rows();
			break;

		case KEY_ACCEPT:
			Party.enter(KEY_ACCEPT);
			break;

		case '>':
			Party.enter('>');
			break;

		case '<':
			Party.enter('<');
			break;

		case '?':
			browse_buffer(helpWindow, &help_buffer);
			break;

#ifdef _DEBUG
		case 'G':
			{
				message("You summon the Game Genie.");
				buffer buf;
				buf.push_back("Toggle Combat");
				buf.push_back("Lots of Money");
				buf.push_back("Every Spell");
				buf.push_back("Awesome Stats");

				int select = menu_select(12, 5, buf);
				
				switch(select)
				{
				case 0:
					message(
						"The genie disappears in a puff of smoke.");
					break;

				case 1:
					message(std::string("Combat is ") +
						((debug_flags ^= DEBUG_NO_COMBAT) ?
						"off." : "on."));
					break;

				case 2:
					message("You now have 1,000,000,000 Gil.");
					Party.Gil = 1000000000;
					Party.draw(rightWindow);
					break;

				case 3:
					message("The party learns all of its spells.");
					for(CPlayer& player : Party.Player)
					{
						for(CAbility* abil : player.bAbil)
							if (abil->type == "Set")
								for(CSpell* spell : spellsetmap[abil->name])
									player.bSpell.insert(spell);
						player.cal_stats();
					}
					break;

				case 4:
					message("You feel like gods!");
					for(CPlayer& player : Party.Player)
					{
						player.bInt = 99;
						player.bAgi = 99;
						player.bStr = 99;
						player.bLuc = 99;
						player.bVit = 99;
						player.cal_stats();
					}
					break;
				}
				break;
			}
#endif
			
		case 'S':
			if (save_game())
                quit_game(0);
			break;

		case 'Q':
			remove(Party.savefile.c_str());
			quit_game(0);
			break;
		}
	}

	return 0;
}
