#ifndef __JOBS_H_
#define __JOBS_H_

#include <set>
#include <string>

#include "Input.h"
#include "Misc.h"
#include "Window.h"

class CJob
{
public:
	int serial;

	std::string title, description;
	int Str, Agi, Vit, Int, Luc;
	std::set<CAbility*, sort_serial<CAbility> > Ability;
	std::set<std::string> WeaponSkill;
	std::set<std::string> ArmorSkill;

	void describe(_wintype* window);
};

class CJobWindow : public CSelectWindow
{
public:
	void describe(_wintype*);
	void empty_menu(_wintype*);

	CJobWindow(_wintype* w, bool p = true, int m = 18)
		: CSelectWindow(w, p, true, m, INPUT_START)
	{}
};

extern std::vector<CJob> Job;

#endif
