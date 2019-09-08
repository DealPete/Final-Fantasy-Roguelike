#include <assert.h>

#include "Combat.h"
#include "Defines.h"
#include "Input.h"
#include "Items.h"
#include "Map.h"
#include "Message.h"
#include "Output.h"
#include "Quests.h"
#include "Random.h"
#include "Town.h"
#include "Party.h"

int move_vec[4][2] =
{
{	-1, 0,  },		// MV_NORTH
{	0, 1,   },		// MV_EAST
{	1, 0,   },		// MV_SOUTH
{	0, -1   },		// MV_WEST
};

terrain_type overworld_terrain[13][3] =
{
{    TRN_GRASS,     TRN_MOUNTAIN,   TRN_WATER,      },
{    TRN_GRASS,     TRN_WATER,      TRN_MOUNTAIN,   },
{    TRN_MOUNTAIN,  TRN_WATER,      TRN_GRASS,      },
{    TRN_WATER,     TRN_MOUNTAIN,   TRN_GRASS,      },
{    TRN_MOUNTAIN,  TRN_GRASS,      TRN_WATER,      },
{    TRN_WATER,     TRN_GRASS,      TRN_MOUNTAIN,   },
{    TRN_GRASS,     TRN_GRASS,      TRN_MOUNTAIN,   },
{    TRN_GRASS,     TRN_GRASS,      TRN_WATER,      },
{    TRN_MOUNTAIN,  TRN_GRASS,      TRN_GRASS,      },
{    TRN_WATER,     TRN_GRASS,      TRN_GRASS,      },
{    TRN_GRASS,     TRN_MOUNTAIN,   TRN_GRASS,      },
{    TRN_GRASS,     TRN_WATER,      TRN_GRASS,      },
{    TRN_GRASS,     TRN_GRASS,      TRN_GRASS,      }
};

encounter_type overworld_encounter[13] =
{
	ENC_NONE, ENC_NONE, ENC_NONE, ENC_NONE, ENC_NONE, ENC_NONE,
	ENC_NONE, ENC_NONE, ENC_NONE,
//	ENC_MONSTERS, ENC_MONSTERS,  // official rules have 2 in 13 chance.
	ENC_NONE, ENC_MONSTERS,
	ENC_TOWN,
	ENC_DUNGEON
};

// EXIT_SOUTH isn't needed because it gets added when
// adjacent tiles are scanned for exits.
char dungeon_exits[13] =
{
	EXIT_NONE, EXIT_NONE,
	EXIT_NORTH, EXIT_NORTH, EXIT_NORTH,
	EXIT_WEST, EXIT_WEST,
	EXIT_EAST, EXIT_EAST,
	EXIT_NORTH | EXIT_WEST,
	EXIT_NORTH | EXIT_EAST,
	EXIT_WEST | EXIT_EAST,
	EXIT_ALL
};

encounter_type dungeon_encounter[13] =
{
	ENC_NONE, ENC_NONE, ENC_NONE, ENC_NONE, ENC_NONE, ENC_NONE,
	ENC_SAVE_POINT,
// ENC_MONSTERS, ENC_MONSTERS, ENC_MONSTERS,
// official rules have 3 in 13 chance of monster attack.
	ENC_NONE, ENC_NONE, ENC_MONSTERS,
	ENC_TREASURE, ENC_TREASURE,
	ENC_DAMAGE_FLOOR
};

CMapTile& CLocation::tile()
{
	return Dungeon[donjon].Map[floor].tile(y, x);
}

CMap& CLocation::map()
{
	return Dungeon[donjon].Map[floor];
}

CDomain& CLocation::domain()
{
	return Domain[Dungeon[donjon].domain];
}

CDungeon& CLocation::dungeon()
{
	return Dungeon[donjon];
}

CMapTile::CMapTile(const CMapTile& ref)
{
	terrain = ref.terrain;
	exits = ref.exits;
	explored = ref.explored;

	if (ref.Feature == NULL)
	{
		Feature = NULL;
		return;
	}

	switch (ref.Feature->type)
	{
	case FEA_TOWN:
		Feature = new CTown(*(CTown*)ref.Feature);
		break;

	case (FEA_DUNGEON | FEA_STAIRS_DOWN | FEA_STAIRS_UP):
		Feature = new CPortal(ref.Feature->type);
		break;

	default:
		Feature = new CFeature();
	}

	*Feature = *(ref.Feature);
}

CMapTile& CMapTile::operator=(const CMapTile& ref)
{
	if (this == &ref)
		return *this;

	terrain = ref.terrain;
	exits = ref.exits;
	explored = ref.explored;

	if (Feature != NULL) delete Feature;

	if (ref.Feature == NULL) return *this;

	switch (ref.Feature->type)
	{
	case FEA_TOWN:
		Feature = new CTown();
		break;

	case (FEA_DUNGEON | FEA_STAIRS_DOWN | FEA_STAIRS_UP):
		Feature = new CPortal(ref.Feature->type);
		break;

	default:
		Feature = new CFeature();
	}

	return *this;
}

CMapTile::~CMapTile()
{
	if (Feature != NULL) delete Feature;
}

CPortal::CPortal(feature_type ftype)
{
	type = ftype;
	switch (type)
	{
	case FEA_TUNNEL_EXIT:
		type = FEA_DUNGEON;
		Dest = Party.L;
		break;

	case FEA_TUNNEL:
		type = FEA_DUNGEON;

	case FEA_DUNGEON:
		Dest.donjon = Dungeon.size() - 1;
		Dest.floor = 0;
		Dest.y = Dest.x = MAP_SIZE / 2;
		break;

	case FEA_STAIRS_UP:
		if (Party.dungeon().is_tunnel)
		{
			type = FEA_STAIRS_UP;
			Dest = Dungeon[0].Map[0].create_tunnel_exit();
		}
		else
			Dest = Party.L;
		break;

	case FEA_STAIRS_DOWN:
		Party.dungeon().Map.push_back(
			CMap(static_cast<domain_type>(Party.domain().serial)));
		Party.dungeon().floors++;
		Dest.donjon = Party.L.donjon;
		Dest.floor = Party.L.floor + 1;
		Dest.y = Dest.x = MAP_SIZE / 2;
		break;

    default:
		break;
	}
}

// creates a new empty map.
CMap::CMap(domain_type domain)
{
	int y = MAP_SIZE / 2, x = MAP_SIZE / 2;

	key_used = false;
	tiles = 1;
	xoffset = yoffset = 0;

	yx.insert(yx.begin(), MAP_SIZE, std::deque<CMapTile*>());

	for(int i = 0; i < MAP_SIZE; i++)
		for(int j = 0; j < MAP_SIZE; j++)
			yx[i].push_back(new CMapTile());

	yx[y][x]->exits = EXIT_ALL;
	yx[y][x]->explored = true;

	if (domain == DOM_OVERWORLD)
	{
		yx[y][x]->terrain = TRN_GRASS;
		yx[y][x]->Feature = new CTown;
		Party.SaveSpots.push_back(CLocation(0, 0, y, x));

		for(int i = 0; i < 4; i++)
		{
			yx[y + move_vec[i][0]][x + move_vec[i][1]]->terrain = TRN_GRASS;
			unexplored_tile.insert(yx[y + move_vec[i][0]][x + move_vec[i][1]]);
		}
	}
	else
	{
		for(int i = 0; i < 4; i++)
			unexplored_tile.insert(yx[y + move_vec[i][0]][x + move_vec[i][1]]);
		yx[y][x]->terrain = TRN_DUNGEON;
		yx[y][x]->Feature = new CPortal(FEA_STAIRS_UP);
	}
#ifdef _DEBUG_MAP
	message("Map has %d unexplored tiles.", unexplored_tile.size());
#endif
}

void draw_dungeon_map(_wintype* window, CLocation Loc, bool cursor)
{
	_werase(window);

	unsigned int height = _getheight(window);
	unsigned int width = _getwidth(window);
	unsigned int centreY = height / 2;
	unsigned int centreX = width / 2;

	int top = 0;
	int left = 0;
	int bottom = height - 1;
	int right = width - 1;

	int y = Loc.y;
	int x = Loc.x;

	CMap& map = Loc.map();
	feature_type ftr;

	// Determine what part of the map vector to display on the screen.
	int minY = y - (height / 4);
	if (minY < map.min_y()) minY = map.min_y();
	int minX = x - (width / 4);
	if (minX < map.min_x()) minX = map.min_x();
	int maxY = y + dv(height, 4) - 1;
	if (maxY > map.max_y()) maxY = map.max_y();
	int maxX = x + dv(width, 4) - 1;
	if (maxX > map.max_x()) maxX = map.max_x();

	for(int i = minY;i <= maxY;i++)
	{
		for(int j = minX;j <= maxX;j++)
		{
			if (map.tile(i, j).explored)
			{
				// A white # indicates that the tile didn't display properly.
				unsigned char tile_ch = '#';
				_colortype tile_color = WHITE;

				if (map.tile(i, j).Feature == NULL)
					ftr = FEA_NONE;
				else
					ftr = map.tile(i, j).Feature->type;

				switch(ftr)
				{
				case FEA_STAIRS_UP:
					tile_ch = '<';
					tile_color = LIGHTGRAY;
					break;

				case FEA_STAIRS_DOWN:
					tile_ch = '>';
					tile_color = LIGHTGRAY;
					break;

				case FEA_SAVE_POINT:
					tile_ch = 'S';
					tile_color = BLUE;
					break;

				case FEA_DAMAGE_FLOOR:
#ifndef _WIN32
					tile_ch = '=';
#else
					tile_ch = 240;
#endif
					
					tile_color = RED;
					break;

				case FEA_BOSS:
					tile_ch = '&';
					tile_color = RED;
					break;

				default:
#ifndef _WIN32
					tile_ch = 46;
#else
					tile_ch = 250;
#endif
					tile_color = Loc.domain().color;
				}

				int Y = (i - y) * 2 + centreY;
				int X = (j - x) * 2 + centreX;
				for (int k = 0;k < 4;k ++)
				{
					int Yk = Y + move_vec[k][0];
					int Xk = X + move_vec[k][1];
					if ((Yk >= top) && (Yk <= bottom) &&
						(Xk >= left) && (Xk <= right))
					{
						if (map.tile(i, j).exits & (1 << k))
							_wputchar(window, Yk, Xk, 250,
								Loc.domain().color);
						else
							_wputchar(window, Yk, Xk, Loc.domain().wall,
								Loc.domain().color);
					}
					Yk += move_vec[(k + 1) % 4][0];
					Xk += move_vec[(k + 1) % 4][1];
					if ((Yk >= top) && (Yk <= bottom) &&
						(Xk >= left) && (Xk <= right))
					{
						_wputchar(window, Yk, Xk, Loc.domain().wall,
							Loc.domain().color);
					}
				}

				_wputchar(window, Y, X, tile_ch, tile_color,
					cursor && i == y && j == x);
			}
		}
	}
	if (&Loc.map() == &Party.map())
		if (minY <= Party.L.y && maxY >= Party.L.y &&
			minX <= Party.L.x && maxX >= Party.L.x)
		{
			_wputchar(window, centreY + 2 * (Party.L.y - y),
				centreX + 2 * (Party.L.x - x), '@', LIGHTGRAY);
		}
}

void draw_overworld_map(_wintype* window, CLocation Loc, bool cursor)
{
	_werase(window);

	unsigned int height = _getheight(window);
	unsigned int width = _getwidth(window);
	unsigned int centreY = height / 2;
	unsigned int centreX = width / 2;

	int y = Loc.y;
	int x = Loc.x;
	CMap& map = Loc.map();

	feature_type ftr;

	// Determine what part of the map to display on the screen.
	int minY = y - (height / 2);
	if (minY < map.min_y()) minY = map.min_y();
	int minX = x - (width / 2);
	if (minX < map.min_x()) minX = map.min_x();
	int maxY = y + dv(height, 2) - 1;
	if (maxY > map.max_y()) maxY = map.max_y();
	int maxX = x + dv(width, 2) - 1;
	if (maxX > map.max_x()) maxX = map.max_x();

	for(int i = minY;i <= maxY;i++)
	{
		for(int j = minX;j <= maxX;j++)
		{
			// A white # indicates that the tile didn't display properly.
			unsigned char tile_ch = '#';
			_colortype tile_color = WHITE;

			if (map.tile(i, j).Feature == NULL)
				ftr = FEA_NONE;
			else
				ftr = map.tile(i, j).Feature->type;

			switch(ftr)
			{
			case FEA_TOWN:
#ifndef _WIN32
				tile_ch = 37;
#else
				tile_ch = 254;
#endif
				tile_color = WHITE;
				break;

			case FEA_DUNGEON:
				tile_ch = dynamic_cast<CPortal*>(map.tile(i, j).Feature)->
					Dest.domain().entrance;
				tile_color = dynamic_cast<CPortal*>(
					map.tile(i, j).Feature)->Dest.domain().color;
				break;

			default:
				switch(map.tile(i, j).terrain)
				{
				case TRN_GRASS:
#ifndef _WIN32
					tile_ch = 46;
#else
					tile_ch = 250;
#endif
					tile_color = LIGHTGREEN;
					break;

				case TRN_MOUNTAIN:
					tile_ch = '^';
					tile_color = RED;
					break;

				case TRN_WATER:
#ifndef _WIN32
					tile_ch = '~';
#else
					tile_ch = 247;
#endif
					tile_color = BLUE;
					break;

				default:
					tile_ch = 0;
				}
			}
		
		if (!map.tile(i, j).explored && map.tile(i, j).terrain == TRN_GRASS)
				tile_color = DARKGRAY;

		if (tile_ch != 0)
			_wputchar(window, i - y + centreY, j - x + centreX, tile_ch,
				tile_color, cursor && i == y && j == x);
		}
	}
	if (&Loc.map() == &Party.map())
		if (minY <= Party.L.y && maxY >= Party.L.y &&
			minX <= Party.L.x && maxX >= Party.L.x)
		{
			_wputchar(window, centreY + (Party.L.y - y),
				centreX + (Party.L.x - x), '@', LIGHTGRAY);
		}
}

void draw_map(_wintype* window, CLocation Loc, bool cursor)
{
	if (Loc.domain().name == "Overworld")
		draw_overworld_map(window, Loc, cursor);
	else draw_dungeon_map(window, Loc, cursor);
	show_window(window);
}

void browse_map(_wintype* window, CLocation Loc)
{
	int ch;

	stack_window();

	cbox(window, BORDER_COLOR);
	show_window(window);

	_wintype* mapWindow = _derwin(window, _getheight(window) - 2,
		_getwidth(window) - 2, 1, 1);

	do
	{
		draw_map(mapWindow, Loc);
		switch (ch = _getch())
		{
		case _KEY_UP:
			Loc.y--;
			break;

		case _KEY_DOWN:
			Loc.y++;
			break;

		case _KEY_LEFT:
			Loc.x--;
			break;

		case _KEY_RIGHT:
			Loc.x++;
			break;

		case '<':
		case KEY_PGUP:
			//do
				if (Loc.floor > 0)
					Loc.floor--;
				else
				{
					if (Loc.donjon == 0)
						Loc.donjon = Dungeon.size() - 1;
					else
						Loc.donjon--;
					Loc.floor = Loc.dungeon().floors - 1;
					Loc.y = MAP_SIZE / 2;
					Loc.x = MAP_SIZE / 2;
				}
			//while ((Dungeon[Loc.dungeon].Map[Loc.floor].tiles == 1) &&
			//	(Dungeon[Loc.dungeon].domain != DOM_OVERWORLD));
			break;

		case '>':
		case KEY_PGDN:
			//do
				if (Loc.floor < Loc.dungeon().floors - 1)
					Loc.floor++;
				else
				{
					if (Loc.donjon == Dungeon.size() - 1)
						Loc.donjon = 0;
					else
						Loc.donjon++;
					Loc.floor = 0;
					Loc.y = MAP_SIZE / 2;
					Loc.x = MAP_SIZE / 2;
				}
			//while ((Dungeon[Loc.dungeon].Map[Loc.floor].tiles == 1) &&
			//	(Dungeon[Loc.dungeon].domain != DOM_OVERWORLD));
			break;
		}
	} while (ch != _KEY_CANCEL);

	unstack_window();
}

// This function grows the map in a random direction and
// puts the tunnel exit there somewhere.
CLocation CMap::create_tunnel_exit()
{
	CLocation Loc;
	switch(CCard().suit)
	{
	case SUIT_SPADE:
		grow_north();
		Loc.y = random(MAP_SIZE - 2) + 2;
		Loc.x = random(yx[0].size() - 4) + 2;
		break;

	case SUIT_CLUB:
		grow_east();
		Loc.y = random(yx.size() - 4) + 2;
		Loc.x = yx[0].size() - random(MAP_SIZE - 2) - 3;
		break;

	case SUIT_DIAMOND:
		grow_south();
		Loc.y = yx.size() - random(MAP_SIZE - 2) - 3;
		Loc.x = random(yx[0].size() - 4) + 2;
		break;

	case SUIT_HEART:
		grow_west();
		Loc.y = random(yx.size() - 4) + 2;
		Loc.x = random(MAP_SIZE - 2) + 2;
		break;
	}

	int y = Loc.y;
	int x = Loc.x;

	yx[y][x]->terrain = TRN_GRASS;
	yx[y][x]->Feature = new CPortal(FEA_TUNNEL_EXIT);
	yx[y][x]->explored = true;

	for(int i = 0; i < 4; i++)
	{
		yx[y + move_vec[i][0]][x + move_vec[i][1]]->terrain = TRN_GRASS;
		unexplored_tile.insert(yx[y + move_vec[i][0]][x + move_vec[i][1]]);
	}

	Loc.y -= yoffset;
	Loc.x -= xoffset;

	return Loc;
}

void CMap::grow_north()
{
	int map_width = yx[0].size();

	yx.insert(yx.begin(), MAP_SIZE, std::deque<CMapTile*>());

	for (int i = 0; i < MAP_SIZE; i++)
		for (int j = 0;j < map_width; j++)
		{
			yx[i].push_back(new CMapTile());
		}

	yoffset += MAP_SIZE;
}

void CMap::grow_south()
{
	int map_width = yx[0].size();

	yx.insert(yx.end(), MAP_SIZE, std::deque<CMapTile*>());

	for (unsigned int i = yx.size() - MAP_SIZE; i < yx.size(); i++)
		for (int j = 0; j < map_width; j++)
			yx[i].push_back(new CMapTile());
}

void CMap::grow_west()
{
	for(unsigned int i = 0;i < yx.size();i++)
		for(int j = 0; j < MAP_SIZE; j++)
			yx[i].insert(yx[i].begin(), new CMapTile());

	xoffset += MAP_SIZE;
}

void CMap::grow_east()
{
	for(unsigned int i = 0;i < yx.size();i++)
		for(int j = 0; j < MAP_SIZE; j++)
			yx[i].push_back(new CMapTile());
}

CDungeon::CDungeon(domain_type d, feature_type ftype)
{
	is_tunnel = (ftype == FEA_TUNNEL);
	domain = d;
	floors = 1;

	if (domain == DOM_OVERWORLD)
	{
		quest = &Quest[0];
		floors = 1;
		Map.push_back(CMap());
	}
	else
	{
		quest = Party.quest;
		Map.push_back(CMap(domain));

		if (Party.KeyItem.count(itemmap["Cube"]) &&
			!(Party.used_cube & 1 << (quest->serial - 1)))
		{
			draw_map(leftWindow, Party.L);
			message("You come upon %s.  Activate the cube?  (Y/N)",
				Domain[domain].name.c_str());
			
			_chtype answer;
			do
			{
				answer = _getch();
				if (answer == 'y')
					answer = 'Y';
				if (answer == 'n')
					answer = 'N';
			} while (answer != 'Y' && answer != 'N');

			if (answer == 'Y')
			{
				domain_type new_domain = (domain_type)(CCard().rank + 1);
				message(
	"The dungeon collapses upon itself and %s emerges from the rubble.",
				Domain[new_domain].name.c_str());
				domain = new_domain;
				Party.used_cube |= 1 << (quest->serial - 1);
			}
		}
	}
}

std::vector<CDomain> Domain;
CDungeonWrap Dungeon;
