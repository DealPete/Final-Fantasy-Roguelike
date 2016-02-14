#include <time.h>

#include "Init.h"
#include "InitData.h"
#include "Input.h"
#include "Map.h"
#include "Message.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Random.h"
#include "Savefile.h"

void start_game()
{
	seed_rng();

	load_data("config.txt");
	load_data("Data/FFCG.txt");
	
	init_display();

	message(
"Welcome to Final Fantasy!   Copyright 1985, Peter Deal & Ron Keating.");
	message("Would you like to (B)egin a new game, or (R)estore an old one?");

	int ch;

	do 
	{
		ch = _getch(INPUT_START);

		switch(ch)
		{
		case 'b':
		case 'B':
			Party.create();
			Dungeon.push_back(CDungeon(DOM_OVERWORLD, FEA_DUNGEON));
			set_new_savefile();
			Party.describe_location();
			break;

		case 'r':
		case 'R':
			if (restore_game())
			{
				Party.describe_location();
			}
			else
			{
				message(
"Couldn't find any savefiles!  Press (B) to begin a new game.");
				ch = KEY_ACCEPT;
			}
			break;

		default:
			ch = KEY_ACCEPT;
			break;
		}
	} while (ch == KEY_ACCEPT);

	time(&Party.start_time);

	message("Press ? for a list of commands.");

	Party.draw(rightWindow);
}
