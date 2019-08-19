#include <list>
#include <set>

#include "Action.h"
#include "Actor.h"
#include "Combat.h"
#include "Input.h"
#include "Message.h"
#include "Misc.h"
#include "Monsters.h"
#include "Party.h"
#include "Player.h"
#include "Random.h"
#include "Spells.h"
#include "Target.h"
#include "Town.h"
#include "Weapons.h"
#include "Window.h"

std::string CAction::next()
{
	return token = ns(effect);
}

CActionAttack::CActionAttack(CActor& a, buffer e) :
	CAction(a, e), attack_adds(ST_NONE), attack_hurts(RACE_NONE),
	attack_element(ELEM_NONE), ignore_evasion(false), ignore_row(false),
	jumping(false),	throwing(false), attack_drainHP(false),
	attack_drainMP(false), meatbone_slashing(false), modifier(0),
	num_attacks(1), attack_weapon((CWeapon*)noneItem[ITEM_WEAPON])
{}

action_result_type CActionAttack::attack(CActor& target)
{
	bool mult_includes_base = false;
	int base_damage, mult = modifier;
	std::string msg = attack_msg;
	item_type equip_slot = ITEM_NONE;
	
	if (target != CF_TARGETABLE)
		return AR_NONE;

	int final_Hit = adjusted_Hit;
	int final_Atk = adjusted_Atk;
	if (actor == CF_CHARGING)
	{
		actor -= CF_CHARGING;
		mult++;
		final_Atk++;
	}

	if (target == ST_BLINK)
	{
		more(msg + " misses.");
		if (--target.blink_num == 0)
			Party.battle->lose_status(target, ST_BLINK);
		return AR_NORMAL;
	}

	if (jumping && attack_weapon->Set->name == "Spear")
	{
		mult++;
		final_Hit++;
	}

	// Draw attack and defense cards.
	CDeal attackCards(actor == ST_BLIND ? dv_percentage(final_Hit, 2) :
		final_Hit);
	CDeal targetCards(actor == ST_LOCK ? dv_percentage(final_Hit, 2) :
		adjusted_Eva);

	// Check to see if the attack hits.
	if (meatbone_slashing && attackCards.contains_king())
		mult++;
	else if (actor == CF_AIMING)
		actor -= CF_AIMING;
	else if (attackCards < targetCards && !(ignore_evasion || throwing ||
		target == ST_SLEEP || target == ST_STOP))
	{
		more(msg + " misses.");
		return AR_NORMAL;
	}

	unsigned char hurts = attack_hurts | (throwing ? attack_weapon->hurts :
		actor.hurts);
	if (hurts && !target.is_player())
		if (target.race()->races & hurts)
			mult++;

	int distance = 1;

	if (target == CF_DEFENDING)
		mult--;
	if (target == ST_PROTECT)
		mult--;
	if (target == ST_FROG)
		mult++;

	if (!ignore_row)
	{
		if (target == CF_BACK_ROW)
			distance++;
		if (actor == CF_BACK_ROW)
			distance++;
	}

	if (actor.is_player())
	{
		CPlayer& player = (CPlayer&)actor;

		distance -= ((CWeapon*)player.Equip[ITEM_WEAPON])->range;

		if (player.has_ability("Double Grip") &&
			player.Equip[ITEM_ACCESSORY]->name == "None"
			&& ((CWeapon*)player.Equip[ITEM_WEAPON])->twoHanded == false
			&& player.Equip[ITEM_WEAPON]->name != "None")
			mult++;
	}
	else
		distance--;

	if (distance > 0)
		mult -= (distance);

	if (throwing)
		base_damage = attack_weapon->Atk * attack_weapon->Hit;
	else
		base_damage = final_Atk;

	if (attack_weapon->Set->name == "Gun")
	{
		mult += 3;
		mult_includes_base = true;
	}

	if (actor == ST_FROG)
	{
		mult = 0;
		mult_includes_base = true;
	}

	bool drainHP = attack_drainHP || attack_weapon->drainHP;
	bool drainMP = attack_drainMP || attack_weapon->drainMP;
	
	if (drainHP)
		if (target == ST_UNDEAD)
			msg += " hits." + target.name + " drains %d HP from " + actor.name + "!";
		else
			msg += " hits, draining %d HP.";
	else
		msg += " hits for %d damage.";

	// hack to prevent Dragon Spirit from giving someone reraise
	// after he's been revived.
	bool died = false;

	int damage_taken = 0;
	
	if (!throwing || attack_weapon->Atk > 0)
	{
		unsigned char element = attack_element;

		if (throwing)
		{
			msg = actor.name + " throws the " + attack_weapon->name + " at " +
				target.name + ", dealing %d damage.";
			element |= attack_weapon->element;
		}
		else
			element |= actor.element;

		if (jumping)
			element |= ELEM_AIR;

		int curHP = target.curHP();
		damage_taken = Party.battle->damage(&actor, target, base_damage,
			element, mult, msg, mult_includes_base, drainHP, drainMP);
		if (damage_taken >= curHP)
			died = true;
	}
	else
		more("%s uses the %s.", actor.name.c_str(),
			attack_weapon->name.c_str());

	unsigned long status_gain = ST_NONE;
	bool can_jump = false;

	if (!target.defeated() && damage_taken)
	{
		if (target.control && !target.is_player())
			target.control = target.control->control = NULL;

		target -= ST_CONFUSE | ST_SLEEP | ST_CHARM;
		
		CPlayer& ref = (CPlayer&)target;
			
		if (ref.has_ability("Dragon Spirit") && !died) {
			if (ref == ST_RERAISE)
				can_jump = true;
			else
				status_gain |= ST_RERAISE;
		}

		if (ref.has_ability("Sunken State") &&
			CCard().is_black())
			status_gain |= ST_BLINK;
		
		if (actor.has_ability("Train") && !target.is_player() &&
			(int)((CMonster&)target).HPCards.size() <= dv(target.eVit(), 4)
			&& CCard().is_black())
			status_gain |= ST_INVITE;

		if (target.race()->name == "Chaos")
			((CMonster&)target).change_form();
	}

	CCard trigger_card;

	if (!target.defeated())
	{
		if (throwing)
			status_gain |= attack_weapon->adds;
		else
		{
			unsigned long status = actor.adds | attack_adds;
			if (status && (trigger_card.suit == SUIT_SPADE ||
			(trigger_card.is_black() &&	attack_weapon->Set->name == "Blade")))
				status_gain |= status;
		}
	
		Party.battle->gain_status(target, status_gain);

		if (target != ST_INCAPACITATING)
		{
			if (target.has_ability("Counter") && CCard().is_black())
			{
				CTarget tgt(&actor);
				buffer buf;
				buf.push_back("counter");
				buf.push_back("eoe");
				(void)CActionAttack(target, buf)(tgt);
			}

			if (can_jump)
			{
				message(
				"The spirit of the dragon comes over %s. Have %s jump?",
					target.name.c_str(), target.objp().c_str());

				if (yesno())
				{
					target += CF_JUMPING;
					target -= CF_TARGETABLE;
					target -= ST_RERAISE;

					message("%s leaps into the air.",
						target.name.c_str());
				}
			}
		}
	}

	CSpell* spell = attack_weapon->casts;
	
	if (spell && actor != ST_INCAPACITATING && actor == CF_TARGETABLE)
	{
		unsigned char to_beat_rank = 9;
		if (attack_weapon->Set->name == "Rod")
			to_beat_rank = 6;

		if (throwing || trigger_card.rank > to_beat_rank)
		{
			CTarget tgt;
			tgt.fill();
				
			bool target_success = false;

			if (spell->target == TRGT_SINGLE ||
				spell->target == TRGT_ANY)
			{
				tgt = CTarget(&target);
				if (!target.defeated())
					target_success = true;
			}
			else
			{
				target_success = (tgt)(actor, spell->target, true,
					target == CF_ON_PLAYER_SIDE ? AI_ALLY : AI_ENEMY);
			}

			if (target_success)
			{
				action_result_type result =
					CActionCastSpellBattle(actor, spell)(tgt);
				if (result != AR_NONE)
					return result;
				else
					message("It has no effect.");
			}
		}
	}

	return AR_NORMAL;
}

void CActionAttack::parse()
{
	attack_msg = actor.name;

	if (next() == "counter")
		attack_msg += " counterattacks ";
	else if (token == "jump")
		attack_msg += " descends upon ";
	else
		attack_msg += " attacks ";
	attack_msg += (&actor == target ? actor.refp() : target->name) + " and";

	adjusted_Hit = actor.eHit();
	adjusted_Atk = actor.eAtk();
	adjusted_Eva = target->eEva();
	
	if (actor.has_ability("Meatbone Slash"))
		meatbone_slashing = true;

	while (token != "eoe")
	{
		if (token == "add")
		{
			while(Status.count(next()))
				attack_adds |= statusmap[token];
			if (token == "eoe")
				break;
		}
		else if (token == "berserk")
			modifier++;
		else if (token == "d4_times")
			num_attacks = CCard().suit + 1;
		else if (token == "drainHP")
			attack_drainHP = true;
		else if (token == "drainMP")
			attack_drainMP = true;
		else if (Element.count(token))
			attack_element |= elementmap[token];
		else if (token == "hurts")
		{
			while (racemap.count(next()))
				attack_hurts |= racemap[token];
			if (token == "eoe")
				break;
		}
		else if (token == "ignore_evasion")
			ignore_evasion = true;
		else if (token == "ignore_row")
			ignore_row = true;
		else if (token == "jump")
		{
			ignore_row = true;
			jumping = true;
		}
		else if (token == "meatbone_slash")
			modifier++;
		else if (token == "modifier_minus_one")
			modifier--;
		else if (token == "rapid")
		{
			adjusted_Hit--;
			modifier--;
		}
		else if (token == "throw")
		{
			throwing = true;
			attack_weapon = (CWeapon*)itemmap[next()];
		}
		else if (token == "twice")
			num_attacks = 2;
		else
			token = "eoe";

		next();
	}
}

void reequip_thrown_weapon(CPlayer& player, item_type slot)
{
	CItemWindow functor(
		upperWindow, ITEM_WEAPON, ITEM_MODE_SELECT, false);

	functor.player = &player;

	if (functor.build_menus())
	{
		prompt(" Choose a new weapon to equip.");

		bool still_deciding = true;
		
		while (still_deciding)
		{
			int choice = functor().second;

			if (choice)
			{
				unsigned long always = player.always;

				if (player.equip(functor.Item[0][--choice], slot))
				{
					Party.battle->lose_status(player,
						(always & ~player.always) | player.cancels);
					Party.battle->gain_status(player,
						(player.always & ~always) & ~player.cancels);
					player.cal_stats();
					still_deciding = false;
				}
			}
			else
				still_deciding = false;
		}
	}
}

action_result_type CActionAttack::operator()(CTarget& Target)
{
	target = *Target.begin();

	if (target->defeated())
		return AR_NONE;

	if (*target == CF_ON_PLAYER_SIDE)
	{
		std::vector<CPlayer*> targets;
//		CTarget targets;
//		buffer target_names;
		for(CPlayer& it : Party.Player)
			if (it.has_ability("Cover") && &it != target
				&& it != (ST_INCAPACITATING | ST_DISTRACTING) &&
				it == CF_ON_PLAYER_SIDE && it == CF_TARGETABLE &&
				((CCard().is_black() && target->curHP() < 
				actor.curHP())|| target->curHP() <
				dv(((CPlayer*)target)->MaxHP, 4)))
			{
				targets.push_back(&it);
//				targets.insert(&it);
//				target_names.push_back(it.name);
			}
		if (!targets.empty())
		{
			CPlayer* new_target = targets[random(targets.size())];
			message("%s covers %s.", new_target->name.c_str(),
				target->name.c_str());
			target = new_target;
/*			message(
"%s is attacking %s, but %s can cover %s.  Choose who will be attacked.",
actor.name.c_str(), target->name.c_str(), comma_list(target_names).c_str(),
target->objp().c_str());
			targets.insert(target);
			targets(*target, TRGT_SINGLE);
			target = *targets.begin();*/
		}
	}

	parse();

	action_result_type result = AR_NONE, subresult = AR_NONE;
	
	while( num_attacks-- && !target->defeated()
		&& !(actor == ST_INCAPACITATING))
	{
		if (actor.is_player() && !throwing)
			attack_weapon = (CWeapon*)((CPlayer&)actor).Equip[ITEM_WEAPON];

		if (!throwing && attack_weapon->Set->name == "Ammunition")
		{
			CPlayer& player = (CPlayer&)actor;
			throwing = true;
			player.Equip[ITEM_WEAPON] = noneItem[ITEM_WEAPON];
			player.cal_stats(false);
			subresult = attack(*target);
			throwing = false;

			reequip_thrown_weapon(player, ITEM_WEAPON);
		}
		else
			subresult = attack(*target);

		if ((subresult == AR_FLED) || (subresult == AR_ERROR))
			return subresult;
		else if (subresult == AR_NORMAL)
			result = subresult;

		if (actor.is_player() && !target->defeated()
			&& !(actor == ST_INCAPACITATING))
		{
			CPlayer& player = (CPlayer&)actor;
			if (player.Equip[ITEM_ACCESSORY]->type == ITEM_WEAPON &&
				!throwing && player.Equip[ITEM_WEAPON]->name != "None")
			{
				attack_weapon = (CWeapon*)player.Equip[ITEM_ACCESSORY];

				if (attack_weapon->Set->name == "Ammunition")
				{
					CPlayer& player = (CPlayer&)actor;
					throwing = true;
					player.Equip[ITEM_ACCESSORY] = noneItem[ITEM_ACCESSORY];
					player.cal_stats(false);
					subresult = attack(*target);
					throwing = false;

					reequip_thrown_weapon(player, ITEM_ACCESSORY);
				}
				else
				{
					player.cal_stats(false, NULL, ITEM_ACCESSORY);
					subresult = attack(*target);
				}

				if ((subresult == AR_FLED) || (subresult == AR_ERROR))
					return subresult;
				else if (subresult == AR_NORMAL)
					result = subresult;
			}
		}
	}

	return result;
}

action_result_type CActionDefend::operator()()
{
	actor += CF_DEFENDING;
	message("%s assumes a defensive posture.", actor.name.c_str());
	return AR_NORMAL;
}

action_result_type CActionEquip::operator()()
{
	CPlayer& ref = dynamic_cast<CPlayer&>(actor);

	CEquipWindow functor(upperWindow, ITEM_WEAPON, ITEM_MODE_EQUIP, false);

	functor.player = &ref;
	functor.build_menus();

	unsigned long always = ref.always;

	(void)functor();

	Party.battle->lose_status(actor, (always & ~ref.always) | ref.cancels);
	Party.battle->gain_status(actor, (ref.always & ~always) & ~ref.cancels);

	if (functor.equipped_something)
		return AR_NORMAL;
	else
		return AR_NONE;
}

action_result_type CActionFlee::operator()()
{
	bool flee = false;

	if (actor == ST_QUICK)
		flee = true;
	else
	{
		int agi_enemy = 0;
		for(CActor* it : Party.battle->Actor)
			if ((it->cflag & CF_ON_PLAYER_SIDE) != (actor.cflag &
				CF_ON_PLAYER_SIDE) && it->eAgi() > agi_enemy)
				agi_enemy = it->eAgi();
		if (CDeal(agi_enemy) < CDeal(actor.eAgi()))
			flee = true;
	}

	if (flee)
	{
		message("Close Call...");
		Party.battle->the_battle_rages = false;
		return AR_FLED;
	}

	more("%s fails to escape.", actor.name.c_str());

	return AR_NORMAL;
}

action_result_type CActionChangeRow::operator()()
{
	actor ^= CF_BACK_ROW;
	if (actor == CF_BACK_ROW)
		message("%s moves to the back row.", actor.name.c_str());
	else
		message("%s moves to the front row.", actor.name.c_str());
	return AR_NORMAL;
}

bool CActionCommand::operator()(CTarget& Target)
{
	if (ability->name == "Aim")
	{
		actor += CF_AIMING;
		message("%s aims carefully at the foe.", actor.name.c_str());
	}
	else if (ability->name == "Control")
	{
		target = *Target.begin();
		if (target->is_player())
		{
			message("%s is already under your control.",
				target->name.c_str());
			return false;
		}

		if (target->race()->boss)
		{
			message("%s will never submit %s will to another.",
				target->name.c_str(), target->posp().c_str());
			return false;
		}

		if (*target & (ST_INCAPACITATING | ST_DISTRACTING))
		{
			more("%s is not in a state to be controlled.",
				target->name.c_str());
			return false;
		}

		if (CCard().is_black())
		{
			more("%s compels %s to do %s bidding.",	actor.name.c_str(),
				target->name.c_str(), actor.posp().c_str());
			actor.control = target;
			if (target->control)
				target->control->control = NULL;
			target->control = &actor;
		}
		else
			more("%s fails to control %s.",
				actor.name.c_str(), target->name.c_str());
	}
	else if (ability->name == "Dark Wave")
	{
		message("%s unleashes dark energy.", actor.name.c_str());
		for(CActor* it : Party.battle->Actor)
			if (((*it == CF_ON_PLAYER_SIDE) != (actor == CF_ON_PLAYER_SIDE)) &&
				!it->defeated())
			{
				Party.battle->damage(&actor, *it, actor.Atk, ELEM_DARK, -1
					+ actor.focus, std::string(it->name + " takes %d damage.")
					.c_str());

				if (it->race()->name == "Chaos")
				{
					CSpell* spell = spellmap["Debilitator"];
					
					CTarget target;
					target.fill();
					
					if ((target)(actor, spell->target, true, spell->AI))
						if (CActionCastSpellBattle(actor, spell)(target)
							== AR_NONE)
						add_message("It has no effect.");
				}
			}
		
		int loss = dv_percentage(((CPlayer&)actor).MaxHP, 10);
		message("%s loses %d HP.", actor.name.c_str(), loss);
		Party.battle->loseHP(actor, loss);
	}
	else if (ability->name == "Focus")
	{
		actor.focus++;
		message(actor.name + " channels mystical energy.");
	}
	else if (ability->name == "Jump")
	{
		actor += CF_JUMPING;
		actor -= CF_TARGETABLE;
		message("%s leaps into the air.", actor.name.c_str());
	}
	else if (ability->name == "Pray")
	{
		message("%s begs the LORD for mercy.", actor.name.c_str());
		CCard card;
		bool answered = false;

		if (card.rank > 9)
		{
			for(CPlayer& player : Party.Player)
			{
				buffer buf = status_to_buffer(player.status);
				
				if (buf.size())
				{
					answered = true;

					message("Choose a status for %s to lose.",
						player.name.c_str());

					buf.push_front("None");

					int select = menu_select(12, 5, buf);

					if (--select)
					{
						buffer::iterator it = buf.begin();
						for (int i = 0; i < select;i++)
							it++;

						Party.battle->lose_status(player,
							statusmap[*it]);
					}
				}
			}
		}

		if (card.is_black())
			for(CPlayer& player : Party.Player)
				if (Party.battle->heal(player,
					dv_percentage(player.MaxHP, 10)))
					answered = true;

		if (!answered)
			add_message("His prayer is unanswered.");
	}
	else if (ability->name == "Rage")
	{
		Party.battle->gain_status(actor, ST_BERSERK);

		buffer buf;
		buf.push_back("eoe");

		if (!CActionAttack(actor, buf)(Target))
			more(actor.name + " fails to attack " +
				(*Target.begin())->name + ".");
	}
	else if (ability->name == "Rapid Fire")
	{
		effect.push_back("rapid");
		effect.push_back("eoe");

		bool target_failure = true;

		for(int i = 0; i < 4; i++)
		{
			CTarget attack_target;
			attack_target.fill();

			if (attack_target(actor, TRGT_SINGLE, true))
			{
				target_failure = false;
				(void)CActionAttack(actor, effect)(attack_target);
			}
		}

		if (target_failure)
			message(actor.name + " fails to hit anyone.");
	}
	else if (ability->name == "Throw")
	{
		CItem* item = NULL;
		target = *Target.begin();

		CPlayer& player = (CPlayer&)actor;

		CItemWindow functor(upperWindow, ITEM_WEAPON, ITEM_MODE_SELECT, false);

		functor.player = &player;
		if (!functor.build_menus(true))
		{
			message("%s has nothing to throw at %s.", actor.name.c_str(),
				target->name.c_str());
			return false;
		}

		prompt("  Choose a weapon to throw at " + target->name + ".");

		if (int choice = functor().second)
			item = functor.Item[0][--choice];
		else
			return false;

		if (((CWeapon*)item)->twoHanded)
		{
			message("The %s is two large to throw at your foes.",
				item->name.c_str());
			return false;
		}

		buffer buf;

		buf.push_back("throw");
		buf.push_back(item->name);
		buf.push_back("eoe");

		CActionAttack CAA(actor, buf);
		CTarget CT(target);
		(void)CAA(CT);

		//(void)CActionAttack(actor, buf)(CTarget(target));

		item_type slot = player.remove_item(item);

		if (slot != ITEM_NONE)
			reequip_thrown_weapon(player, slot);
	}
	else
		more("%s has yet to be implemented.", ability->name.c_str());

	return true;
}

action_result_type CActionItem::operator()()
{
	CPlayer& ref = dynamic_cast<CPlayer&>(actor);

	CItemWindow functor(upperWindow, ITEM_ITEM, ITEM_MODE_SELECT, false);

	functor.player = &ref;
	functor.build_menus();

	int item = functor().second;
	
	if (!item)
		return AR_NONE;

	if (ref.use(functor.Item[0][--item]))
	{
		ref.remove_item(functor.Item[0][item]);
		return AR_NORMAL;
	}

	return AR_NONE;
}

void CActionCastSpell::warp_outside()
{
	std::list<CLocation> tmp = Party.SaveSpots;
	std::list<CLocation> Towns;

	for(CLocation& it : tmp)
		if (it.tile().Feature->type == FEA_TOWN)
			Towns.push_back(it);
	
	std::list<CLocation>::iterator Loc = Towns.end();
	Loc--;
	
	int ch;
	cbox(upperWindow, BORDER_COLOR);
	show_window(upperWindow);

	_wintype* mapWindow = _derwin(upperWindow,
		_getheight(upperWindow) - 2, _getwidth(upperWindow) - 2, 1, 1);

	add_message("Use the up and down arrow keys to choose a town.");
	do
	{
		draw_map(mapWindow, *Loc, true);
		ch = _getch();

		if (ch == _KEY_UP)
			if (Loc != Towns.begin())
				--Loc;

		if (ch == _KEY_DOWN)
		{
			++Loc;
			if (Loc == Towns.end())
				--Loc;
		}
	
	} while (ch != KEY_ACCEPT);

	Party.L = *Loc;

	message("You appear before the gates of %s.",
		dynamic_cast<CTown*>(Party.tile().Feature)->name.c_str());

	draw_explore_screen();
}

/*
action_result_type CActionCastSpell::absorb()
{
	unsigned char element = ELEM_NONE;
	while(Element.count(next()))
		element |= elementmap[token];

	if (target->defeated())
		return AR_NONE;
		
	element &= ~target->absorbs;

	if (element)
	{
		add_message("%s now absorbs %s attacks.", target->name.c_str(),
			comma_list(element_to_buffer(element)).c_str());
		if (target->is_player())
		{
			((CPlayer*)target)->tabsorbs |= element;
			((CPlayer*)target)->cal_stats();
		}
		else
			target->absorbs |= element;
		return AR_NORMAL;
	}

	return AR_NONE;
}*/

CTarget CActionCastSpell::chance(CTarget Target)
{

	CTarget Passed, Candidate;
	int divisor, addendum;

	if (next() == "min")
		divisor = 6, addendum = 0;
	else if (token == "low")
		divisor = 5, addendum = 1;
	else if (token == "med")
		divisor = 4, addendum = 2;
	else if (token == "high")
		divisor = 3, addendum = 3;
	else if (token == "max")
		divisor = 2, addendum = 4;
	else
	{
		message("Error: chance token %s not understood.", token.c_str());
		return CTarget();
	}

	int chance_cards = dv(actor.eInt(), divisor) + addendum;

	if (spell->target == TRGT_ANY)
	{
		for(CActor* it : Target)
			if (!it->defeated())
				Candidate.insert(it);
	
		while (chance_cards > 0 && !Candidate.empty())
		{
			for(CActor* it : Candidate)
				if (chance_cards-- > 0 && CCard().rank > 9)
					Passed.insert(it);

			for(CActor* it : Passed)
				Candidate.erase(it);
		}
	}
	else
	{
		Passed = Target;
		for(CActor* it : Target)
		{
			int i = chance_cards;
			while (--i > 0 && CCard().rank <= 9)
				if (i == 0) Passed.erase(it);
		}
	}
	return Passed;
}

action_result_type CActionCastSpell::condition(CActor* target)
{
	std::string condition = next();
	if (Status.count(condition))
		if (*target == statusmap[token])
			return AR_NONE;
		else
			return AR_COND_FAIL;
	else
		if (condition == "monster" && !target->is_player())
			return AR_NONE;
		else
			return AR_COND_FAIL;
	
	message("Error: strange token " + token);
	return AR_ERROR;
}

action_result_type CActionCastSpell::damage(CTarget Target)
{
	action_result_type result = AR_NONE;
	int amt;
	next();

	int dividend, addendum;

	if (token == "min")
		dividend = 6, addendum = 1;
	//amt = CDeal(1).total() + dv(actor.eInt(), 2);
	else if (token == "low")
		dividend = 5, addendum = 4;
	//amt = CDeal(3).total() + actor.eInt();
	else if (token == "med")
		dividend = 4, addendum = 8;
	//amt = CDeal(6).total() + dv(3 * actor.eInt(), 2);
	else if (token == "high")
		dividend = 3, addendum = 12;
	//amt = CDeal(10).total() + 2 * actor.eInt();
	else if (token == "max")
		dividend = 2, addendum = 16;
	//amt = CDeal(15).total() + dv(5 * actor.eInt(), 2);
	else
		return AR_ERROR;

	amt = dv(actor.eInt(), dividend) + addendum;

	next();

	if (!elementmap.count(token))
	{
		token = "notelement";
		return AR_ERROR;
	}

	unsigned char element = elementmap[token];

	next();

	int dam;

	CTarget Alive;

	for(CActor* target : Target)
		if (!reflectmap[target]->defeated())
			Alive.insert(reflectmap[target]);

	if (Alive.size() == 0)
		return AR_NONE;

	amt = dv(amt, Alive.size());

	for(CActor* target : Alive)
	{
		int mult = 0;

		if (*target == ST_SHELL)
			mult--;

/*		if (drainHP && *target == ST_UNDEAD)
		{
			dam = Party.battle->damage(target, actor, amt, element, mult,
				std::string(target->name + " loses %d HP.").c_str());
			Party.battle->heal(*target, dam, false);
		}
		else*/
		dam = Party.battle->damage(&actor, *target, amt, element,
			mult + actor.focus,	std::string(target->name + " loses %d HP.")
			 .c_str());
/*		if (drainHP)
			Party.battle->heal(actor, dam, false);*/
		

		total_damage += dam;
		
		result = AR_NORMAL;

		if (target->race()->name == "Chaos")
		{
			CSpell* spell = spellmap["Debilitator"];
					
			CTarget target;
			target.fill();
					
			if ((target)(actor, spell->target, true, spell->AI))
				if (CActionCastSpellBattle(actor, spell)(target)
					== AR_NONE)
				add_message("It has no effect.");
		}
	}
	return result;
}

action_result_type CActionCastSpell::drain(CTarget Target)
{
	action_result_type result = AR_NONE;
	next();

	int dividend, addendum;

	if (token == "min")
		dividend = 6, addendum = 0;
	else if (token == "low")
		dividend = 5, addendum = 1;
	else if (token == "med")
		dividend = 4, addendum = 2;
	else if (token == "high")
		dividend = 3, addendum = 3;
	else if (token == "max")
		dividend = 2, addendum = 4;
	else
		return AR_ERROR;

	CDeal drain_deal(dv(actor.eInt(), dividend) + addendum);

	next();

	int amt = drain_deal.total();

	if (actor.is_player())
	{
		int max_gainable = ((CPlayer&)actor).MaxHP - actor.curHP();
		if (max_gainable < amt)
			amt = max_gainable;
	}
		
	CTarget Alive;

	for(CActor* target : Target)
		if (!reflectmap[target]->defeated())
			Alive.insert(reflectmap[target]);

	if (Alive.size() == 0)
		return AR_NONE;

	int amt_each = dv(amt, Alive.size());

	for(CActor* target : Alive)
	{
		if (*target == ST_UNDEAD)
		{
			more("%s drains %d HP from %s.", target->name.c_str(), amt_each,
				actor.name.c_str());
			target->gainHP(amt_each);
			Party.battle->loseHP(actor, amt_each);
		}
		else
		{
			more("%s drains %d HP from %s.", actor.name.c_str(), amt_each,
				target->name.c_str());
			actor.gainHP(amt_each);
			Party.battle->loseHP(*target, amt_each);
		}
	}

	return AR_NORMAL;
}

action_result_type CActionCastSpell::escape()
{
	next();

	if (!target || target->is_player())
	{
		message("You escape from the enemy.");
		Party.battle->the_battle_rages = false;
		return AR_FLED;
	}
	else
	{
		if (target->race()->boss)
		{
			message("%s does not fear you.", target->name.c_str());
			return AR_NORMAL;
		}
		Party.battle->escape((CMonster*)target);
		return AR_NORMAL;
	}
}

action_result_type CActionCastSpell::gainMP(CTarget Target)
{
	int amt = 0;

	next();

	int dividend, addendum;

	if (token == "min")
		dividend = 6, addendum = 0;
	else if (token == "low")
		dividend = 5, addendum = 1;
	else if (token == "med")
		dividend = 4, addendum = 2;
	else if (token == "high")
		dividend = 3, addendum = 3;
	else if (token == "max")
		dividend = 2, addendum = 4;
	else
		return AR_ERROR;

	next();

	amt = CDeal(dv(actor.eInt(), dividend) + addendum).total();

	CTarget Alive;

	for(CActor* target : Target)
		if (!reflectmap[target]->defeated() &&
			reflectmap[target]->is_player())
			Alive.insert(reflectmap[target]);

	if (Alive.size() == 0)
		return AR_NONE;

	amt = dv(amt, Alive.size());

	for(CActor* target : Alive)
	{
			((CPlayer*)target)->tMP += amt;
			message(target->name + " gains " + to_string(amt) +
				" temporary MP.");
	}

	return AR_NORMAL;
}

action_result_type CActionCastSpell::heal(CTarget Target)
{
	int amt = 0;
	bool casterHP = false;

	next();

	int dividend, addendum;

	if (token == "min")
		dividend = 6, addendum = 0;
	else if (token == "low")
		dividend = 5, addendum = 2;
	else if (token == "med")
		dividend = 4, addendum = 5;
	else if (token == "high")
		dividend = 3, addendum = 9;
	else if (token == "max")
		dividend = 2, addendum = 14;
	else if (token == "caster_HP")
		casterHP = true;
	else
		return AR_ERROR;

	next();

	if (casterHP)
		amt = actor.curHP();
	else
		amt = CDeal(dv(actor.eInt(), dividend) + addendum).total();

	CTarget Alive;

	for(CActor* target : Target)
		if (!reflectmap[target]->defeated())
			Alive.insert(reflectmap[target]); 

	if (Alive.size() == 0)
		return AR_NONE;

	amt = dv(amt, Alive.size());

	int gain;

	for(CActor* target : Alive)
		if (Party.battle)
		{
			gain = Party.battle->heal(*target, amt, true);
		}
		else
		{
			gain = target->gainHP(amt);
			if (gain)
				more("%s gains %d HP.", target->name.c_str(), gain);
		}

	if (gain)
		return AR_NORMAL;
	else
		return AR_NONE;
}

action_result_type CActionCastSpell::loseHP()
{
	int amt;
	next();
	
	if (token == "caster_HP")
	{
		amt = actor.curHP();
	}
	else if (token == "all")
	{
		amt = target->curHP();
	}
	else if (token == "lost")
	{
		if (!actor.is_player())
		{
			message("Monsters do not have MaxHP!");
			return AR_ERROR;
		}
		else
			amt = ((CPlayer&)actor).MaxHP - ((CPlayer&)actor).curHP();
	}

	if (!target->defeated())
	{
		if (token != "all")
			message("%s loses %d HP.", target->name.c_str(), amt);

		Party.battle->loseHP(*target, amt);
	}

	if (next() == "both")
	{
		if (!actor.defeated())
		{
			message("%s loses %d HP.", actor.name.c_str(), amt);
			Party.battle->loseHP(actor, amt);
		}
		next();
	}

	return AR_NORMAL;
}

action_result_type CActionCastSpell::stat()
{
	next();
	int bonus = ni(effect);

	if (target->defeated())
	{
		next();
		return AR_NONE;
	}

	if (target->gain_stat(token, bonus))
	{
		if (token == "Agi")
			Party.battle->form_BattleQ();

		std::string statname;
		if (token == "Agi")
			statname = "Agility";
		else if (token == "Eva")
			statname = "Evade";
		else if (token == "Hit")
			statname = "Hit";
		else if (token == "Int")
			statname = "Intelligence";
		else if (token == "Vit")
			statname = "Vitality";
		else if (token == "Atk")
			statname = "Attack";

		if (bonus >= 0)
			more("%s gains %d %s.", target->name.c_str(), bonus,
				statname.c_str());
		else
			more("%s loses %d %s.", target->name.c_str(), -bonus,
				statname.c_str());

		next();

		if (target->curHP() == 0)
			Party.battle->die(*target);

		return AR_NORMAL;
	}
	else
		return AR_ERROR;
}

action_result_type CActionCastSpellBattle::add()
{
	unsigned long status = ST_NONE;

	if (next() == "random_of")
	{
		std::vector<unsigned long> status_list;
		while(Status.count(next()))
			status_list.push_back(statusmap[token]);
		status |= random(status_list[status_list.size()]);
	}
	else do
		status |= statusmap[token];
	while(Status.count(next()));

	if (target->defeated())
		return AR_NONE;

	if (Party.battle->gain_status(*target, status))
		return AR_NORMAL;

	return AR_NONE;
}

action_result_type CActionCastSpellBattle::remove()
{
	buffer removed;
	unsigned long status = ST_NONE;

	while(Status.count(next()))
		status |= statusmap[token];

	if (Party.battle->lose_status(*target, status))
		return AR_NORMAL;

	return AR_NONE;
}

action_result_type CActionCastSpellBattle::special(CTarget& Target)
{
	if (next() == "charge")
	{
		message("%s prepares a powerful attack.", actor.name.c_str());
		actor += CF_CHARGING;

		return AR_NORMAL;
	}
	else if (token == "dark_magic")
	{
		unsigned long status = statusmap[next()];
		next();

		if ((*Target.begin())->defeated())
			return AR_NONE;

		target = reflectmap[*Target.begin()];

		if (target->defeated())
			return AR_NONE;

		if (!(actor.cancels & status || target->cancels & status))
		{
			Party.battle->gain_status(actor, status);
			Party.battle->gain_status(*target, status);
			return AR_NORMAL;
		}

		return AR_NONE;
	}
	else if (token == "exit")
	{
		target = NULL;
		if (escape() == AR_FLED)
		{
			warp_outside();
			return AR_FLED;
		}
		return AR_NORMAL;
	}
	else if (token == "pep_up")
	{
		next();

		if (Target.empty())
			return AR_COND_FAIL;

		target = reflectmap[*Target.begin()];

		if (target == NULL || target->defeated())
			return AR_COND_FAIL;
	
		target->Vit += actor.Vit;

		message("%s and %s combine.",
			actor.name.c_str(), target->name.c_str());

		return AR_NORMAL;
	}
	else if (token == "red_feast")
	{	
		if (Target.empty())
			return AR_NONE;

		target = reflectmap[*Target.begin()];
		if (target == NULL || target->defeated())
			return AR_NONE;

		if (actor.is_player())
		{
			buffer buf;
			
			buf.push_back("drain");
			buf.push_back("med");
			buf.push_back("eoe");

			effect = buf;

			return drain(Target);
		}

		next();

		CMonster& ref = (CMonster&)actor;

		int feast_cards = ref.eVit() - ref.HPCards.size();
		int dam = 0;

		if (feast_cards < 1 || target->immune & ELEM_DARK)
			return AR_NONE;

		for(int i = 0; i < feast_cards; i++)
		{
			CCard tmpcard;
			ref.HPCards.push_back(tmpcard);
			dam += tmpcard.rank + 1;
		}

		Party.battle->loseHP(*target, dam);

		message("%s loses %d HP.", target->name.c_str(), dam);

		return AR_NORMAL;
	}
	else if (token == "scan")
	{
		for(CActor* it : Target)
		{
			target = reflectmap[it];
			if (!it->is_player())
				for(CCard& card : ((CMonster*)it)->HPCards)
					card.face_down = false;

			more("%s has %d HP.", target->name.c_str(), target->curHP());

			if (target->race()->races)
				add_message("Race: %s.",
					comma_list(race_to_buffer(target->race()->races)).c_str());

			if (target->weak)
				add_message("Weak to %s.",
					comma_list(element_to_buffer(target->weak)).c_str());

			if (target->resists)
				add_message("Resists %s.",
					comma_list(element_to_buffer(target->resists)).c_str());

			if (target->immune)
				add_message("Immune to %s.",
					comma_list(element_to_buffer(target->immune)).c_str());
		}

		next();

		return AR_NORMAL;
	}
	else if (token == "steal")
	{
		if (next() == "gil")
		{
			int gold_found = (target->Lv() *
				(target->Lv() + 1)) * 10;
			message("The party acquires %d gold.", gold_found);
			Party.Gil += gold_found;
			next();
		}
		else if (token == "item")
		{
			next();
		}
		else
			return AR_ERROR;
		
		return AR_NORMAL;
	}
	else if (token == "warp")
	{
		target = NULL;
		return escape();
	}

	return AR_ERROR;
}

action_result_type CActionCastSpellBattle::toggle()
{
	unsigned long gain = ST_NONE, remove = ST_NONE;

	while(Status.count(next()))
		if (*target == statusmap[token])
			remove |= statusmap[token];
		else
			gain |= statusmap[token];

	action_result_type result = Party.battle->gain_status(*target, gain) ?
		AR_NORMAL : AR_NONE;

	if (Party.battle->lose_status(*target, remove) || result == AR_NORMAL)
		return AR_NORMAL;

	return AR_NONE;
}

action_result_type CActionCastSpell::parse(buffer buf, CTarget Target)
{
	std::string local_token = ns(buf);
	CTarget subTarget(Target);

	while (local_token != "eoe")
	{
		action_result_type result = AR_NONE;

		if (local_token == "caster")
		{
			apply_to_caster = true;
			local_token = ns(buf);
		}
		else if (local_token == "special")
		{
			effect = buf;
			result = special(Target);
			buf = effect;
			local_token = token;
		}
		else if (local_token == "if")
		{
			apply_to_caster = false;

			if (Target.empty())
				return AR_NONE;
			
			effect = buf;
			target = *Target.begin();
			if (condition(reflectmap[target]) != AR_COND_FAIL)
				result = parse(effect, CTarget(target));
			else
				result = AR_NONE;
			Target.erase(Target.begin());
		}
		else if (local_token == "chance")
		{
			apply_to_caster = false;
			effect = buf;
			CTarget Passed = chance(Target);
			return parse(effect, Passed);
		}
		else if (local_token == "party")
		{
			apply_to_caster = false;
			CTarget PTarget;

			for(CActor* it : Party.battle->Actor)
				if (!it->defeated() && *it == CF_TARGETABLE &&
					it->is_player() == actor.is_player())
					PTarget.insert(it);

			return parse(buf, PTarget);
		}
		// effects below this point have no effect if there are no targets.
		else if (Target.empty())
		{
			local_token = ns(buf);
			result = AR_NONE;
		}
		else if (local_token == "damage" && spell->target == TRGT_ANY)
		{
			effect = buf;
			result = damage(Target);
			buf = effect;
			local_token = token;
		}
		else if (local_token == "heal" && spell->target == TRGT_ANY)
		{
			effect = buf;
			result = heal(Target);
			buf = effect;
			local_token = token;
		}
		else if (local_token == "gainMP" && spell->target == TRGT_ANY)
		{
			effect = buf;
			result = gainMP(Target);
			buf = effect;
			local_token = token;
		}
		else if (local_token == "drain" && spell->target == TRGT_ANY)
		{
			effect = buf;
			result = drain(Target);
			buf = effect;
			local_token = token;
		}
		else if (apply_to_caster)
		{
			target = &actor;
			effect = buf;
			if (local_token == "loseHP")
				result = loseHP();
			else if (local_token == "stat")
				result = stat();
			else
			{
				message("Actor cannot use effect %s on itself.",
					local_token.c_str());
				result = AR_ERROR;
			}

			buf = effect;
			local_token = token;
		}
		// The above effects happen once to all targets (or just happen if
		// there are no targets). The effects below happen once to each
		// actor in subTarget (i.e. each target of the spell).
		else if (subTarget.empty())
		{
			result = AR_NONE;
			buf = effect;
			subTarget = Target;
			if (local_token == "attack")
				local_token = "eoe";
			else
				local_token = token;
		}
		else
		{
			target = *subTarget.begin();
			effect = buf;

			if (local_token == "damage")
				result = damage(CTarget(target));
			else if (local_token == "drain")
				result = drain(CTarget(target));
			else if (local_token == "gainMP")
				result = gainMP(CTarget(target));
			else if (local_token == "heal")
				result = heal(CTarget(target));
			else
			{
				target = reflectmap[*subTarget.begin()];

				if (local_token == "add")
					result = add();
				else if (local_token == "attack")
				{
					CActionAttack CAA(actor, effect);
					CTarget CT(target);
					result = CAA(CT);
					
//					result = CActionAttack(actor, effect)(CTarget(target));
				}
				else if (local_token == "escape")
					result = escape();
				else if (local_token == "loseHP")
					result = loseHP();
				else if (local_token == "remove")
					result = remove();
				else if (local_token == "stat")
					result = stat();
				else if (local_token == "toggle")
					result = toggle();
				else
					result = AR_ERROR;
			}

			subTarget.erase(*subTarget.begin());
		}

		switch (result)
		{
		case AR_NORMAL:
			final_result = AR_NORMAL;
			break;

		case AR_COND_FAIL:
			token = "eoe";
			break;

		case AR_FLED:
		case AR_ERROR:
			return result;

		default:
			break;
		}
	}

	return AR_NONE;
}

action_result_type CActionCastSpell::operator()(CTarget& Target)
{
	final_result = AR_NONE;
	total_damage = 0;
	
	std::string verb = "casts";

	if (spell->Set == "Summon Low" || spell->Set == "Summon High")
		verb = "summons";

	more("%s %s %s.", actor.name.c_str(), verb.c_str(), spell->name.c_str());

	if (spell->Set == "Blue Low" || spell->Set == "Blue High")
		for(CPlayer& it : Party.Player)
			if (it != (ST_KO | ST_PETRIFY) && it.Spell.count(spell)
				&& !it.BlueMemory.count(spell))
			{
				it.BlueMemory.insert(spell);
				message("%s learns how to cast %s.", it.name.c_str(),
					spell->name.c_str());
			}

	CTarget::iterator it = Target.begin();

	for (CActor* target : Target)
	{
		if (*target == ST_REFLECT && Party.battle &&
			(spell->target == TRGT_ANY || spell->target == TRGT_SINGLE
			|| spell->target == TRGT_SELF || spell->target == TRGT_OTHER))
		{
			std::vector<CActor*> enemies;
			for(CActor* it : Party.battle->Actor)
				if (!it->defeated() && (*it & CF_ON_PLAYER_SIDE)
					!= (*target & CF_ON_PLAYER_SIDE) &&
					(it != &actor || spell->target != TRGT_OTHER))
					enemies.push_back(it);
			
			if (enemies.size() == 0)
			{
				message("%s reflects the spell.",
					target->name.c_str());
				reflectmap[target] = NULL;
			}
			else
			{
				reflectmap[target] = enemies[random(enemies.size())];

				more("%s reflects the spell onto %s.", target->name.c_str(),
					reflectmap[target]->name.c_str());
			}
		}
		else
			reflectmap[target] = target;

		it++;
	}

	std::set<CActor*> evaded;

	for(CActor* target : Target)
	{
		CActor* rtarget = reflectmap[target];
		if (rtarget->is_player() && !actor.is_player() && spell->AI == AI_ENEMY)
		{
			CPlayer* player = (CPlayer*)rtarget;
			if (player->has_ability("Absorb MP"))
			{
				if (CDeal(player->MEv).contains_ace_or_two())
				{
					evaded.insert(target);
					final_result = AR_NORMAL;
					message("%s absorbs %d MP from the spell.", player->name.
						c_str(), spell->MP);
					player->gainMP(spell->MP);
				}
			}
			else if (CDeal(player->MEv).contains_ace())
			{
				evaded.insert(target);
				final_result = AR_NORMAL;
			}
		}
/*		else if (target->is_player() && ((CPlayer*)target)->Abil.count(
			abilitymap["Absorb MP"]) && target != &actor &&
			*target != CF_HAS_ABSORBED)
		{
			int gain = ((CPlayer*)target)->gainMP(spell->MP);
			if (gain)
				message("%s absorbs %d MP from the spell.",
					target->name.c_str(), spell->MP);
			*target += CF_HAS_ABSORBED;
		}*/
	}

	if (evaded.size() > 0)
	{
		buffer Name;
		for(CActor* it : evaded)
		{
			Name.push_front(reflectmap[it]->name);
			Target.erase(it);
		}
		more("%s evade%s the spell.", comma_list(Name).c_str(), Name.size() == 1 ?
			"s" : "");
	}

	for(buffer& buf : spell->effect)
	{
		apply_to_caster = false;
		action_result_type result = parse(buf, Target);

		switch (result)
		{
		case AR_NORMAL:
			final_result = AR_NORMAL;
			break;

		case AR_FLED:
			return AR_FLED;

		case AR_ERROR:
			message(
				"Aborting spell: did not expect effect token "
				+ token + ".");
			return AR_ERROR;

		default:
			break;
		}
	}

	return final_result;
}


//void CActionCastSpell::search_for_endif()
//{
//	if (token == "endif")
//	{
//		next();
//		return;
//	}
//
//	int ifcount = 1;
//	while (ifcount > 0)
//	{
//		if (token == "eos")
//			message("Aborting Spell: expected \"endif\" before \"eos\".");
//
//		if (token == "endif")
//			ifcount--;
//
//		next();
//	}
//}
