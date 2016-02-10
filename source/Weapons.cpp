#include <deque>
#include <string>

//#include "Output.h"
#include "Party.h"
#include "Spells.h"
#include "Weapons.h"

void CWeapon::describe(_wintype* window, int y)
{
	if (name == "None")
		return;

	_wputstring(window, y, 1, TITLE_COLOR, "Price:");
	_wputcstr(window, y, 11, LIGHTGRAY, "%d", price);
	_wputstring(window, y, 26, TITLE_COLOR, "Hands:");
	_wputstring(window, y, 37, LIGHTGRAY, twoHanded ? "Two" : "One" );

	_wputstring(window, y + 1, 1, TITLE_COLOR, "Set:");
	_wputstring(window, y + 1, 11, LIGHTGRAY, Set->name);

	if (weight)
	{
		_wputstring(window, y + 1, 26, TITLE_COLOR, "Weight:");
		_wputcstr(window, y + 1, 37, LIGHTGRAY, "%d", weight);
	}

	y += 3;

	y = format_print(window, y, "Attack: +%d   Hit: +%d", Atk, Hit);

	if (Set->name == "Ammunition")
		y = format_print(window, y,
"You can carry a number of extra Ammunitions equal to your Strength. \
To throw this, equip it and attack the enemy.");

	if (range > 1 && Set->name != "Ammunition")
		y = format_print(window, y,
			"Deals full damage from the back row.");

		if (Set->name == "Gun")
		y = format_print(window, y,
			"Your strength does not impact the damage dealt.");
	if (adds)
		y = format_print(window, y, "Adds %s to the foe.",
			comma_list(status_to_buffer(adds)).c_str());

	if (always)
		y = format_print(window, y,
			"You always have the status %s.",
			comma_list(status_to_buffer(always)).c_str());

	if (cancels)
		y = format_print(window, y,
			"Cancels %s.",
			comma_list(status_to_buffer(cancels)).c_str());

	if (casts)
		if (Set->name == "Ammunition")
			y = format_print(window, y, "Casts %s.", casts->name.c_str());
		else
			y = format_print(window, y, "May cast %s upon wounding the foe.",
				casts->name.c_str());

	if (element)
	{
		buffer buf = element_to_buffer(element);
		std::string ess = buf.size() == 1 ? "" : "s";
		y = format_print(window, y, "It is imbued with the element%s %s.",
			ess.c_str(), comma_list(buf).c_str());
	}

	if (grantsA)
		y = format_print(window, y, "Grants the ability %s.",
			grantsA->name.c_str());

	if (grantsS)
		y = format_print(window, y, "Can be invoked to cast %s.",
			grantsS->name.c_str());

	if (hurts)
		y = format_print(window, y, "Does extra damage to %s.",
			comma_list(race_to_buffer(hurts)).c_str());

	if (starts)
		y = format_print(window, y,
			"You begin battle with the status %s.",
			comma_list(status_to_buffer(starts)).c_str());

	if (strengthens)
		y = format_print(window, y,
			"Your %s attacks deal extra damage.",
			comma_list(element_to_buffer(strengthens)).c_str());
}

std::deque<CWeapon> Weapon;
