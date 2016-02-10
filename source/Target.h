#ifndef __TARGET_H_
#define __TARGET_H_

#include <map>
#include <set>
#include <vector>

#include "Misc.h"
#include "Party.h"

class CActor;
class CBattle;

class CTarget : public std::set<CActor*, sort_serial<CActor> >
{
public:
	CTarget()
	{}

	// Creates a target from a list.
	CTarget(std::list<CActor*>&);
	CTarget(std::vector<CPlayer>&);

	// Creates a simple target of one unit.
	CTarget(CActor* target);

	// fills the set with all possible targets.  In battle this
	// means all combatants.  Outside of battle it means the party.
	void fill();

	// To have a unit choose its targets, fill the set up with
	// legitimate targets.  Then call the function below.  The
	// resulting set will contain the targets chosen by the unit.
	bool operator()(CActor&, target_type, bool automatic = false,
		AI_type AI = AI_ENEMY);

private:
	bool choose_multiple_targets(bool, std::vector<CActor*>& Friend,
		std::vector<CActor*>& Enemy);
	bool choose_group_target(bool, std::vector<CActor*>& Friend,
		std::vector<CActor*>& Enemy);
	bool choose_single_target(bool, std::vector<CActor*>& Friend,
		std::vector<CActor*>& Enemy);
};

#endif