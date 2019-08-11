#ifndef __SPELLS_H_
#define __SPELLS_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Misc.h"
#include "Output.h"

enum spell_type
{
	SPELL_DAMAGE,
	SPELL_HEAL,
	SPELL_CHANCE,
	SPELL_OTHER
};

class CActor;
class CItem;

class CSpell
{
public:
	static CSpell* get_spell(std::string spell_name);

	int serial;

	std::string name, desc, Set;
	int Lv, JP, MP;
	bool Map;
	AI_type AI;
	target_type target;
	std::string consumes;
	std::list<buffer> effect;

	int describe(_wintype*, bool casting = true);
};

class CPlayer;

class CSpellWindow : public CSelectWindow
{
public:
	std::vector<std::vector<CSpell*> > SpellItem;
	bool casting, combat;
	CPlayer* caster;

	CSpellWindow(_wintype* w, bool p = true, int m = 18);

	void describe(_wintype*);
	void empty_menu(_wintype*);
	void select();
	void build_learning_menus();
};

bool cast_spell(CPlayer&, CSpell*);
void select_spell();
void learn_spells();

extern std::deque<CSpell> Spell;
extern std::map<std::string, CSpell*> spellmap;
extern std::map<std::string,
	std::set<CSpell*, sort_serial<CSpell> > > spellsetmap;
extern std::map<std::string, target_type> targetmap;

#endif
