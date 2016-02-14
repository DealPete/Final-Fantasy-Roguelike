#include <list>
#include <string>
#include <vector>

#include "Action.h"
#include "Combat.h"
#include "Items.h"
#include "Message.h"
#include "Monsters.h"
#include "Output.h"
#include "Party.h"
#include "Player.h"
#include "Quests.h"
#include "Random.h"
#include "Target.h"
#include "Spells.h"

CBattle::CBattle(bool f) : fighting_boss(f)
{
	Party.battle = this;

	prepare_combat_output();

	Party.enemyPad_viewport = 0;
	the_battle_rages = true;
	party_died = false;
	quick_turn = false;
	Party.view_init_cards = false;
}

void CBattle::add_actors()
{
	for(CPlayer& it : Party.Player)
	{
		Actor.push_back(&it);
		it.cflag = START_CF_PLAYER;
		it.focus = 0;
		it += (it.backRow ? CF_BACK_ROW : CF_NONE);
	}

	int i = 0;
	for (CMonster* monster : Enemy)
	{
		//Actor.push_back(new CMonster(monsterRace, (i % 2) ?
		//	CF_BACK_ROW : CF_NONE));
		Actor.push_back(monster);
		if (!(config_flags & CONFIG_CHAOS))
		{
			char suffix = i++ + 65;
			if (Enemy.size() > 1)
				Actor.back()->name += " " + std::string(1, suffix);
		}
	}

	for(CActor* it : Actor)
	{
		*it += (it->always | it->starts) & ~it->cancels;
		if (*it == ST_BLINK)
			it->blink_num = 1;
		if (*it == ST_QUICK)
			quick_turn = true;
		it->init = NULL;
		it->control = NULL;
	}
}

void CBattle::begin_round()
{
	int i = 0, init_cards = 0;

	for(CActor* it : Actor)
	{
		if (*it == ST_HASTE)
			init_cards = it->eAgi() * 2;
		else if (*it == ST_SLOW)
			init_cards = dv(it->eAgi(), 2);
		else
			init_cards = it->eAgi();

		if (it->init)
			delete it->init;

		it->init = new CDeal(init_cards, i);

		if (it->defeated() || *it == ST_STOP)
			it->init->clear();
		else if (*it == ST_HALT)
		{
			it->init->clear();
			lose_status(*it, ST_HALT);
			message("%s will not act this round.", it->name.c_str());
		}

		i++;
	}

	form_BattleQ();
}

bool CBattle::cast_spell(CPlayer& player, CSpell* spell)
{
	if (spell->MP > player.MP + player.tMP)
	{
		message(player.name + " does not have enough MP to cast " +
			spell->name + ".");
		return false;
	}

	if ((spell->Set == "Blue Low" || spell->Set == "Blue High")
		&& !player.BlueMemory.count(spell))
	{
		message("%s must witness %s being cast before %s can use it %s.",
			player.name.c_str(), spell->name.c_str(),
			player.subp().c_str(), player.refp().c_str());
		return false;
	}

	CTarget target;
	target.fill();

	if ((target)(player, spell->target, false,
		spell->AI))
	{
		player.tMP -= spell->MP;
		if (player.tMP < 0)
		{
			player.loseMP(-player.tMP);
			player.tMP = 0;
		}

		if (CActionCastSpellBattle(player, spell)(target) == AR_NONE)
			add_message("It has no effect.");
		return true;
	}
	return false;
}

void CBattle::conduct()
{
	begin_round();

	do
	{
		CActor* actor;

		draw();

		if (quick_turn)
		{
			QuickList.clear();
			for(CActor* it : Actor)
				if (*it == ST_QUICK)
					QuickList.push_back(*it->init);

			QuickList.sort();
			QuickList.reverse();
			quick_turn = false;
		}

		if (BattleQ.empty() && QuickList.empty())
		{
			end_round();
			if (the_battle_rages)
				begin_round();
		}
		else
		{
			if (!QuickList.empty())
			{
				actor = Actor[QuickList.front().deal_num];
				QuickList.pop_front();
				*actor -= ST_QUICK;
			}
			else
			{
				actor = BattleQ.front();

				if (actor->init->front().rank == 12)
					actor->init->pop_front();
				else
					actor->init->clear();

				BattleQ.pop_front();
				draw_BattleQ();
			}
				
			if (*actor != ST_INCAPACITATING)
					take_turn(*actor);
					
			if (one_side_dead())
				the_battle_rages = false;
		}
		
	} while (the_battle_rages);
}

void CBattle::control_monster(CActor& actor)
{
	CMonster& target = (CMonster&)*actor.control;
	buffer buf;
	
	if (target != (ST_MUTE | ST_FROG))
		for(int i = SUIT_SPADE; i <= SUIT_HEART; i++)
			if (target.action(i) != "Attack")
				buf.push_back(target.action(i));

	buf.push_back("Attack");
	buf.push_back("Release");

	for (;;)
	{
		draw_monsters(actor.control);
		Party.draw(rightWindow, &actor);
        int select;
		
		while ((select = menu_select(18 - buf.size(), 51, buf)) == 0)
			{};
	
		std::string choice;
		buffer::iterator it = buf.begin();

		for(int i = 1; i < select; i++, it++)
			;

		choice = *it;

		if (choice == "Release")
		{
			actor.control = actor.control->control = NULL;
			draw();
			player_combat(actor);
			return;
		}
		else if (choice == "Attack")
		{
			CTarget tgt;
			tgt.fill();

			if (tgt(target, TRGT_SINGLE, false))
			{
				buffer buf;
				buf.push_back("eoe");
				(void)CActionAttack(target, buf)(tgt);
				return;
			}
		}
		else
		{
			CSpell* spell = spellmap[choice];

			if (!spell)
				message("That spell has yet to be implemented.");
			else
			{
				CTarget tgt;
				tgt.fill();

				if ((tgt)(target, spell->target, false, spell->AI))
				{
					if (CActionCastSpellBattle(target, spell)(tgt) == AR_NONE)
						add_message("It has no effect.");
					return;
				}
			}
		}
	}
}

void CBattle::die(CActor& actor)
{
	if (actor.defeated())
		return;

	if (actor == ST_RERAISE)
	{
		more(actor.name + " regains consciousness.");
		actor -= ST_RERAISE;
		actor.setHP(actor.eVit());
		
		draw();
		return;
	}
		
	actor -= ~ST_DEFEATING;
	actor += ST_KO | actor.always;

	actor.setHP(0);

	if (actor.init)
	{
		actor.init->clear();
		form_BattleQ();
	}

	if (!actor.is_player())
		actor -= CF_TARGETABLE;

	actor &= ~LOSE_WHEN_DEFEATED_CF;

	if (actor.control)
		actor.control = actor.control->control = NULL;

	draw();
}

void CBattle::end_round()
{
	for(CActor* it : Actor)
	{
		if (*it == CF_TARGETABLE && !it->defeated())
		{
			if (*it == ST_COUNT)
			{
				it->time_counters--;
				if (it->time_counters == 0)
				{
					more(it->name + " meets the grim reaper.");
					*it -= ST_COUNT;
					if (!gain_status(*it, ST_KO))
						add_message(it->name + " ignores his icy grip.");
					
				}
			}

			if (*it == ST_CALCIFY)
			{
				it->calcify_counters--;
				if (it->calcify_counters == 0)
				{
					more(it->name + " becomes a statue.");
					*it -= ST_CALCIFY;
					if (!gain_status(*it, ST_PETRIFY))
						add_message(it->name + " shatters his stony form.");
				}
			}

			if (*it == ST_POISON)
			{
				message("%s loses %d HP from poison.", 
					it->name.c_str(), it->eVit());
				loseHP(*it, it->eVit());
			}

			if (*it == ST_REGEN)
				heal(*it, CCard().rank);
		}

		for (unsigned long status = ST_SLEEP; status < ST_BERSERK; status <<= 1)
			if (*it == status && CDeal(it->eVit()).contains_ace())
				(void)lose_status(*it, status);
	}

	if (one_side_dead()) the_battle_rages = false;
}

void CBattle::escape(CMonster* monster)
{
	more(monster->name + " escapes from the battle!");
	
	monster->init->clear();
	Party.battle->form_BattleQ();

	Enemy.remove(monster);

	*monster += CF_FLED;
	*monster -= CF_TARGETABLE;

	*monster &= ~LOSE_WHEN_DEFEATED_CF;

	if (monster->control)
		monster->control = monster->control->control = NULL;

	draw();
}

int CBattle::damage(CActor* source, CActor& target, int base_damage,
					unsigned char element, int mult,
					std::string msg, bool mult_includes_base,
					bool drainHP, bool drainMP)
{
	bool absorb = false;
	int HP_before = target.curHP();
	CCard card1, card2;

	if (!mult_includes_base)
		if (card1.suit == card2.suit)
			mult += 4;
		else if (card1.is_black() == card2.is_black())
			mult += 3;
		else
			mult += 2;

	for(unsigned int i = 1; i <= 8; i++)
	{
		if (element & target.absorbs & 1 << i)
			absorb = true;
		if (element & target.weak & 1 << i)
			mult++;
		if (element & target.resists & 1 << i)
			mult--;
		if (source && element & source->strengthens & 1 << i)
			mult++;
		/*if (element & target.weak & 1 << i)
			numer *= 2;
		if (element & target.resists & 1 << i)
			denom *= 2;
		if (source && element & source->strengthens & 1 << i)
		{
			numer *= 3;
			denom *= 2;
		}*/
	}

	int dam = base_damage;
	
	if (element & target.immune)
		dam = 0;
	else
	{
		if (dam < 1)
			dam = 1;
		
		if (mult <= 0)
			dam = dv(dam, 2);
		else
			dam *= mult;
	}


	if (drainMP)
		if (target == ST_UNDEAD)
		{
			msg += " " + source->name + " loses " + std::to_string(dv(dam, 2))
				+ " MP!";
			source->loseMP(dv(dam, 2));
		}
		else
		{
			msg += " " + source->name + " gains " + std::to_string(dv(dam, 2))
				+ " MP.";
			source->gainMP(dv(dam, 2));
		}

	if (drainHP && source->is_player())
		{
			CPlayer& ref = (CPlayer&)(*source);
			if (dam > (ref.MaxHP - ref.curHP()))
				dam = ref.MaxHP - ref.curHP();
		}

	more(msg.c_str(), dam);

	if (target.has_ability("MP Switch") && CCard().is_black())
	{
		int half_dam = dv(dam, 2);
		int lost_MP = target.loseMP(half_dam);

		if (lost_MP)
		{
			dam -= lost_MP;
			add_message("%s absorbs %d damage as lost MP.",
				target.name.c_str(), lost_MP);
		}
	}

	if (drainHP)
	{
		if (source == &target)
			add_message("The wound heals itself.");
		else if (target == ST_UNDEAD)
		{
			target.gainHP(dam);
			loseHP(*source, dam);
		}
		else
		{
			source->gainHP(dam);
			loseHP(target, dam);
		}
	}
	else if (absorb)
	{
		add_message("%s absorbs the damage.", target.name.c_str());
		target.gainHP(dam);
	}
	else
		loseHP(target, dam);

	draw();

	if (HP_before - target.curHP() >= 0)
		return HP_before - target.curHP();
	else
		return 0;
}

void CBattle::draw_monsters(CTarget& cursor, CTarget* Target)
{
	_werase(enemyPad);

	int i = 1;
//	buffer buf;

	std::map<CActor*, int> EnemyY;

	for(CActor* it : Actor)
	{
		if (!it->is_player() && !it->defeated())
		{
			//buf.push("");

			CMonster& ref = *((CMonster*)it);

			_colortype color = ref == CF_BACK_ROW ? LIGHTGRAY : WHITE;

			bool reverse = (cursor.count(it) != 0);

			if (reverse)
				_wputcstr_reverse(enemyPad, i, 1, color, "%s", ref.name.c_str());
			else
				_wputcstr(enemyPad, i, 1, color, "%s", ref.name.c_str());

		//	buf.push(" " + ref.name, color);

			if (Target && Target->count(it))
			{
#ifdef __linux__
				_chtype checkMark = 42;
#else
				_chtype checkMark = 251;
#endif
				_wputchar(enemyPad, i, 17, 251, WHITE, reverse);
			}
			//if (Target.Cards[it] > 0)
				//buf.push_back
				//wmvcprintw(enemyPad, i, 17, WHITE | A_REVERSE, "%d",
			//		Target.Cards[it]);

			EnemyY[it] = i++;


			std::string HPstring;
			for(CCard& it : ref.HPCards)
				if (it.face_down)
					HPstring += " ";
				else
					HPstring += Ranks[it.rank];

			//buf.push(" " + HPstring, color);
			int j = 0;

			for(char it : HPstring)
			{
				if (++j > _getwidth(enemyPad) - 2)
				{
					j = 1;
					++i;
				}

				_wputchar(enemyPad, i, j, it, LIGHTGRAY, true);
			}

			++i;

			if (ref != ST_NONE)
				//buf.push(buffer_to_string(status_to_buffer(ref.status,
				//	ref.time_counters, ref.blink_num)), RED);
				i = cformat_print(enemyPad, LIGHTRED, i,
					buffer_to_string(status_to_buffer(ref.status,
					ref.time_counters, ref.blink_num, ref.calcify_counters)));

			if (ref.control)
				i = cformat_print(enemyPad, RED, i,
					"Controlled by " + ref.control->name);

			if (Party.view_init_cards)
				_wputstring(enemyPad, i++, 1, GREEN,
					"[" + ref.init->rank_string() + "]");
			++i;

		}
	}


	//draw_buffer(enemyWindow, &buf, false);
	show_enemy_pad(Party.enemyPad_viewport);
}

void CBattle::draw_monsters(CActor* actor)
{
	CTarget CT(actor);
	draw_monsters(CT);
	
//	draw_monsters(CTarget(actor));
}

void CBattle::draw_monsters()
{
	CTarget CT;
	draw_monsters(CT);
	
//	draw_monsters(CTarget());
}

void CBattle::draw_BattleQ()
{
	buffer BattleQbuffer;

	//if (card_view)
	//	foreach (CActor* it, Actor)
	//	{
	//		BattleQbuffer.push_back(it->name);
	//		std::string str = "[" + it->init->rank_string() + "]";
	//		BattleQbuffer.push_back(str);
	//	}
	//else
		for(CActor* it : BattleQ)
			BattleQbuffer.push_back(it->name);

	draw_buffer(battleQWindow, &BattleQbuffer, false);
	show_window(battleQWindow);
}

void CBattle::draw()
{
	draw_BattleQ();

	draw_monsters();

	Party.draw(rightWindow);
}

void CBattle::form_BattleQ()
{
	BattleQ.clear();

	int kings;
	std::list<CDeal> Inits;

	for(CActor* it : Actor)
		Inits.push_back(*it->init);

	Inits.sort();

	do
	{
		kings = 0;

		for(CDeal& it : Inits)
			if (it.size() && it.front().rank == 12)
			{
				BattleQ.push_back(Actor[it.deal_num]);
				it.pop_front();
				kings++;
			}
	} while (kings);

	BattleQ.reverse();

	Inits.sort();

	Inits.reverse();

	for(CDeal& it : Inits)
		if (it.size() && it.front().rank)
			BattleQ.push_back(Actor[it.deal_num]);

	draw_BattleQ();
}

bool CBattle::gain_status(CActor& target, unsigned long status)
{
	if (status & target.status & ST_INVITE)
	{
		lose_status(target, ST_INVITE);
		status &= ~ST_INVITE;
	}

	if (status & target.status & ST_CALCIFY)
	{
		target.calcify_counters--;
		if (target.calcify_counters == 0)
		{
			target -= ST_CALCIFY;
			if (!gain_status(target, ST_PETRIFY))
				add_message("%s gains Petrify, but shatters %s stony form.",
					target.name.c_str(), target.posp().c_str());
			else
				return true;

		}
		else
			message("%s feels %s limbs stiffening quicker.",
				target.name.c_str(), target.posp().c_str());

		status &= ~ST_CALCIFY;
		if (status == ST_NONE)
			return true;
	}
		
	status &= ~(target.status | target.cancels);

	if (status & ST_STOP)
		status &= ~(ST_HASTE | ST_SLOW);

	if (status & ST_SLOW)
		status &= ~ST_HASTE;

	if (status & ST_DEFEATING)
	{
		if (status & ST_INVITE)
			target ^= CF_ON_PLAYER_SIDE;

		if (status & ST_PETRIFY)
		{
			add_message(target.name + " gains Petrify.");

			target += ST_PETRIFY;

			if (!target.is_player())
				target -= CF_TARGETABLE;

			target &= ~LOSE_WHEN_DEFEATED_CF;

			draw();

			return true;
		}
		else if (status & ST_KO)
			if (target == ST_UNDEAD)
			{
				add_message(target.name + " returns to Maximum HP.");
				target.heal_to_full();
				status &= ~ST_KO;
				if (status == ST_NONE)
					return true;
			}
			else
			{
				add_message(target.name + " is defeated.");
				die(target);
				return true;
			}
	}

	if (status & ST_BLINK)
		target.blink_num = 1;

	if (status & ST_BLINKx2)
	{
		target.blink_num = 2;
		status &= ~ST_BLINKx2;
		status |= ST_BLINK;
	}

	if (status & ST_COUNT)
		target.time_counters = 3;

	if (status & ST_CRITICAL)
	{
		int lost_HP = 0;
		if (target.is_player())
		{
			int crit_point = dv(((CPlayer&)target).MaxHP, 4);
			if (target.curHP() > crit_point)
			{
				lost_HP = target.curHP() - crit_point;
				message("%s loses %d HP.", target.name.c_str(), lost_HP);
				loseHP(target, lost_HP);
			}
		}
		else
		{
			CMonster& ref = (CMonster&)target;
			while ((int)ref.HPCards.size() > dv(ref.eVit(), 4))
			{
				lost_HP += ref.HPCards.back().rank + 1;
				ref.HPCards.pop_back();
			}
			if (lost_HP)
				message("%s loses %d HP.", target.name.c_str(), lost_HP);
		}

		status &= ~ST_CRITICAL;

		if (lost_HP && status == ST_NONE)
			return true;
	}

	if (status & ST_HASTE)
	{
		lose_status(target, ST_SLOW | ST_STOP);
		if (target != (ST_SLOW | ST_STOP))
		{
			if (!target.init->empty())
			{
				target.init->add_cards(target.eAgi());
				form_BattleQ();
			}
		}
		else
			status &= ~ST_HASTE;
	}

	if (status & ST_CALCIFY)
		target.calcify_counters = 4;

	if (status & ST_SLOW)
	{
		lose_status(target, ST_HASTE | ST_STOP);
		if (target != (ST_HASTE | ST_STOP))
		{
			if (target.init->size())
			{
				target.init->cut_highest(dv(target.init->size(), 2));
				form_BattleQ();
			}
		}
		else
			status &= ~ST_SLOW;
	}

	if (status & ST_STOP)
	{
		lose_status(target, ST_SLOW | ST_HASTE);
		if (target != (ST_SLOW | ST_HASTE))
		{
			target.init->clear();
			form_BattleQ();
		}
		else
			status &= ~ST_STOP;
	}

	if (status == ST_NONE)
		return false;

	add_message(target.name + " gains " +
		comma_list(status_to_buffer(status)) + ".");
	target += status;

	if (status & ST_QUICK)
		quick_turn = true;

	if (status & ST_HALT && !target.init->empty())
	{
		target.init->clear();
		add_message("%s loses his turns for this round.",
			target.name.c_str());
		target -= ST_HALT;
		form_BattleQ();
	}

	if (target.control && (target & (ST_INCAPACITATING | ST_DISTRACTING)))
	{
		if (target.is_player())
			message("%s loses control of %s.", target.name.c_str(),
				target.control->name.c_str());
		else
			message("%s loses control of %s.",
				target.control->name.c_str(), target.name.c_str());

		target.control = target.control->control = NULL;
	}

	draw();
	return true;
}

int CBattle::heal(CActor& target, int total, bool HPrecovery)
{
	int recovered = 0;
	std::string str = " HP.";
	std::string verb = " gains ";

	if (target == ST_UNDEAD)
	{
		recovered = total;
		verb = " loses ";
	}
	else
	{
		recovered = target.gainHP(total);
		if (target.is_player() && HPrecovery &&
			((CPlayer&)target).Equip[ITEM_HEAD]->name == "Crown")
			recovered *= 2;
	}

	if (recovered == 0)
		return 0;

	more(target.name + verb + to_string(recovered) + str);

	if (target == ST_UNDEAD)
		loseHP(target, total);

	return recovered;
}

void CBattle::loseHP(CActor& target, int total)
{
	if (!total)
		return;

	bool is_critical = target.is_critical();

	if ((config_flags & CONFIG_LUCK) &&
		total > target.curHP() && target.is_player() &&
		CDeal(dv(((CPlayer&)target).Luc, 2)).contains_ace())
	{
		add_message(
"%s would be defeated, but fortune intercedes to stay his demise!",
target.name.c_str());
		target.setHP(target.eVit());
	}
	else if (!target.loseHP(total))
	{
		add_message(target.name + " is defeated.");
		die(target);
		return;
	}

	if (target.control && !target.is_player())
	{
		message("%s loses control of %s.",
			target.control->name.c_str(), target.name.c_str());

		target.control = target.control->control = NULL;
	}

	if (target.is_critical() && !is_critical)
	{
		if (target.has_ability("Critical Quick"))
			gain_status(target, ST_QUICK);
		if (target.race()->name == "Chaos")
			((CMonster&)target).change_form("Chaos");
	}
}

void CBattle::monster_combat(CActor& actor)
{
	CTarget target;
	target.fill();

	CCard decision;

	std::string spellstr = actor.action(decision.suit);

	if (spellstr == "Attack" || spellstr == "Form" || actor == ST_MUTE)
	{
		if (target(actor, TRGT_SINGLE, true))
		{
			buffer buf;
			buf.push_back("eoe");
			(void)CActionAttack(actor, buf)(target);
		}

		if (spellstr == "Form")
			((CMonster&)actor).change_form();
	}
	else  if (spellstr == "Spawn6Pirate")
	{
		message("Bikke calls for help. Six more pirates arrive!");
		summon(monstermap["Pirate"], 6);
	}
	else
	{
		if (actor == ST_FROG)
		{
			message(
"%s tries to cast %s, not realizing that frogs cannot use magic.",
				actor.name.c_str(), spellstr.c_str());
			return;
		}

		if (!spellmap.count(spellstr))
		{
			message("%s tries to cast %s, but it has yet to be implemented.",
				actor.name.c_str(), spellstr.c_str());
			return;
		}

		CSpell* spell = spellmap[spellstr];

		CTarget target;
		target.fill();

		if ((target)(actor, spell->target, true, spell->AI))
			if (CActionCastSpellBattle(actor, spell)(target) == AR_NONE)
				add_message("It has no effect.");
	}
}

bool CBattle::lose_status(CActor& target, unsigned long status)
{
	bool defeated = false;
	status &= target.status & ~target.always;

	if (status == ST_NONE)
		return false;

	if (status & ST_HASTE)
		target.init->cut_highest(target.eAgi());

	if (target.defeated())
		defeated = true;

	add_message(target.name + " loses " +
		comma_list(status_to_buffer(status)) + ".");
	target -= status;

	if (defeated && !target.defeated())
		target += CF_TARGETABLE;

	draw();

	return true;
}

bool CBattle::one_side_dead()
{
	int alive_enemy = 0;
	int alive_hero = 0;
	int alive_player = 0;

	for (CActor* it : Actor)
		if (!it->defeated())
		{
			if (it->is_player())
				alive_player++;
			if (*it == CF_ON_PLAYER_SIDE)
				alive_hero++;
			else
				alive_enemy++;
		}

	if (alive_hero == 0 || (alive_enemy == 0 && alive_player == 0))
	{
		more("The party was defeated.");
		party_died = true;
		return true;
	}
	
	if (alive_enemy == 0)
	{
		int GilGain = 0;

		if (fighting_boss)
		{
			for(CActor* it : Actor)
				if (it->race()->boss)
					more("%s crackles as %s slowly fades into nothingness.",
						it->name.c_str(), it->subp().c_str());
			
			GilGain = Party.dungeon().quest->Gil;
			Party.quests_complete |= (1 << Party.dungeon().quest->serial);
			delete Party.tile().Feature;
			Party.tile().Feature = NULL;
			if (Party.dungeon().quest->reward->name != "None")
			{
				message("You obtain the %s.",
					Party.dungeon().quest->reward->name.c_str());
				Party.KeyItem.insert(Party.dungeon().quest->reward);
			}
			else
			{
				more("You find yourself back at town.");
				Party.L = Party.QuestTown;
			}
			Party.quest = NULL;
		}
		else
		{
			if (Enemy.size())
			{
				more(
	"The enemy is defeated!  You cheer as a victory fanfare plays.");

				for(CMonster* monster : Enemy)
					GilGain += (monster->race()->Lv *
					(monster->race()->Lv + 1)) * 10;
			}
			else
				more(
	"You mock your craven foes as they flee the battlefield.");
		}

		int XPGain = 0, JPGain = 1;

		if (Enemy.size())
		{
			
			for(CMonster* monster : Enemy)
			{
				int Lv = monster->race()->Lv;

				if (Lv > JPGain)
					JPGain = Lv;

				XPGain += dv(Lv * 12, Party.Player.size());
			}

			message("All conscious heroes gain %d XP and %d JP.",
				XPGain, JPGain);

			for(CPlayer& it : Party.Player)
				if (!it.defeated())
				{
					it.XP += XPGain;
					it.JP += JPGain;

					while (it.XP >= it.TNL)
						it.level_up();
				}

			message("The party finds %d Gil.", GilGain);
			Party.Gil += GilGain;

			if (!fighting_boss)
			{
				for(CActor* it : Actor)
					if (!it->is_player())
					{
						CMonster& ref = *((CMonster*)it);
						CCard card;
						if (ref.has_item && card.rank == 0)
						{
							std::vector<CPlayer*> vec;
							CPlayer* fdr;
							CItem* found;

							for(CPlayer& player : Party.Player)
								if (player.burden()
									+ player.weight < player.Str)
									vec.push_back(&player);

							if (vec.size() == 0)
								fdr = &Party.Player[
									random(Party.Player.size())];
							else
								fdr = vec[random(vec.size())];

							if (card.suit == SUIT_SPADE)
								found = ref.race()->rare_item;
							else
								found = ref.race()->common_item;

							message("%s finds the %s.", fdr->name.c_str(),
								found->name.c_str());
							fdr->Inventory.push_back(found);
						}
					}
			}
		}

		return true;
	}

	return false;
}

void CBattle::player_combat(CActor& actor)
{
	std::vector<std::string> selection;

	selection.push_back("Item");
	selection.push_back("Run");
	selection.push_back("Row");
	selection.push_back("Attack");
	selection.push_back("Defend");
	selection.push_back("Equip");

	for(CAbility* it : ((CPlayer&)actor).Abil)
		if (it->type == "Command" || (it->type == "Set"
			&& actor != (ST_MUTE | ST_FROG) &&
			((CPlayer&)actor).seals != it))
			selection.push_back(it->name);

	for (;;)
	{
		Party.draw(rightWindow, &actor);

		int command = get_player_battle_command(selection,
			((CPlayer&)actor).battle_select_cursor);
		if (command != SEL_TOGGLE_CV)
			((CPlayer&)actor).battle_select_cursor = command;
			
		switch(command)
		{
		case SEL_TOGGLE_CV:
			Party.view_init_cards = !Party.view_init_cards;
			draw();
			break;

		case SEL_ITEM:
			{
				if (((CPlayer&)actor).Inventory.empty())
					message(actor.name + " has no items.");
				else
					if (CActionItem(actor)())
						return;
				break;
			}

		case SEL_RUN:
			{
				(void)CActionFlee(actor)();
				return;
			}

		case SEL_ROW:
			{
				(void)CActionChangeRow(actor)();
				return;
			}

		case SEL_ATTACK:
			{
				CTarget target;
				target.fill();

				if (target(actor, TRGT_SINGLE))
				{
					buffer buf;
					buf.push_back("eoe");
					if (CActionAttack(actor, buf)(target))
						return;
					message("%s is already defeated.",
						(*target.begin())->name.c_str());
				}
				break;
			}

			
		case SEL_DEFEND:
			{
				(void)CActionDefend(actor)();
				return;
			}

		case SEL_EQUIP:
			{
				if (CActionEquip(actor)())
					return;
				break;
			}

		case SEL_ABIL1:
			if (use_ability(*abilitymap[selection[SEL_ABIL1]], actor))
				return;
			break;

		case SEL_ABIL2:
			if (use_ability(*abilitymap[selection[SEL_ABIL2]], actor))
				return;
			break;

		case SEL_ABIL3:
			if (use_ability(*abilitymap[selection[SEL_ABIL3]], actor))
				return;
			break;
		}
	}
}

void CBattle::summon(CMonsterRace* monsterRace, int number)
{
	for (int i = 0; i < number; i++)
	{
		//Actor.push_back(new CMonster(monsterRace, (i % 2) ?
		//	CF_BACK_ROW : CF_NONE));
		CMonster* monster = new CMonster(monsterRace, CF_NONE);
		Enemy.push_back(monster);
		Actor.push_back(monster);

		//char suffix = i + 65;
		//monster->name += " " + std::string(1, suffix);

		*monster += (monster->always | monster->starts) & ~monster->cancels;
		if (*monster == ST_BLINK)
			monster->blink_num = 1;
		if (*monster == ST_QUICK)
			quick_turn = true;
		monster->init = new CDeal(0, Actor.size() - 1);
		monster->control = NULL;
	}
}

void CBattle::take_turn(CActor& actor)
{
	actor -= CF_DEFENDING;

	if (actor == CF_JUMPING)
	{
		CTarget target;
		target.fill();
		buffer buf;
		buf.push_back("jump");
		buf.push_back("eoe");

		message("Choose a target for %s's jump.", actor.name.c_str());
		while(!target(actor, TRGT_SINGLE, false, AI_ENEMY))
		{}

		actor += CF_TARGETABLE;
		actor -= CF_JUMPING;

		if (!CActionAttack(actor, buf)(target))
			more(actor.name + " fails to attack " +
				(*target.begin())->name + ".");

		return;
	}

	if (actor == ST_BERSERK)
	{
		CTarget target;
		target.fill();

		if (target(actor, TRGT_SINGLE, true))
		{
			buffer buf;
			buf.push_back("berserk");
			buf.push_back("eoe");
			(void)CActionAttack(actor, buf)(target);
		}

		return;
	}

	if (actor.is_player())
	{
		if (actor == ST_CONFUSE || actor == ST_CHARM || actor == ST_INVITE)
		{
			CTarget target;
			target.fill();

			if (target(actor, TRGT_SINGLE, true))
			{
				buffer buf;
				buf.push_back("eoe");
				(void)CActionAttack(actor, buf)(target);
			}

			return;
		}

		if (actor.control)
			control_monster(actor);
		else
			player_combat(actor);
	}
	else if (!actor.control)
		monster_combat(actor);

	if (actor == ST_FATAL)
		loseHP(actor, actor.curHP());
}

bool CBattle::use_ability(CAbility& ability, CActor& actor)
{
	CPlayer& p_ref = (CPlayer&)actor;

	if (ability.type == "Command")
	{
		CTarget target;
		target.fill();
		
		if ((target)(actor, ability.target))
			return CActionCommand(actor, &ability)(target);

		return false;
	}
	else
	{
		CSpellFunctor functor(upperWindow, false);
		functor.casting = true;
		functor.combat = true;
		functor.caster = (CPlayer*)&actor;

		for(CSpell* spell : p_ref.Spell)
		{
			if (spell->Set == ability.name)
			{
				functor.Menu[0].push_back(color_string(spell->name,
					((CPlayer&)actor).MP + ((CPlayer&)actor).tMP >= spell->MP
					? LIGHTGRAY : DARKGRAY));
				functor.SpellItem[0].push_back(spell);
			}
		}

		if (functor.SpellItem[0].size() == 0)
		{
			message("%s does not know any spells from the set %s.",
				actor.name.c_str(), ability.name.c_str());
			return false;
		}

		for(;;)
		{
			int choice = functor().second;

			if (choice)
			{
				if (cast_spell(p_ref, functor.SpellItem[0][--choice]))
					return true;
			}
			else
				return false;
		}
	}
}

void the_boss_attacks()
{
	CBattle Battle(true);

	for(int i = 0; i < Party.dungeon().quest->num_bosses; i++)
		Battle.Enemy.push_back(
			new CMonster(Party.dungeon().quest->boss, CF_NONE));

	Battle.add_actors();

	CActor& ref = *Battle.Enemy.back();

	message(ref.race()->greeting);

	std::string spellstr = ref.race()->starts_by_casting;

	if (spellstr == "");
	else if (spellstr == "Spawn6Pirate")
	{
		Battle.summon(monstermap["Pirate"], 6);
	}
	else if (!spellmap.count(spellstr))
	{
		message("%s tries to cast %s, but it has yet to be implemented.",
			ref.name.c_str(), spellstr.c_str());
		return;
	}
	else
	{
		CSpell* spell = spellmap[spellstr];

		CTarget target;
		target.fill();

		if ((target)(ref, spell->target, true, spell->AI))
			if (CActionCastSpellBattle(ref, spell)(target) == AR_NONE)
				add_message("It has no effect.");
	}

	Battle.conduct();
}

void monsters_attack()
{
#ifdef _DEBUG
	if (debug_flags & DEBUG_NO_COMBAT)
	{
		message("DEBUG: Ignoring combat.");
		return;
	}
#endif

	CBattle Battle(false);

	domain_type enemy_domain = (domain_type)Party.domain().serial;

	if (Party.domain().name == "Overworld")
	{
		if (Party.tile().terrain == TRN_WATER)
			enemy_domain = DOM_WATER;
	}

	int party_level = Party.dungeon().quest->Lv + Party.L.floor;
	int num_enemy = 0;
	
	if (config_flags & CONFIG_CHAOS)
	{
		num_enemy = random(4) + 2;
		if (num_enemy == 5 && random(2))
			num_enemy = 6;

		for(int i = 0; i < num_enemy; i++)
		{
			int enemy_level = party_level;

			switch (random(4))
			{
			case 0:
				while (random(2) && enemy_level > 1)
					enemy_level--;
				break;

			case 1:
				while (random(2) && enemy_level < 13)
					enemy_level++;
				break;
			}

			int enemy_race = (enemy_level - 1) * 4 + CCard().suit;

			Battle.Enemy.push_back(new CMonster(
				Domain[enemy_domain].encounter[enemy_race], CF_NONE));
		}

		message("Your enemies attack!");
	}
	else
	{
		int rand_num = random(3);
		int num_enemy = rand_num + 2;
		int enemy_level;

		if (Party.domain().name == "Overworld")
		{
			enemy_level = Party.QL;
		}
		else
		{
			enemy_level = party_level + 2 - rand_num;
			if (enemy_level > 12)
				enemy_level = 12;
		}

		int enemy_race = (enemy_level - 1) * 4 + CCard().suit;

		CMonsterRace& EnemyRace = *Domain[enemy_domain].encounter[enemy_race];
		more("%d %s attack%s!", num_enemy, num_enemy == 1 ?
			EnemyRace.name.c_str() : EnemyRace.plural.c_str(), num_enemy == 1 ?
			"s" : "");

		for(int i = 0; i < num_enemy; i++)
			Battle.Enemy.push_back(new CMonster(&EnemyRace, CF_NONE));
	}

	Battle.add_actors();
	Battle.conduct();
}

CBattle::~CBattle()
{
	for(CActor* it : Actor)
	{
		delete it->init;
		if (!it->is_player())
			delete it;
	}

	Party.battle = NULL;

	for(CPlayer& it : Party.Player)
	{
		it -= ~(ST_REMAIN | it.always);

		it.tStr = it.tAgi = it.tVit = it.tInt = it.tLuc =
			it.tAtk = it.tHit = it.tEva = it.tMP = 0;
		it.resists = it.timmune = ELEM_NONE;

		// debuff players, return to default rows, &c.
		it.cal_stats();
	}

	if (party_died)
		Party.die();

	draw_explore_screen();
}
