#include <deque>

#include "Armor.h"
#include "Output.h"
#include "Party.h"
#include "Spells.h"

void CArmor::describe(_wintype* window, int y)
{
	if (name == "None")
		return;

	std::string slot;
	switch(type)
	{
	case ITEM_BODY:
		slot = "Body";
		break;

	case ITEM_HEAD:
		slot = "Head";
		break;

	case ITEM_ACCESSORY:
		slot = "Accessory";
		break;

	default:
		slot = "Error";
	}

	_wputstring(window, y, 1, TITLE_COLOR, "Price:");
	_wputcstr(window, y, 11, LIGHTGRAY, "%d", price);
	_wputstring(window, y, 26, TITLE_COLOR, "Slot:");
	_wputstring(window, y, 37, LIGHTGRAY, slot);

	_wputstring(window, y + 1, 1, TITLE_COLOR, "Set:");
	_wputstring(window, y + 1, 11, LIGHTGRAY, Set->name);

	if (weight)
	{
		_wputstring(window, y + 1, 26, TITLE_COLOR, "Weight:");
		_wputcstr(window, y + 1, 37, LIGHTGRAY, "%d", weight);
	}

	y += 3;

	y = format_print(window, y, "Max HP: +%d   Max MP: +%d", HP, MP);

	if (always)
		y = format_print(window, y,
			"You always have the status %s.",
			comma_list(status_to_buffer(always)).c_str());

	if (cancels)
		y = format_print(window, y,
			"Cancels %s.",
			comma_list(status_to_buffer(cancels)).c_str());

	if (grantsS)
		y = format_print(window, y, "Allows the wearer to cast %s.",
			grantsS->name.c_str());

	if (immune)
		y = format_print(window, y, "Provides immunity to %s.",
			comma_list(element_to_buffer(immune)).c_str());

	if (resists)
	y = format_print(window, y, "Provides resistance to %s.",
		comma_list(element_to_buffer(resists)).c_str());

	if (seals)
		y = format_print(window, y,
			"While wearing the %s, you cannot use %s.", name.c_str(),
			seals->name.c_str());

	if (starts)
		y = format_print(window, y,
			"You begin every battle with the status %s.",
			comma_list(status_to_buffer(starts)).c_str());

	if (strengthens)
		y = format_print(window, y,
			"Increases your %s damage.",
			comma_list(element_to_buffer(strengthens)).c_str());

	if (name == "Crown")
		y = format_print(window, y,
	"While wearing the Crown, you recover twice as many HP from healing.");
}

std::deque<CArmor> Armor;
