#include "Jobs.h"
#include "Output.h"

void CJob::describe(_wintype* window)
{
	_werase(window);
	
	if (_getwidth(window) < 51)
	{
		_wputstring(window, 1, 1, ERROR_COLOR, "Error: window not wide \
enough!");
		return;
	}

	_wputstring(window, 1, 1, TITLE_COLOR,
		"Stats  Abilities       Weapons         Armor");

	_wputcstr(window, 3, 1, TEXT_COLOR, "Str %d", Str);
	_wputcstr(window, 4, 1, TEXT_COLOR, "Agi %d", Agi);
	_wputcstr(window, 5, 1, TEXT_COLOR, "Vit %d", Vit);
	_wputcstr(window, 6, 1, TEXT_COLOR, "Int %d", Int);
	_wputcstr(window, 7, 1, TEXT_COLOR, "Luc %d", Luc);

	int i = 0;
	for(CAbility* it : Ability)
	{
		_wputstring(window, 3 + i, 8, TEXT_COLOR, it->name);
		i++;
	}

	i = 0;
	for(const std::string& it : WeaponSkill)
	{
		_wputstring(window, 3 + i, 24, TEXT_COLOR, it);
		i++;
	}

	i = 0;
	for(const std::string& it : ArmorSkill)
	{
		_wputstring(window, 3 + i, 40, TEXT_COLOR, it);

		i++;
	}

	format_print(window, 10, description);
}

void CJobFunctor::describe(_wintype* window)
{
	Job[pos].describe(window);
}

void CJobFunctor::empty_menu(_wintype* window)
{
	_werase(window);
	_wputstring(window, 1, 1, ERROR_COLOR, "Error: no jobs appear to \
be loaded.");

	return;
}
	
std::vector<CJob> Job;
