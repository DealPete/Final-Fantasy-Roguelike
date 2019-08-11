#include <map>
#include <set>
#include <vector>

#include "Action.h"
#include "Actor.h"
#include "Message.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Random.h"
#include "Spells.h"
#include "Target.h"
#include "Town.h"

CSpell* CSpell::get_spell(std::string name)
{
	if (!spellmap[name])
		error("ERROR: Cannot find spell " + name + ".");
	return spellmap[name];
}

int CSpell::describe(_wintype* window, bool casting)
{
	if (_getwidth(window) < 39)
	{
		_wputstring(window, 1, 1, ERROR_COLOR, "Error: window not wide \
enough!");
		return 0;
	}
	
	_wputstring(window, 1, 1, TITLE_COLOR, "Spell:");
	_wputstring(window, 1, 8, LIGHTGRAY, name);
	_wputstring(window, 1, 25, TITLE_COLOR, "Level:");
	_wputcstr(window, 1, 32, LIGHTGRAY, "%d", Lv);
	_wputstring(window, 2, 25, TITLE_COLOR, "MP Cost:");
	_wputcstr(window, 2, 34, LIGHTGRAY, "%d", MP);
	_wputstring(window, 2, 1, TITLE_COLOR, "Target:");

	const char* targets[] =
		{ "None", "Any", "Single", "Group",	"Self",
		"Other", "Random", "Random Group", "All"};

	_wputstring(window, 2, 9, LIGHTGRAY, targets[target]);

	int y = 4;

	if (!casting)
	{
		_wputstring(window, 3, 1, TITLE_COLOR, "JP Required:");
		_wputcstr(window, 3, 14, LIGHTGRAY, "%d", JP);
		y++;
	}
	
	return format_print(window, y, desc);
}

CSpellWindow::CSpellWindow(_wintype* w, bool p, int m)
		: CSelectWindow(w, p, true, m)
	{
		for(CPlayer& player : Party.Player)
		{
			Menu.push_back(buffer());
			SpellItem.push_back(std::vector<CSpell*>());
		}
	}

void CSpellWindow::describe(_wintype* window)
{
	CSpell* spell = SpellItem[menu][pos];
	int y = spell->describe(window, casting);

	if (y==0) return;

	y += 2;

	if (!combat)
		caster = &Party.Player[menu];

	if (casting)
	{
		_wputstring(window, _getheight(window) - 2, 1, TITLE_COLOR, "MP:");
		if (caster->tMP)
			_wputcstr(window, _getheight(window) - 2, 5, LIGHTGRAY, "%d + %d/%d",
				caster->MP, caster->tMP, caster->MaxMP);
		else
			_wputcstr(window, _getheight(window) - 2, 5, LIGHTGRAY, "%d/%d",
				caster->MP, caster->MaxMP);

		_setColor(window, INFO_COLOR);
	
		if ((spell->Set == "Blue High" || spell->Set == "Blue Low") &&
			!caster->BlueMemory.count(spell))
			y = format_print(window, y, "%s has not witnessed %s being cast.",
				caster->name.c_str(), spell->name.c_str());
		if (!combat && !spell->Map)
			y = format_print(window, y, "%s can only be cast in combat.",
				spell->name.c_str());
		else if (spell->MP > caster->MP + caster->tMP)
			y = format_print(window, y, "%s does not have enough MP to cast %s.",
				caster->name.c_str(), spell->name.c_str());
//		else
//			y = format_print(window, y, "Casting %s would leave %s with %d MP.",
//				spell->name.c_str(), caster->name.c_str(),
//				caster->MP - spell->MP);
	}
	else
	{
		_wputstring(window, _getheight(window) - 2, 1, TITLE_COLOR, "JP:");
		_wputcstr(window, _getheight(window) - 2, 5, LIGHTGRAY, "%d",
			caster->JP);

		_setColor(window, INFO_COLOR);

		if (spell->JP > caster->JP)
			y = format_print(window, y,
				"%s does not have enough JP to learn %s.",
				caster->name.c_str(), spell->name.c_str());
		//else
		//	y = format_print(window, y, "Learning %s would leave %s with %d JP.",
		//		spell->name.c_str(), caster->name.c_str(),
		//		caster->JP - spell->JP);
	}
}

void CSpellWindow::empty_menu(_wintype* window)
{
	_setColor(window, INFO_COLOR);

	if (!casting)
		format_print(window, 1, " %s has no spells left to learn.",
			Party.Player[menu].name.c_str());
	else
		format_print(window, 1, " %s does not know any spells.",
			Party.Player[menu].name.c_str());
}

void CSpellWindow::select()
{
	if (casting)
		menu_move = CLIMB;
	else
	{
		player = &Party.Player[menu];
		CSpell* spell = SpellItem[menu][pos];

		if (spell->JP > player->JP)
		{
			message("%s does not have enough JP to learn %s.",
				player->name.c_str(), spell->name.c_str());
		}
		else
		{
			message(player->name + " learns " + spell->name + ".");
			player->bSpell.insert(spell);
			player->JP -= spell->JP;
			player->cal_stats();
			build_learning_menus();
		}
	}
}

void CSpellWindow::build_learning_menus()
{
	int i = 0;
	for(CPlayer& player : Party.Player)
	{
		Menu[i].clear();
		SpellItem[i].clear();

		for(CAbility* abil : player.Abil)
			if (abil->type == "Set")
				for(CSpell* spell : spellsetmap[abil->name])
					if (!player.bSpell.count(spell))
					{
						Menu[i].push_back(spell->name);
						if (spell->JP > player.JP)
							Menu[i].back().color = DARKGRAY;
						SpellItem[i].push_back(spell);
					}
		++i;
	}
}

action_result_type CCastSpellMap::add()
{
	unsigned long status = ST_NONE;
	while(Status.count(next()))
		status |= statusmap[token];

	if (target->defeated())
		return AR_NONE;

	status &= ~target->status & ~target->cancels;
	
	if (status == ST_NONE)
		return AR_NONE;

	add_message(target->name + " gains " +
		comma_list(status_to_buffer(status)) + ".");
	*target += status;

	return AR_NORMAL;
}

action_result_type CCastSpellMap::remove()
{
	buffer removed;
	unsigned long status = ST_NONE;

	while(Status.count(next()))
		status |= statusmap[token];

	status &= target->status & ~target->always;
	
	if (status == ST_NONE)
		return AR_NONE;

	add_message(target->name + " loses " +
		comma_list(status_to_buffer(status)) + ".");
	*target -= status;

	return AR_NORMAL;
}

action_result_type CCastSpellMap::special(CTarget& Target)
{
	if (next() == "exit")
	{
		warp_outside();
		return AR_NORMAL;
	}
	else if (token == "warp")
	{
		if (Party.L.donjon == 0)
			add_message("The spell has no effect on the overworld.");
		else
		{
			add_message("You feel terribly disoriented.");
			Party.L = Party.map().entrance().Dest;
			draw_map(leftWindow, Party.L);
		}
	}

	return AR_NORMAL;
	return AR_ERROR;
}

action_result_type CCastSpellMap::toggle()
{
	unsigned long gain = ST_NONE, remove = ST_NONE;

	while(Status.count(next()))
		if (*target == statusmap[token])
			remove |= statusmap[token];
		else
			gain |= statusmap[token];

	gain &= ~target->status & ~target->cancels;
	remove &= target->status & ~target->always;

	if (gain == ST_NONE && remove == ST_NONE)
		return AR_NONE;

	add_message(target->name + " gains " +
		comma_list(status_to_buffer(gain)) + ".");
	*target += gain;

	add_message(target->name + " loses " +
		comma_list(status_to_buffer(remove)) + ".");
	*target -= remove;
	
	return AR_NORMAL;
}

bool cast_spell(CPlayer& actor, CSpell* spell)
{
	if (spell->Map == false)
	{
		message(spell->name + " cannot be cast outside of combat.");
		return false;
	}

	if ((spell->Set == "Blue Low" || spell->Set == "Blue High")
		&& !actor.BlueMemory.count(spell))
	{
		message("%s must witness %s being cast before %s can use it %s.",
			actor.name.c_str(), spell->name.c_str(),
			actor.objp().c_str(), actor.refp().c_str());
		return false;
	}

	if (spell->MP > actor.MP)
	{
		message(actor.name + " does not have enough MP to cast " +
			spell->name + ".");
		return false;
	}

	CTarget Target;
	Target.fill();

	if (Target(actor, spell->target, false, spell->AI))
	{
		actor.loseMP(spell->MP);

		if (CCastSpellMap(actor, spell)(Target) == AR_NONE)
			add_message("It has no effect.");

		return true;
	}
	
	return false;
}

void select_spell()
{
	CSpellWindow functor(bigWindow);
	functor.casting = true;
	functor.combat = false;

	int i = 0;
	for(CPlayer& player : Party.Player)
	{
		for(CSpell* spell : player.Spell)
		{
			functor.Menu[i].push_back(color_string(spell->name,
				player.MP + player.tMP >= spell->MP ? LIGHTGRAY : DARKGRAY));
			functor.SpellItem[i].push_back(spell);
		}
		++i;
	}

	std::pair <int, int> choice = functor();

	if (choice.second--)
		cast_spell(Party.Player[choice.first],
			functor.SpellItem[choice.first][choice.second]);

	Party.draw(rightWindow);
}

void learn_spells()
{
	CSpellWindow functor(upperWindow);
	functor.casting = false;
	functor.combat = false;
	functor.build_learning_menus();

	(void)functor();
}

std::deque<CSpell> Spell;
std::map<std::string, CSpell*> spellmap;
std::map<std::string, std::set<CSpell*, sort_serial<CSpell> > > spellsetmap;
std::map<std::string, target_type> targetmap;
