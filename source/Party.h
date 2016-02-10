#ifndef __PARTY_H_
#define __PARTY_H_

#include <time.h>

#include <bitset>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "Map.h"
#include "Quests.h"

class CActor;
class CBattle;
class CItem;
class CPlayer;
class CTarget;

class CParty
{
public:
	CLocation L, QuestTown;
	CQuest* quest;
	CBattle* battle;

	std::vector<CPlayer> Player;
	int QL, Gil;

	unsigned long quests_complete;
	unsigned long quest_dungeons;
	unsigned long used_cube;

	std::set<CItem*> KeyItem;
	std::set<CItem*> EquippedKeyItem;

	std::string savefile;
	double playing_time;
	time_t start_time;

	int enemyPad_viewport;

	bool view_init_cards;
	bool flying;

	std::list<CLocation> SaveSpots;
	
	CParty();
	void restore(std::ifstream&);
	void out(std::ofstream&);

	void change_rows();
	void create();
	bool describe_location();
	void die();
	void display_status();
	void draw(_wintype*, CTarget& cursor, CTarget* Target = NULL);
	void draw(_wintype*, CActor*);
	void draw(_wintype*);
	void enter(char ch);
	void explore(int direction);
	void move(int direction);

	CMapTile& tile();
	CMap& map();
	CDomain& domain();
	CDungeon& dungeon();

private:
	bool add_player();
	void draw_players();
	void step_on_damage_floor();
};

class CStatusFunctor : public CSelectFunctor
{
public:
	CStatusFunctor(_wintype* w)
		: CSelectFunctor(w, true, false, 18, INPUT_STATUS)
	{}

	void describe(_wintype*);
	void empty_menu(_wintype* window)
	{
		describe(window);
	}
};

extern CParty Party;

#endif
