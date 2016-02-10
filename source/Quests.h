#ifndef __QUESTS_H_
#define __QUESTS_H_

#include <deque>
#include <string>
#include <vector>

#include "Items.h"
#include "Output.h"

class CMonsterRace;
class CSpell;

class CQuest
{
public:
	int serial, Gil, Lv, num_bosses;
	std::string name;

	CMonsterRace* boss;
	CItem* reward;
};

class CQuestFunctor : public CSelectFunctor
{
public:

	std::vector<CQuest*> Item;
	CQuestFunctor(_wintype* w, int m = 30);

	void describe(_wintype*);
	void empty_menu(_wintype*);
};

class CQuestItemFunctor : public CSelectFunctor
{
public:
	std::vector<CItem*> Item;

	CQuestItemFunctor(_wintype*);
	void describe(_wintype*);
	void empty_menu(_wintype*);
	void select();
};

class CItemSlab : public CItem
{
public:
	std::set<CSpell*>* spellset;
};

void quest_item_menu();

extern std::deque<CQuest> Quest;

#endif
