#include "Combat.h"
#include "Input.h"
#include "Items.h"
#include "Jobs.h"
#include "Message.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Quests.h"
#include "Random.h"
#include "Target.h"
#include "Town.h"

CParty::CParty(): L(0, 0, MAP_SIZE / 2, MAP_SIZE / 2)
{
	QL = 1, quests_complete = quest_dungeons = used_cube = 0;
	flying = false, quest = NULL, battle = NULL;
}

void desc_row(int i)
{
	prompt("Who will change rows?");
}

void CParty::change_rows()
{
	buffer buf;
	for(CPlayer& player : Player)
		buf.push_back(player.name);

	int select = menu_select(12, 5, buf, desc_row);
	
	if (select--)
	{
		if (Player[select].backRow)
			message(Player[select].name + " moves to the front row.");
		else
			message(Player[select].name + " moves to the back row.");
		Player[select].backRow = !Player[select].backRow;
	}
	
	draw_map(leftWindow, L);
	draw(rightWindow);
}

void CParty::create()
{
	int ch;
	do
	{
		std::string prmpt;

		if (Player.size() == 0)
			while (!add_player());

		draw_players();

		prmpt = " Would you like to ";
		if (Player.size() < 4) prmpt += "(A)dd a hero, ";
		prmpt += "(R)emove a hero, or (J)ourney onward?";

		prompt(prmpt);

		ch = _getch(INPUT_START);

		switch (ch)
		{
		case 'a':
		case 'A':
			if (Player.size() < 4) add_player();
			break;

		case 'r':
		case 'R':
			Player.pop_back();
			break;
		}

	} while ((ch != 'J' && ch != _KEY_DOWN) || Player.size() == 0);

	Gil = 600;
	playing_time = 0;
}

bool CParty::describe_location()
{
	draw_map(leftWindow, Party.L);

	if (tile().Feature != NULL)
	{
		switch(tile().Feature->type)
		{
		case FEA_TOWN:
			message("You are outside " +
		static_cast<CTown*>(tile().Feature)->name + ", a peaceful town.");
			break;

		case FEA_DUNGEON:
			message("There is %s here.", dynamic_cast<CPortal*>(
				tile().Feature)->Dest.domain().name.c_str());
			break;

		case FEA_STAIRS_UP:
			message("Stairs lead up.");
			break;

		case FEA_STAIRS_DOWN:
			message("Stairs lead down.");
			break;

		case FEA_SAVE_POINT:
			message(
"A save point hovers in front of you, pulsating as it records your progress.");
			break;

		case FEA_DAMAGE_FLOOR:
			message("There is a damage floor here!");
			break;

		case FEA_BOSS:
/*			message("You have discovered the lair of %s. Enter?",
				quest->boss->name.c_str());

			if (yesno())
				the_boss_attacks();*/
			the_boss_attacks();
			break;

        default:
            message("ERROR: you step into a great nothingness!");
			break;
		}
		return true;
	}
	return false;
}

void CParty::die()
{
	if (config_flags & CONFIG_IRONMAN)
	{
		remove(Party.savefile.c_str());
		quit_game(0);
	}

	Party.Gil = 0;
	for(CPlayer& it : Party.Player)
	{
		it &= it.always & ~it.cancels;
		it.HP = it.MaxHP;
		it.MP = it.MaxMP;
	}
	message("You feel yourself floating, dreamlike, through the realm.");
	message("You can resume your game at any town or save spot.");
	message("Use the up and down arrows to cycle through them.");

	std::list<CLocation>::iterator Loc = SaveSpots.end();
	Loc--;
	
	int ch;
	cbox(upperWindow, BORDER_COLOR);
	show_window(upperWindow);

	_wintype* mapWindow = _derwin(upperWindow, _getheight(upperWindow) - 2,
		_getwidth(upperWindow) - 2, 1, 1);

	do
	{
		draw_map(mapWindow, *Loc, true);
		ch = _getch();

		if (ch == _KEY_UP)
			if (Loc != SaveSpots.begin())
				--Loc;

		if (ch == _KEY_DOWN)
		{
			++Loc;
			if (Loc == SaveSpots.end())
				--Loc;
		}
	
	} while (ch != KEY_ACCEPT);

	L = *Loc;
}

void CParty::display_status()
{
	(void)CStatusFunctor(bigWindow)();
}

void CParty::draw(_wintype* window, CTarget& cursor, CTarget* Target)
{
	_werase(window);
	_wputstring(window, 1, 1, TITLE_COLOR, "Player           HP       MP");

	int i = 3;

	if (!Party.battle)
	{
		_wputstring(window, _getheight(window) - 2, 2, TITLE_COLOR,
			"Gil:  ");
		_wputcstr(window, _getheight(window) - 2, 8, TEXT_COLOR, "%d",
			Gil);
	}

	for(CPlayer& it : Player)
	{
		bool backRow;

		if (Party.battle)
			backRow = it == CF_BACK_ROW;
		else
			backRow = it.backRow;

		_colortype color;
		bool reverse = false;
		
		if (cursor.count(&it))
		{
			color = WHITE;
			reverse = true;
		}
		else
		{
			color = backRow ? LIGHTGRAY : WHITE;
		
			if (Party.battle && it != CF_TARGETABLE)
				color = LIGHTBLUE;

			if (it & (ST_KO | ST_PETRIFY))
				color = DARKGRAY;
		}

		if (reverse)
			_wputcstr_reverse(window, i, 1, color, "%-15s",
				it.name.c_str());
		else
			_wputcstr(window, i, 1, color, "%-15s", it.name.c_str());

		_wputcstr(window, i, 18, color, "%d/%d", it.HP, it.MaxHP);
		_wputcstr(window, i, 18, color, "%d/%d", it.HP, it.MaxHP);
		std::string printMP(std::to_string(it.MP + it.tMP));
		_wputstring(window, i, 27, it.tMP ? CYAN : color, printMP);
		_wputcstr(window, i, 27 + printMP.length(), color, "/%d", it.MaxMP);

		if (Target && Target->count(&(CActor&)it))
		{
			_wputchar(window, i, 35, 251, WHITE);
		}

		i++;
		if (it & ~ST_KO)
			i = cformat_print(window, LIGHTRED, i,
				buffer_to_string(status_to_buffer(it.status & ~ST_KO,
				it.time_counters, it.blink_num, it.calcify_counters)));

		if (Party.battle && view_init_cards)
			_wputstring(window, i++, 1, GREEN,
				"[" + it.init->rank_string() + "]");
	}

	show_window(window);
}

void CParty::draw(_wintype* window, CActor* select)
{
	CTarget CT(select);
	draw(window, CT);
}

void CParty::draw(_wintype* window)
{
	CTarget CT;
	draw(window, CT);
}

void CParty::enter(char ch)
{
	if (tile().Feature == NULL) return;

	switch (tile().Feature->type)
	{
	case FEA_TOWN:
		if (ch != '<')
			dynamic_cast<CTown*>(tile().Feature)->enter();
		flying = false;
		return;
		break;

	case FEA_DUNGEON:
	case FEA_STAIRS_DOWN:
		if (ch == '<') return;
		break;

	case FEA_STAIRS_UP:
		if (ch == '>') return;
		break;

	default:
		return;
	}

	flying = false;

	for(CPlayer& it : Player)
		if (it == ST_FLOAT)
		{
			it -= ST_FLOAT;
			add_message(it.name + " loses float.");
		}

	L = dynamic_cast<CPortal*>(tile().Feature)->Dest;
	draw_map(leftWindow, L);
	draw(rightWindow);
}

void CParty::explore(int direction)
{
	CCard card, encCard;
	encounter_type encounter;

	// direction is the direction the party moved to enter the tile.
	// dfrom is the direction they came from.
	int dfrom = (direction + 2) % 4;

	if (domain().name == "Overworld")
	{
		for(int i = 0;i < 4;i++)
		{
			CMapTile& ref = map().tile(L.y + move_vec[i][0],
				L.x + move_vec[i][1]);
			if ((i != dfrom) && (ref.terrain == TRN_NONE))
			{
				ref.terrain =
					overworld_terrain[card.rank][(i - dfrom + 3) % 4];
				if (tile().terrain == TRN_WATER)
				{
					if (ref.terrain == TRN_GRASS)
						ref.terrain = TRN_WATER;
					else if (ref.terrain == TRN_WATER)
						ref.terrain = TRN_GRASS;
				}
				if (tile().terrain == TRN_MOUNTAIN)
				{
					if (ref.terrain == TRN_GRASS)
						ref.terrain = TRN_MOUNTAIN;
					else if (ref.terrain == TRN_MOUNTAIN)
						ref.terrain = TRN_GRASS;
				}
				if (!ref.explored)
				{
					if (ref.terrain == TRN_GRASS)
						map().unexplored_tile.insert(&ref);
//					if (ref.terrain == TRN_WATER)
//						map().unexplored_water_tile.insert(&ref);
				}
			}
		}
		encounter = overworld_encounter[encCard.rank];
	}
	else
	{
		tile().exits = (dungeon_exits[card.rank] << direction) |
			(dungeon_exits[card.rank] >> (4 - direction));
		for(int i = 0; i < 4; i++)
		{
			CMapTile& ref = map().tile(L.y + move_vec[i][0],
				L.x + move_vec[i][1]);
			if (ref.explored == true)
				if (ref.exits >> ((i + 2) % 4) & 1)
					tile().exits |= 1 << i;
				else
					tile().exits &= ~(1 << i);
			else if (tile().exits & 1 << i)
			{
				map().unexplored_tile.insert(&ref);
			}
		}
		encounter = dungeon_encounter[encCard.rank];
	}

	//if (tile->terrain == TRN_WATER)
	//{
	//	map->unexplored_water_tile.erase(tile);
	//	if (map->unexplored_tile.empty() &&
	//		map->unexplored_water_tile.empty())
	//		encounter = ENC_NO_EXITS;
	//}
	//else
	if (!flying)
	{
		map().unexplored_tile.erase(&tile());
		if (map().unexplored_tile.empty())
			encounter = ENC_NO_EXITS;
	}

	map().tiles++;

	if (Party.domain().name != "Overworld" &&
		map().tiles == 13 + dungeon().quest->Lv)
	{
		switch (domain().serial)
		{
		case DOM_ICE:
			message("Huge blocks of ice close off unexplored passages.");
			break;

		case DOM_SWAMP:
			message("Pools of fetid water fill the unexplored paths.");
			break;

		case DOM_FOREST:
			message("Living vines snake out and choke off the unexplored paths.");
			break;

		case DOM_CRYSTALLINE:
			message("The unexplored passageways suddenly fill with crystal!");
			break;

		case DOM_MOUNTAIN:
		case DOM_VOLCANO:
			message("Torrents of loose rock tumble down into the passes.");
			break;

		default:
			message("The dungeon rumbles as unexplored tunnels are sealed.");
		}

		for (int i = map().min_y(); i <= map().max_y(); i++)
			for (int j = map().min_x(); j <= map().max_x(); j++)
				if (map().unexplored_tile.count(&map().tile(i, j)))
					for (int k = 0; k < 4; k++)
						map().tile(i + move_vec[k][0],
							j + move_vec[k][1]).exits &= ~(1 << (k + 2) % 4);
		encounter = ENC_NO_EXITS;
	}

	if (!flying)
		tile().explored = true;
	map().grow(L.y, L.x);

	switch(encounter)
	{
	case ENC_NO_EXITS:
		if (domain().name == "Overworld")
		{
			Dungeon.push_back(CDungeon((domain_type)(CCard().rank + 1),
				FEA_TUNNEL));
			tile().Feature = new CPortal(FEA_TUNNEL);
		}
		else if (dungeon().is_tunnel)
			tile().Feature = new CPortal(FEA_STAIRS_UP);
		else if (CCard().rank <= 5 * (int)L.floor + 4 - Party.QL)
			tile().Feature = new CFeature(FEA_BOSS);
		else
			tile().Feature = new CPortal(FEA_STAIRS_DOWN);

		map().unexplored_tile.clear();
		break;

	case ENC_TOWN:
		if (!flying && tile().terrain == TRN_GRASS)
		{
			tile().Feature = new CTown();
			SaveSpots.push_back(CLocation(L));
			break;
		}

	case ENC_DUNGEON:
		if (!flying && tile().terrain == TRN_GRASS)
		{
#ifdef _DEBUG_FORCE_TUNNEL
			tile().Feature = new CPortal(FEA_TUNNEL);
			break;
#endif
			if (quest && !(1 << quest->serial & quest_dungeons))
			{
				quest_dungeons |= (1 << quest->serial);
				Dungeon.push_back(CDungeon((domain_type)(CCard().rank + 1),
					FEA_DUNGEON));
				tile().Feature = new CPortal(FEA_DUNGEON);
			}
		}
		break;

	case ENC_SAVE_POINT:
		tile().Feature = new CFeature(FEA_SAVE_POINT);
		SaveSpots.push_back(L);
		break;

	case ENC_TREASURE:
	{
		describe_location();

		more("You find a treasure box here.");

		CItem* found;
		CPlayer* finder;

		CCard treasure_type;

		if (treasure_type.suit == SUIT_DIAMOND)
		{
			int level = dungeon().quest->Lv + L.floor + 1;
			int gold_found = (CCard().suit + 1) *
				(level * level + level) * 10;

			add_message("The box contains %d Gil.", gold_found);
			Gil += gold_found;
			Party.draw(rightWindow);
			break;
		}

		int avglv = 0;

		std::vector<CPlayer*> vec;
		for(CPlayer& it : Player)
		{
			avglv += it.Lv();

			if (it.burden() + it.weight < it.Str)
				vec.push_back(&it);
		}

		avglv = dv(avglv, Player.size());

		if (vec.size() == 0)
			finder = &Player[random(Player.size())];
		else
			finder = vec[random(vec.size())];

		if (treasure_type.suit == SUIT_HEART)
			found = &Item[treasure_type.rank % avglv];
		else
		{
			CItemSet* found_set = random_set();
			
			// The items aren't organized in the set, so we choose
			// an arbitrary item from the set and scan through
			// for a more appropriate one.
			found = *found_set->begin();

			for(CItem* it : *found_set)
				if ((found->Lv < avglv && it->Lv > found->Lv)
					|| (found->Lv > avglv && it->Lv < found->Lv &&
					it->Lv >= avglv))
						found = it;
		}

			add_message("%s takes the %s.", finder->name.c_str(),
				found->name.c_str());
			finder->Inventory.push_back(found);
		break;
	}

	case ENC_DAMAGE_FLOOR:
		tile().Feature = new CFeature(FEA_DAMAGE_FLOOR);
		describe_location();
		step_on_damage_floor();
		return;

	case ENC_MONSTERS:
		if (!flying)
		{
			describe_location();
			monsters_attack();
		}
		break;

    default:
		break;
	}

	describe_location();

#ifdef _DEBUG_MAP
	message("Map has %d unexplored tiles.", std::map->unexplored_tile.size());
#endif
}

void CParty::move(int direction)
{
	for(CPlayer& it : Player)
		if (it.burden() > it.Str)
		{
			message(it.name + " is carrying too much to move.");
			return;
		}
			
	const CMapTile& dest_tile =	map().tile(
		L.y + move_vec[direction][0], L.x + move_vec[direction][1]);

	if (domain().name == "Overworld")
	{
		bool move = false;
		switch(dest_tile.terrain)
		{
		case TRN_MOUNTAIN:
			if (flying)
				move = true;
			else
			{
				message("Towering peaks impede your progress.");
				return;
			}
			break;

		case TRN_WATER:
			if (KeyItem.count(itemmap["Ship"]) || KeyItem.count(
				itemmap["Airship"]))
				move = true;
			else
			{
				message("The raging current would drown you.");
				return;
			}
			break;

		case TRN_GRASS:
			move = true;
			break;

		default:
			message("ERROR:  Walking into unknown terrain.");
		}

		if (move)
		{
			L.y += move_vec[direction][0];
			L.x += move_vec[direction][1];
			if (!dest_tile.explored)
			{
				explore(direction);
				return;
			}
		}

		if (describe_location())
			return;

		if (!flying && overworld_encounter[CCard().rank] == ENC_MONSTERS)
			monsters_attack();
	}
	else
	{
		if (tile().exits >> direction & 1)
		{
			L.y += move_vec[direction][0];
			L.x += move_vec[direction][1];
			if (!dest_tile.explored)
			{
				explore(direction);
				return;
			}
		}
		else return;

		if (describe_location())
		{
			if (tile().Feature &&
				tile().Feature->type == FEA_DAMAGE_FLOOR)
				step_on_damage_floor();
			return;
		}

		if (!flying && dungeon_encounter[CCard().rank] == ENC_MONSTERS)
			monsters_attack();
	}
}

CMapTile& CParty::tile()
{
	return L.tile();
}

CMap& CParty::map()
{
	return L.map();
}

CDomain& CParty::domain()
{
	return L.domain();
}

CDungeon& CParty::dungeon()
{
	return L.dungeon();
}

bool CParty::add_player()
{
	CJobFunctor functor(bigWindow, false);

	std::string name;

	functor.Menu.push_back(buffer());

	for (unsigned int i = 0; i < Job.size(); i++)
		functor.Menu[0].push_back(Job[i].title);

	functor.window = bigWindow;

	int job = functor().second;

	if (job-- == 0)
		return false;

	draw_players();

	prompt_input(" Please christen the " + Job[job].title + ": ", name);

	Player.push_back(CPlayer(job, name));
	return true;
}

void CParty::draw_players()
{
	_werase(rightWindow);
	
	_wputstring(rightWindow, 1, 1, YELLOW, "Player           Job");

	int i = 0;

	for(CPlayer& it : Player)
	{
		_wputcstr(rightWindow, i + 3, 1, TEXT_COLOR, "%-15s  %s",
			it.name.c_str(), it.job->title.c_str());
		i++;
	}

	show_window(rightWindow);
}

void CParty::step_on_damage_floor()
{
	int dam = CDeal(Party.dungeon().quest->Lv).total();			
	for(CPlayer& it : Player)
		if (!(it == ST_FLOAT) && !it.defeated())
		{
			int itdam = dam;
			if (itdam >= it.HP)
				itdam = it.HP - 1;
			if (itdam)
			{
				it.loseHP(itdam);
				add_message("%s takes %d damage.", it.name.c_str(), itdam);
			}
		}
	draw(rightWindow);
}

void CStatusFunctor::describe(_wintype* window)
{
	player = &Party.Player[menu];

	_werase(window);

	_wputstring(window, 1, 1, TITLE_COLOR, "Weapon:");
	_wputstring(window, 2, 1, TITLE_COLOR, "Accessory:");
	_wputstring(window, 1, 32, TITLE_COLOR, "Body:");
	_wputstring(window, 2, 32, TITLE_COLOR, "Head:");

	for (unsigned int i = ITEM_WEAPON; i <= ITEM_HEAD; i++)
	{
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
		
		_wputstring(window, yprint, xprint, TEXT_COLOR,
			(player->Equip[i])->name);
	}
	
	player->print_stats(window, NULL, ITEM_NONE, 4);

	_wputstring(window, 4, 49, TITLE_COLOR, "Level:");
	_wputcstr(window, 4, 58, LIGHTGRAY, "%d", player->Lv());
	_wputstring(window, 5, 49, TITLE_COLOR, "EXP:");
	_wputcstr(window, 5, 58, LIGHTGRAY, "%d", player->XP);
	_wputstring(window, 6, 49, TITLE_COLOR, "TNL:");
	_wputcstr(window, 6, 58, LIGHTGRAY, "%d", player->TNL);
	_wputstring(window, 7, 49, TITLE_COLOR, "JP:");
	_wputcstr(window, 7, 58, LIGHTGRAY, "%d", player->JP);
	_wputstring(window, 8, 49, TITLE_COLOR, "Job:");
	_wputcstr(window, 8, 58, LIGHTGRAY, "%s",
		player->job->title.c_str());

	_wputstring(window, 11, 1, TITLE_COLOR, "Weapon Skills:");
	int i = 0;
	for(CItemSet* it : player->WeaponSkill)
	{
		_wputstring(window, 13 + i, 1, LIGHTGRAY, it->name);
		++i;
	}

	_wputstring(window, 11, 26, TITLE_COLOR, "Armor Skills:");
	i = 0;
	for(CItemSet* it : player->ArmorSkill)
	{
		_wputstring(window, 13 + i, 26, LIGHTGRAY, it->name);
		++i;
	}

	_wputstring(window, 11, 49, TITLE_COLOR, "Abilities:");
	i = 0;
	for(CAbility* it : player->Abil)
	{
		_wputstring(window, 13 + i, 49, LIGHTGRAY, it->name);
		++i;
	}
}

CParty Party;
