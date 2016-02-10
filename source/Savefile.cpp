//#define _DEBUG_SAVEFILE

#include <iostream>
#include <fstream>
#include <sstream>

#include <time.h>

#include "Armor.h"
#include "Defines.h"
#include "Items.h"
#include "Jobs.h"
#include "Map.h"
#include "Message.h"
#include "Misc.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Quests.h"
#include "Savefile.h"
#include "Spells.h"
#include "Town.h"
#include "Weapons.h"

static std::string savegame_name;

void CPlayer::out(std::ofstream& savefile)
{
	savefile.write((char*)&bStr, sizeof bStr);
	savefile.write((char*)&bAgi, sizeof bAgi);
	savefile.write((char*)&bVit, sizeof bVit);
	savefile.write((char*)&bInt, sizeof bInt);
	savefile.write((char*)&bLuc, sizeof bLuc);
	savefile.write((char*)&bonusMaxHP, sizeof bonusMaxHP);
	savefile.write((char*)&bonusMaxMP, sizeof bonusMaxMP);

	savefile.write((char*)&Level, sizeof Level);

	savefile.write((char*)&HP, sizeof HP);
	savefile.write((char*)&MP, sizeof MP);

	savefile.write(name.c_str(), 16);
	savefile.write((char*)&sex, sizeof sex);

	savefile.write((char*)&JP, sizeof JP);
	savefile.write((char*)&XP, sizeof XP);
	savefile.write((char*)&TNL, sizeof TNL);

	savefile.write((char*)&backRow, sizeof backRow);
	savefile.write((char*)&status, sizeof status);

	savefile.write((char*)&job->serial, sizeof job->serial);

	int num = bAbil.size();
	savefile.write((char*)&num, sizeof num);
	for(CAbility* it : bAbil)
		savefile.write((char*)&it->serial, sizeof it->serial);

	num = Spell.size();
	savefile.write((char*)&num, sizeof num);
	for(CSpell* it : bSpell)
		savefile.write((char*)&it->serial, sizeof it->serial);

	num = WeaponSkill.size();
	savefile.write((char*)&num, sizeof num);
	for(CItemSet* it : WeaponSkill)
		savefile.write((char*)&it->serial, sizeof it->serial);

	num = ArmorSkill.size();
	savefile.write((char*)&num, sizeof num);
	for(CItemSet* it : ArmorSkill)
		savefile.write((char*)&it->serial, sizeof it->serial);

	num = Inventory.size();
	savefile.write((char*)&num, sizeof num);
	for(CItem* it : Inventory)
	{
		savefile.write((char*)&it->type, sizeof it->type);
		savefile.write((char*)&it->serial, sizeof it->serial);
	}

	for(int i = ITEM_WEAPON; i <= ITEM_HEAD; i++)
	{
		if (i == ITEM_ACCESSORY)
			savefile.write((char*)&Equip[i]->type, sizeof Equip[i]->type);
		savefile.write((char*)&Equip[i]->serial, sizeof Equip[i]->serial);
	}

	num = BlueMemory.size();
	savefile.write((char*)&num, sizeof num);
	for(CSpell* it : BlueMemory)
		savefile.write((char*)&it->serial, sizeof it->serial);
}

void CParty::out(std::ofstream& savefile)
{
	L.out(savefile);
	QuestTown.out(savefile);

	int questnum;
	if (quest)
		questnum = quest->serial + 1;
	else
		questnum = 0;

	savefile.write((char*)&questnum, sizeof questnum);

	int num = KeyItem.size();

	savefile.write((char*)&num, sizeof num);

	for(CItem* it : KeyItem)
	{
		savefile.write((char*)&it->type, sizeof it->type);
		savefile.write((char*)&it->serial, sizeof it->serial);
	}
		
	savefile.write((char*)&QL, sizeof QL);
	savefile.write((char*)&quests_complete, sizeof quests_complete);
	savefile.write((char*)&quest_dungeons, sizeof quest_dungeons);
	savefile.write((char*)&used_cube, sizeof used_cube);
	savefile.write((char*)&Gil, sizeof Gil);

	savefile.write((char*)&playing_time, sizeof playing_time);

	int save_spot_num = SaveSpots.size();
	savefile.write((char*)&save_spot_num, sizeof save_spot_num);

	for(CLocation& it : SaveSpots)
		it.out(savefile);

	int player_num = Player.size();
	savefile.write((char*)&player_num, sizeof player_num);

	for(CPlayer& it : Player)
		it.out(savefile);
}

void CLocation::out(std::ofstream& savefile)
{
	savefile.write((char*)&donjon, sizeof donjon);
	savefile.write((char*)&floor, sizeof floor);
	savefile.write((char*)&y, sizeof y);
	savefile.write((char*)&x, sizeof x);
}

void CTown::out(std::ofstream& savefile)
{
	savefile.write((char*)&type, sizeof type);

	savefile.write(name.c_str(), 16);
	savefile.write((char*)&QL, sizeof QL);
}

void CPortal::out(std::ofstream& savefile)
{
	savefile.write((char*)&type, sizeof type);

	Dest.out(savefile);
}

void CMap::out(std::ofstream& savefile)
{
	unsigned int mapY = yx.size();
	unsigned int mapX = yx[0].size();

	savefile.write((char*)&key_used, sizeof key_used);

	savefile.write((char*)&mapY, sizeof mapY);
	savefile.write((char*)&mapX, sizeof mapX);

	savefile.write((char*)&yoffset, sizeof yoffset);
	savefile.write((char*)&xoffset, sizeof xoffset);

	for(unsigned int i = 0;i < mapY;i++)
		for(unsigned int j = 0;j < mapX;j++)
		{
			CMapTile& ref = *yx[i][j];
			savefile.write((char*)&ref.exits, sizeof ref.exits);
			savefile.write((char*)&ref.explored, sizeof ref.explored);
			savefile.write((char*)&ref.terrain, sizeof ref.terrain);

			if (ref.Feature == NULL)
			{
				int tmp;
				if (unexplored_tile.count(&ref))
					tmp = FEA_UNEXPLORED;
				else
					tmp = FEA_NONE;
				savefile.write((char*)&tmp, sizeof tmp);
			}
			else
				ref.Feature->out(savefile);
		}
}

void CDungeon::out(std::ofstream& savefile)
{
	savefile.write((char*)&domain, sizeof domain);
	savefile.write((char*)&is_tunnel, sizeof is_tunnel);
	savefile.write((char*)&quest->serial, sizeof quest->serial);
	savefile.write((char*)&floors, sizeof floors);

	for(unsigned int i = 0; i < floors; i++)
		this->Map[i].out(savefile);
}

void write_header(std::ofstream& savefile)
{
	int x = Party.Player.size();
	savefile.write((char*)&x, sizeof x);

	for (int i = 0; i < x; i++)
	{
		savefile.write(Party.Player[i].name.c_str(), 16);
		savefile.write((char*)&Party.Player[i].HP,
			sizeof Party.Player[i].HP);
		savefile.write((char*)&Party.Player[i].MaxHP,
			sizeof Party.Player[i].MaxHP);
		savefile.write((char*)&Party.Player[i].MP,
			sizeof Party.Player[i].MP);
		savefile.write((char*)&Party.Player[i].MaxMP,
			sizeof Party.Player[i].MaxMP);
		savefile.write((char*)&Party.Player[i].job->serial,
			sizeof Party.Player[i].job->serial);
		int Lv = Party.Player[i].Lv();
		savefile.write((char*)&Lv, sizeof Lv);
	}

	savefile.write((char*)&Party.Gil, sizeof Party.Gil);
	savefile.write((char*)&Party.playing_time, sizeof Party.playing_time);
}

CPlayer::CPlayer(std::ifstream& savefile)
{
	savefile.read((char*)&bStr, sizeof bStr);
	savefile.read((char*)&bAgi, sizeof bAgi);
	savefile.read((char*)&bVit, sizeof bVit);
	savefile.read((char*)&bInt, sizeof bInt);
	savefile.read((char*)&bLuc, sizeof bLuc);
	savefile.read((char*)&bonusMaxHP, sizeof bonusMaxHP);
	savefile.read((char*)&bonusMaxMP, sizeof bonusMaxMP);

	savefile.read((char*)&Level, sizeof Level);

	savefile.read((char*)&HP, sizeof HP);
	savefile.read((char*)&MP, sizeof MP);

	char pname[16];
	savefile.read(pname, 16);
	name = pname;

	savefile.read((char*)&sex, sizeof sex);

	savefile.read((char*)&JP, sizeof JP);
	savefile.read((char*)&XP, sizeof XP);
	savefile.read((char*)&TNL, sizeof TNL);

	savefile.read((char*)&backRow, sizeof backRow);
	savefile.read((char*)&status, sizeof status);

#ifdef _DEBUG_SAVEFILE
	_wprintw(mainWindow, "bStr, bAgi, bVit, bInt, bLuc: %d %d %d %d %d\n",
		bStr, bAgi, bVit, bInt, bLuc);
	_wprintw(mainWindow,
		"Lv, HP, bonusMaxHP, MP, bonusMaxMP: %d %d %d %d %d\n",
		Lv, HP, bonusMaxHP, MP, bonusMaxMP);
	_wprintw(mainWindow, "name, JP, XP, TNL: %s %d %d %d\n",
		name.c_str(), JP, XP, TNL);
	getch();
#endif

	serial = Party.Player.size() - 1;

	int j;
	savefile.read((char*)&j, sizeof j);

	job = &Job[j];

	int num, serial;

	savefile.read((char*)&num, sizeof num);
	for (int i = 0; i < num; i++)
	{
		savefile.read((char*)&serial, sizeof serial);
		bAbil.insert(&Ability[serial]);
	}

	savefile.read((char*)&num, sizeof num);
	for (int i = 0; i < num; i++)
	{
		savefile.read((char*)&serial, sizeof serial);
		bSpell.insert(&::Spell[serial]);
	}

	savefile.read((char*)&num, sizeof num);
	for (int i = 0; i < num; i++)
	{
		savefile.read((char*)&serial, sizeof serial);
		WeaponSkill.insert(&ItemSet[serial]);
	}

	savefile.read((char*)&num, sizeof num);
	for (int i = 0; i < num; i++)
	{
		savefile.read((char*)&serial, sizeof serial);
		ArmorSkill.insert(&ItemSet[serial]);
	}

	item_type type;

	savefile.read((char*)&num, sizeof num);
	for (int i = 0; i < num; i++)
	{
		savefile.read((char*)&type, sizeof type);
		savefile.read((char*)&serial, sizeof serial);

		switch(type)
		{
		case ITEM_ITEM:
			Inventory.push_back(&Item[serial]);
			break;

		case ITEM_WEAPON:
			Inventory.push_back(&Weapon[serial]);
			break;

		case ITEM_BODY:
		case ITEM_HEAD:
		case ITEM_ACCESSORY:
			Inventory.push_back(&Armor[serial]);
			break;
		}
	}

	savefile.read((char*)&serial, sizeof serial);
	Equip[ITEM_WEAPON] = &Weapon[serial];

	savefile.read((char*)&type, sizeof type);
	savefile.read((char*)&serial, sizeof serial);

	if (type == ITEM_WEAPON)
		Equip[ITEM_ACCESSORY] = &Weapon[serial];
	else
		Equip[ITEM_ACCESSORY] = &Armor[serial];

	savefile.read((char*)&serial, sizeof serial);
	Equip[ITEM_BODY] = &Armor[serial];

	savefile.read((char*)&serial, sizeof serial);
	Equip[ITEM_HEAD] = &Armor[serial];

	savefile.read((char*)&num, sizeof num);
	for (int i = 0; i < num; i++)
	{
		savefile.read((char*)&serial, sizeof serial);
		BlueMemory.insert(&::Spell[serial]);
	}

	tStr = tAgi = tVit = tLuc = tInt = tHit = tEva = tAtk = tMP = 0;
	tresists = timmune = ELEM_NONE;
	battle_select_cursor = 3;
	cal_stats();
}

void CParty::restore(std::ifstream& savefile)
{
	Party.L = CLocation(savefile);
	Party.QuestTown = CLocation(savefile);

	int questnum;
	savefile.read((char*)&questnum, sizeof questnum);
	
	if (questnum)
		quest = &Quest[questnum - 1];
	else
		quest = NULL;
	
	item_type type;
	int itemnum, serial;

	savefile.read((char*)&itemnum, sizeof itemnum);
	for (int i = 0; i < itemnum; i++)
	{
		savefile.read((char*)&type, sizeof type);
		savefile.read((char*)&serial, sizeof serial);

		switch(type)
		{
		case ITEM_ITEM:
			KeyItem.insert(&Item[serial]);
			break;

		case ITEM_WEAPON:
			KeyItem.insert(&Weapon[serial]);
			break;

		case ITEM_BODY:
		case ITEM_HEAD:
		case ITEM_ACCESSORY:
			KeyItem.insert(&Armor[serial]);
			break;
		}
	}
	
	savefile.read((char*)&QL, sizeof QL);
	savefile.read((char*)&quests_complete, sizeof quests_complete);
	savefile.read((char*)&quest_dungeons, sizeof quest_dungeons);
	savefile.read((char*)&used_cube, sizeof used_cube);
	savefile.read((char*)&Gil, sizeof Gil);

#ifdef _DEBUG_SAVEFILE
	_wprintw(mainWindow, "Party.dungeon, .floor, .y, and .x are: %d %d %d %d\n",
		L.dungeon, L.floor, L.y, L.x);
	_wprintw(mainWindow, "QL and GP are: %d %d\n", QL, Gil);
	getch();
#endif

	savefile.read((char*)&playing_time, sizeof playing_time);

	int save_spot_num;
	savefile.read((char*)&save_spot_num, sizeof save_spot_num);

	for(int i = 0;i < save_spot_num;i++)
		SaveSpots.push_back(CLocation(savefile));

	int player_num;
	savefile.read((char*)&player_num, sizeof player_num);

	for (int i = 0; i < player_num; i++)
		Player.push_back(CPlayer(savefile));

	if (tile().terrain == TRN_MOUNTAIN)
		flying = true;
	else
		flying = false;
}

CLocation::CLocation(std::ifstream& savefile)
{
	savefile.read((char*)&donjon, sizeof donjon);
	savefile.read((char*)&floor, sizeof floor);
	savefile.read((char*)&y, sizeof y);
	savefile.read((char*)&x, sizeof x);
}

CTown::CTown(std::ifstream& savefile, feature_type ftype): CFeature(ftype)
{
	it++;
	char tname[16];
	savefile.read(tname, 16);
	name = tname;
	savefile.read((char*)&QL, sizeof QL);
}

CPortal::CPortal(std::ifstream& savefile, feature_type ftype): CFeature(ftype)
{
	Dest = CLocation(savefile);
}

CMap::CMap(std::ifstream& savefile)
{
	feature_type ftype = FEA_NONE;
	CMapTile* ptr = NULL;
	unsigned int mapY = 0;
	unsigned int mapX = 0;
	tiles = 0;

	savefile.read((char*)&key_used, sizeof key_used);
	savefile.read((char*)&mapY, sizeof mapY);

	savefile.read((char*)&mapX, sizeof mapX);


	savefile.read((char*)&yoffset, sizeof yoffset);
	savefile.read((char*)&xoffset, sizeof xoffset);

#ifdef _DEBUG_SAVEFILE
	_wprintw(mainWindow, "mapY, mapX, yoffset, and xoffset are: %d %d %d %d\n",
		mapY, mapX, yoffset, xoffset);
#endif

	yx.insert(yx.begin(), mapY, std::deque<CMapTile*>());

	for(unsigned int i = 0; i < mapY; i++)
	{
		for(unsigned int j = 0; j < mapX; j++)
		{
			yx[i].push_back(new CMapTile());

			CMapTile& ref = *yx[i][j];
			savefile.read((char*)&ref.exits, sizeof ref.exits);
			savefile.read((char*)&ref.explored, sizeof ref.explored);
			savefile.read((char*)&ref.terrain, sizeof ref.terrain);

			savefile.read((char*)&ftype, sizeof ftype);

#ifdef _DEBUG_SAVEFILE
			_wprintw(mainWindow, "exits, explored, terrain, and ftype are: %d %d %d %d\n",
				ref.exits, ref.explored, ref.terrain, ftype);
			getch();
#endif

			switch (ftype)
			{
			case FEA_NONE:
				ref.Feature = NULL;
				break;

			case FEA_UNEXPLORED:
				ref.Feature = NULL;
				//if (ref.terrain == TRN_WATER)
				//	unexplored_water_tile.insert(&ref);
				//else
					unexplored_tile.insert(&ref);
				break;

			case FEA_TOWN:
				ref.Feature = new CTown(savefile, FEA_TOWN);
				break;

			case FEA_DUNGEON:
			case FEA_STAIRS_UP:
			case FEA_STAIRS_DOWN:
				ref.Feature = new CPortal(savefile, ftype);
				break;

			default:
				ref.Feature = new CFeature(ftype);
			}

			if (ref.explored) tiles++;
		}
	}
}

CDungeon::CDungeon(std::ifstream& savefile)
{
	savefile.read((char*)&domain, sizeof domain);
	savefile.read((char*)&is_tunnel, sizeof is_tunnel);

	int questnum;
	savefile.read((char*)&questnum, sizeof questnum);
	quest = &Quest[questnum];

	savefile.read((char*)&floors, sizeof floors);

#ifdef _DEBUG_SAVEFILE
	_wprintw(mainWindow,
		"Dungeon.dungeon and .floors are: %d %d\n",
		domain, floors);
	getch();
#endif

	for(unsigned int i = 0; i < floors; i++)
		Map.push_back(CMap(savefile));
}

void CSaveHeader::describe(_wintype* window)
{
	_werase(window);

	if (_getwidth(window) < 51)
	{
		_wputstring(window, 1, 1, ERROR_COLOR, "Error: window not wide \
enough!");
		return;
	}

	_wputstring(window, 1, 1, TITLE_COLOR,
		"Player           HP       MP       Job               Lv");

	int i = 0;

	for(CPlayer& it : player)
	{
		_wputcstr(window, i + 3, 1, TEXT_COLOR, "%-15s", it.name.c_str());
		_wputcstr(window, i + 3, 18, TEXT_COLOR, "%d/%d", it.HP, it.MaxHP);
		_wputcstr(window, i + 3, 27, TEXT_COLOR, "%d/%d", it.MP, it.MaxMP);
		_wputcstr(window, i + 3, 36, TEXT_COLOR, "%-15s",
			it.job->title.c_str());
		_wputcstr(window, i + 3, 54, TEXT_COLOR, "%d", it.Lv());
		i++;
	}

	_wputstring(window, i + 5, 1, TITLE_COLOR, "Gil:            Playing \
Time:");
	_wputcstr(window, i + 5, 6, TEXT_COLOR, "%d", Gil);
	_wputcstr(window, i + 5, 33, TEXT_COLOR,
		"%.0f:%d", playing_time / 3600, (int)(playing_time / 60) % 60);
}

void CSaveHeaderFunctor::describe(_wintype* window)
{
	SaveHeader[pos].describe(window);
}

void CSaveHeaderFunctor::empty_menu(_wintype* window)
{
	_werase(window);
	_wputstring(window, 1, 1, ERROR_COLOR, "strange... no save files to \
display.");
}

CSaveHeader::CSaveHeader(std::ifstream& savefile)
{
	char pname[16];
	int x;
	int jserial = 0;

	savefile.read((char*)&x, sizeof x);

	for (int i = 0; i < x; i++)
	{
		player.push_back(CPlayer());
		savefile.read(pname, 16);
		player.back().name = pname;
		savefile.read((char*)&player.back().HP, sizeof player.back().HP);
		savefile.read((char*)&player.back().MaxHP, sizeof player.back().MaxHP);
		savefile.read((char*)&player.back().MP, sizeof player.back().MP);
		savefile.read((char*)&player.back().MaxMP, sizeof player.back().MaxMP);
		savefile.read((char*)&jserial, sizeof jserial);
		player.back().job = &Job[jserial];
		savefile.read((char*)&player.back().Level, sizeof player.back().Level);
	}

	savefile.read((char*)&Gil, sizeof Gil);
	savefile.read((char*)&playing_time, sizeof playing_time);

#ifdef _DEBUG_SAVEFILE
	_wputcstr(mainWindow, 1, 1, TEXT_COLOR, "Gil = %d,  playing_time = %f", Gil, playing_time);
#endif
}

void set_new_savefile()
{
	int i = 0;
	std::fstream savefile;

	do
	{
		if (i != 0)
			savefile.close();
		Party.savefile = "Save/FFCG" + to_string(i++) + ".sav";
		savefile.open(Party.savefile.c_str(), std::ios::in);
	} while (savefile.is_open());
}

bool save_game()
{
	std::ofstream savefile(Party.savefile.c_str(), std::ios::out | std::ios::binary);

	if (!savefile.is_open())
	{
	    message("Couldn't open savefile: " + Party.savefile);
	    return false;
	}

	savefile.seekp(0);

	Party.playing_time += difftime(time(NULL), Party.start_time);

	write_header(savefile);

	int d = Dungeon.size();
	savefile.write((char*)&d, sizeof d);

	for(int i = 0; i < d; i++)
		Dungeon[i].out(savefile);

	Party.out(savefile);
	savefile.close();
	return true;
}

bool restore_game()
{
	std::ifstream savefile;
	CSaveHeaderFunctor functor(bigWindow, false);

	int i;

	for(i = 0; i < max_save_file_num; i++)
	{
		std::string fname("Save/FFCG" + to_string(i) + ".sav");
		std::ifstream savefile(fname.c_str(), std::ios::in | std::ios::binary);
		if (savefile.is_open())
		{
			functor.SaveHeader.push_back(CSaveHeader(savefile));
			functor.SaveHeader[functor.SaveHeader.size() - 1].fname
				= fname;
			savefile.close();
			functor.Menu.push_back(buffer());
			functor.Menu[0].push_back(fname);
		}
	}

	if (functor.SaveHeader.size() == 0)
		return false;

	if (functor.SaveHeader.size() > 1)
	{
		do
		{
			i = functor().second;
		} while (i == 0);
	}
	else
		i = 1;

	Party.savefile = functor.SaveHeader[i - 1].fname;

	savefile.open(Party.savefile.c_str(), std::ios::in | std::ios::binary);
	savefile.seekg(0);

#ifdef _DEBUG_SAVEFILE
	_werase(mainWindow);
	scrollok(mainWindow, true);
#endif

	(void)CSaveHeader(savefile);

	int d = 0;
	savefile.read((char*)&d, sizeof d);

	for(int i = 0;i < d;i++)
		Dungeon.push_back(CDungeon(savefile));

	Party.restore(savefile);
	savefile.close();

#ifdef _DEBUG_SAVEFILE
	scrollok(mainWindow, false);
#endif

	if (Party.quest)
		message("Godspeed and " + Party.quest->name + ".");
	else
		message("Godspeed.");
	return true;
}
