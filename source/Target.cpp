#include "Actor.h"
#include "Combat.h"
#include "Input.h"
#include "Message.h"
#include "Misc.h"
#include "Party.h"
#include "Player.h"
#include "Random.h"
#include "Target.h"

// Creates a target from a list (of pointers to CActor).
CTarget::CTarget(std::list<CActor*>& Target)
{
	for(CActor* it : Target)
		insert(it);
}

// Creates a target from a list.
CTarget::CTarget(std::vector<CPlayer>& Target)
{
	for(CActor& it : Target)
		insert(&it);
}

// Creates a simple target of one unit.
CTarget::CTarget(CActor* target)
{
	insert(target);
}

void CTarget::fill()
{
	if (Party.battle)
		for(CActor* it : Party.battle->Actor)
			insert(it);
	else
		for(CActor& it : Party.Player)
			insert(&it);
}

bool CTarget::choose_multiple_targets(bool monster, std::vector<CActor*>& Friend,
									  std::vector<CActor*>& Enemy)
{
	prompt("Select any number of targets.  Push A when done.");

	int ch, pos = 0;

	CTarget Cursor;

	do
	{
		if (monster && Enemy.size() == 0)
			monster = !monster;
		else if (!monster && Friend.size() == 0)
			monster = !monster;

		Cursor.clear();
		if (monster)
			Cursor.insert(Enemy[pos]);
		else
			Cursor.insert(Friend[pos]);

		if (Party.battle)
			Party.battle->draw_monsters(Cursor, this);
		Party.draw(rightWindow, Cursor, this);

		switch (ch = _getch())
		{
		case _KEY_UP:
			pos--;
			break;

		case _KEY_DOWN:
			pos++;
			break;

		case _KEY_LEFT:
		case _KEY_RIGHT:
			monster = !monster;
			break;

		case KEY_ACCEPT:
			if (count(*Cursor.begin()))
				erase(*Cursor.begin());
			else
				insert(*Cursor.begin());
			break;

		case 'a':
			ch = 'A';
			break;
			
		case 'C':
			if (Party.battle)
			{
				Party.view_init_cards = !Party.view_init_cards;
				Party.battle->draw();
			}
			break;
		}

		if (monster)
		{
			if (pos > (int)Enemy.size() - 1)
				pos = 0;
			else if (pos < 0)
				pos = Enemy.size() - 1;
		}

		if (!monster)
		{
			if (pos > (int)Friend.size() - 1)
				pos = 0;
			else if (pos < 0)
				pos = Friend.size() - 1;
		}

	} while (!(ch == _KEY_CANCEL || ch == 'A'));

	if (Party.battle)
		Party.battle->draw();
	else
		Party.draw(rightWindow);

	if (empty() || ch == _KEY_CANCEL)
		return false;

	return true;
}

bool CTarget::choose_group_target(bool monster, std::vector<CActor*>& Friend,
								  std::vector<CActor*>& Enemy)
{
	int ch;

	do
	{
		if (monster && Enemy.size() == 0)
			monster = !monster;
		else if (!monster && Friend.size() == 0)
			monster = !monster;

		clear();
		if (monster)
			for(CActor* it : Enemy)
				insert(it);
		else
			for(CActor* it : Friend)
				insert(it);

		if (Party.battle)
			Party.battle->draw_monsters(*this);
		Party.draw(rightWindow, *this);

		switch (ch = _getch())
		{
		case _KEY_LEFT:
		case _KEY_RIGHT:
			monster = !monster;
			break;

		case 'C':
			if (Party.battle)
			{
				Party.view_init_cards = !Party.view_init_cards;
				Party.battle->draw();
			}
			break;
		}
	} while (ch != KEY_ACCEPT && ch != _KEY_CANCEL);

	if (Party.battle)
		Party.battle->draw();
	else
		Party.draw(rightWindow, *this);

	if (ch == _KEY_CANCEL)
		return false;

	return true;
}

bool CTarget::choose_single_target(bool monster, std::vector<CActor*>& Friend,
								   std::vector<CActor*>& Enemy)
{
	int ch, pos = 0;

	do
	{
		if (monster && Enemy.size() == 0)
			monster = !monster;
		else if (!monster && Friend.size() == 0)
			monster = !monster;

		clear();
		if (monster)
			insert(Enemy[pos]);
		else
			insert(Friend[pos]);

		if (Party.battle)
			Party.battle->draw_monsters(*this);
		Party.draw(rightWindow, *this);

		switch (ch = _getch())
		{
		case _KEY_UP:
			pos--;
			break;

		case _KEY_DOWN:
			pos++;
			break;

		case _KEY_LEFT:
		case _KEY_RIGHT:
			monster = !monster;
			break;

		case 'C':
			if (Party.battle)
			{
				Party.view_init_cards = !Party.view_init_cards;
				Party.battle->draw();
			}
			break;
		}

		if (monster)
		{
			if (pos > (int)Enemy.size() - 1)
				pos = 0;
			else if (pos < 0)
				pos = Enemy.size() - 1;
		}

		if (!monster)
		{
			if (pos > (int)Friend.size() - 1)
				pos = 0;
			else if (pos < 0)
				pos = Friend.size() - 1;
		}

	} while (ch != KEY_ACCEPT && ch != _KEY_CANCEL);

	if (Party.battle)
		Party.battle->draw();
	else
		Party.draw(rightWindow);

	if (ch == _KEY_CANCEL)
		return false;

	return true;
}

bool CTarget::operator()(CActor& actor, target_type type, bool automatic,
						 AI_type AI)
{
	std::vector<CActor*> Friend, Enemy, KO_Friend, All;

	bool enemy_is_player = (actor == CF_ON_PLAYER_SIDE)
		== (actor == ST_CHARM);
	bool can_target_KO = actor.is_player() && !automatic;

	if (actor == ST_CONFUSE)
		if (type == TRGT_GROUP)
			enemy_is_player = (random(2) == 1);
		else if (type == TRGT_ANY || type == TRGT_SINGLE
			|| type == TRGT_OTHER)
			type = TRGT_RANDOM;

	if (automatic && type == TRGT_ANY)
		type = TRGT_SINGLE;

	All.clear();
	Friend.clear();
	Enemy.clear();

	if (Party.battle)
	{
		for(CActor* it : *this)
		{
			if ((!it->defeated() || (can_target_KO && it->is_player())) &&
				(type != TRGT_OTHER || it != &actor) && *it == CF_TARGETABLE)
			{
				All.push_back(it);

				if (automatic)
					if (enemy_is_player ^ (*it == CF_ON_PLAYER_SIDE))
						Friend.push_back(it);
					else
						Enemy.push_back(it);
				else
					if (it->is_player())
						Friend.push_back(it);
					else
						Enemy.push_back(it);
			}
			if (AI == AI_KOALLY && *it == ST_KO)
				if (enemy_is_player ^ (*it == CF_ON_PLAYER_SIDE))
					KO_Friend.push_back(it);
		}
	}
	else
	{
		for(CActor* it : *this)
		{
			All.push_back(it);
			Friend.push_back(it);
		}
	}

	clear();

	switch (type)
	{
	    case TRGT_NONE:
	    break;

	case TRGT_ANY:
		if (!choose_multiple_targets(AI == AI_ENEMY ? true : false, Friend, Enemy))
			return false;
		break;

	case TRGT_OTHER:
	case TRGT_SINGLE:
		if (automatic)
		{
			if (AI == AI_ENEMY)
				if (Enemy.size() == 0)
					return false;
				else
					insert(Enemy[random(Enemy.size())]);
			else if (AI == AI_ALLY)
				if (Friend.size() == 0)
					return false;
				else
					insert(Friend[random(Friend.size())]);
			else
				if (KO_Friend.size() == 0)
					return false;
				else
					insert(KO_Friend[random(KO_Friend.size())]);
		}
		else if (!choose_single_target(AI == AI_ENEMY ? true : false,
			Friend, Enemy))
			return false;

		break;

	case TRGT_GROUP:
		if (automatic)
		{
			if (AI == AI_ENEMY)
				if (Enemy.size() == 0)
					return false;
				else
					for(CActor* it : Enemy)
						insert(it);
			else if (AI == AI_ALLY)
				if (Friend.size() == 0)
					return false;
				else
					for(CActor* it : Friend)
						insert(it);
			else
				if (KO_Friend.size() == 0)
					return false;
				else
					for(CActor* it : KO_Friend)
						insert(it);
		}
		else if (!choose_group_target(AI == AI_ENEMY ? true : false, Friend, Enemy))
			return false;

		break;

	case TRGT_SELF:
		insert(&actor);
		break;

	case TRGT_RANDOM:
		if (All.empty())
			return false;
		insert(All[random(All.size())]);
		break;

	case TRGT_RANDOM_GROUP:
		if (All.empty())
			return false;
		if (random(2))
			for(CActor* it : Enemy)
				insert(it);
		else
			for(CActor* it : Friend)
				insert(it);
		break;

    case TRGT_ALL:
        for(CActor* it : All)
            insert(it);
		break;
	}

	return true;
}
