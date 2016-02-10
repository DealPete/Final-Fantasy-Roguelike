#ifndef __MAP_H_
#define __MAP_H_

#include <deque>
#include <fstream>
#include <map>
#include <set>
#include <string>

#include "Output.h"

const int MAP_SIZE = 19;

const char EXIT_NONE = 0;
const char EXIT_NORTH = 1;
const char EXIT_EAST = 2;
const char EXIT_SOUTH = 4;
const char EXIT_WEST = 8;
const char EXIT_ALL = 15;

class CMonsterRace;

extern char dungeon_exits[13];

enum move_type
{
	MV_NORTH,
	MV_EAST,
	MV_SOUTH,
	MV_WEST,
};

extern int move_vec[4][2];

enum terrain_type
{
	TRN_NONE,
	TRN_DUNGEON,
	TRN_GRASS,
	TRN_MOUNTAIN,
	TRN_WATER,
};

extern terrain_type overworld_terrain[13][3];

enum feature_type
{
	FEA_NONE,
	FEA_UNEXPLORED,			// only used in savefiles.
	FEA_TOWN,
	FEA_DUNGEON,
	FEA_TUNNEL,				// These get converted into FEA_DUNGEON
	FEA_TUNNEL_EXIT,		// by CPortal's constructor.
	FEA_STAIRS_UP,
	FEA_STAIRS_DOWN,
	FEA_SAVE_POINT,
	FEA_DAMAGE_FLOOR,
	FEA_BOSS
};

enum encounter_type
{
	ENC_NONE,
	ENC_MONSTERS,
	ENC_TOWN,
	ENC_DUNGEON,
	ENC_SAVE_POINT,
	ENC_TREASURE,
	ENC_DAMAGE_FLOOR,
	ENC_NO_EXITS
};

extern encounter_type overworld_encounter[13];
extern encounter_type dungeon_encounter[13];

enum domain_type
{
	DOM_OVERWORLD = 0,
	DOM_WATER,
	DOM_MOUNTAIN,
	DOM_CAVERN,
	DOM_FACTORY,
	DOM_RESIDENCE,
	DOM_TEMPLE,
	DOM_SWAMP,
	DOM_ICE,
	DOM_VOLCANO,
	DOM_FOREST,
	DOM_DESERT,
	DOM_TOWER,
	DOM_CRYSTALLINE
};

class CQuest;
class CMapTile;
class CMap;
class CDomain;
class CDungeon;
class CDungeonWrap;

extern std::vector<CDomain> Domain;
extern CDungeonWrap Dungeon;

class CLocation
{
public:
	unsigned int donjon, floor;
	int y, x;

	CLocation(unsigned int d = 0, unsigned int f = 0,
		unsigned int Y = 0, int X = 0):
		donjon(d), floor(f), y(Y), x(X) {}

	CLocation(const CLocation& loc):
		donjon(loc.donjon), floor(loc.floor), y(loc.y), x(loc.x) {}

	CLocation(std::ifstream& savefile);

	CMapTile& tile();
	CMap& map();
	CDomain& domain();
	CDungeon& dungeon();

	void out(std::ofstream& savefile);
};

class CDomain
{
public:
	int serial;
	std::string name;
	unsigned char entrance;
	unsigned char wall;
	_colortype color;
	std::vector<CMonsterRace*> encounter;
};

class CFeature
{
public:
	feature_type type;

	CFeature(feature_type ftype = FEA_NONE): type(ftype)
	{}

	CFeature(std::ifstream& savefile)
	{
		savefile.read((char*)&type, sizeof type);
	}

	virtual void out(std::ofstream& savefile)
	{
		savefile.write((char*)&type, sizeof type);
	}

	virtual ~CFeature() {}
};

class CPortal: public CFeature
{
public:
	CLocation Dest;

	CPortal(feature_type ftype);
	CPortal(std::ifstream& savefile, feature_type ftype);

	void out(std::ofstream& savefile);
};

class CMapTile
{
public:
	terrain_type terrain;
	CFeature* Feature;
	char exits;
	bool explored;

	CMapTile():
		terrain(TRN_NONE), Feature(NULL),
		exits(EXIT_ALL), explored(false) {}

	CMapTile(const CMapTile& ref);
	CMapTile& operator=(const CMapTile& ref);
	~CMapTile();
};

/*	In order to allow an infinite theoretical std::map, we expand the
	map vector dynamically when the party approaches the edge.
	This creates a problem for external elements, such as the Portal
	feature, that refer to the map by absolute coordinates.  Rather
	than updating a mess of pointers whenever the map is shifted, we
	keep track of changes internally with the xoffset and yoffset
	variables, and present an absolute coordinate system to the
	program through public methods.
*/
class CMap
{
public:
	unsigned int tiles;
	bool key_used;

	std::set<CMapTile*> unexplored_tile;
//	std::set<CMapTile*> unexplored_water_tile;

	CMap(domain_type domain);
	CMap(std::ifstream& savefile);

	CPortal& entrance()
	{
		return *dynamic_cast<CPortal*>(
			tile(MAP_SIZE / 2, MAP_SIZE / 2).Feature);
	}

	CMapTile& tile(int y, int x)
	{
		return *yx[y + yoffset][x + xoffset];
	}

	unsigned int height()
	{
		return yx.size();
	}

	unsigned int width()
	{
		return yx[0].size();
	}

	int min_y()
	{
		return -yoffset;
	}

	int min_x()
	{
		return -xoffset;
	}

	int max_y()
	{
		return height() - yoffset - 1;
	}

	int max_x()
	{
		return width() - xoffset - 1;
	}

	void del(int y, int x)
	{
		delete yx[y + yoffset][x + xoffset];
	}

	void grow(int y, int x)
	{
		y += yoffset;
		x += xoffset;
		if (y <= 2) grow_north();
		if (y >= (int)yx.size() - 3) grow_south();
		if (x <= 2) grow_west();
		if (x >= (int)yx[0].size() - 3) grow_east();
	}

	CLocation create_tunnel_exit();

	void out(std::ofstream& savefile);

private:
	int yoffset, xoffset;
	std::deque<std::deque<CMapTile*> > yx;

	void grow_north();
	void grow_south();
	void grow_west();
	void grow_east();

	void no_exits();
};

void draw_map(_wintype* _wintype, CLocation Loc, bool cursor = false);
void browse_map(_wintype* _wintype, CLocation Loc);

class CDungeon
{
public:
	domain_type domain;

	bool is_tunnel;
	
	// floors is the number of floors that have been created.
	unsigned int floors;
	CQuest* quest;

	std::deque<CMap> Map;

	CDungeon(domain_type d, feature_type ftype);
	CDungeon(std::ifstream& savefile);

	void out(std::ofstream& savefile);
};

class CDungeonWrap : public std::vector<CDungeon>
{
public:
	~CDungeonWrap()
	{
		//foreach(CDungeon& d, *this) 
		for(CDungeon& d : *this)
			for(unsigned int m = 0; m < d.Map.size(); m++)
			{
				CMap& ref = d.Map[m];
				for(int y = ref.min_y(); y <= ref.max_y(); y++)
					for(int x = ref.min_x(); x <= ref.max_x(); x++)
						ref.del(y, x);
			}
	}
};

#endif
