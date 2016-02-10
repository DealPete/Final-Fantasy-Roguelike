#include <chrono>

#include "Defines.h"
#include "Message.h"
#include "Misc.h"
#include "Output.h"
#include "Random.h"

void seed_rng()
{
	std::chrono::system_clock::rep seed =
		std::chrono::steady_clock::now().time_since_epoch().count();
	random_engine.seed((unsigned)seed);
}

// returns a random int between 0 and max-1.
int random(int max)
{
	std::uniform_int_distribution<int> dist(0, max - 1);
	
    return dist(random_engine);
}

CCard::operator std::string()
{
	return std::string(1, Ranks[rank]) + Suits[suit];
}

bool CCard::is_black()
{
	if (suit == SUIT_SPADE || suit == SUIT_CLUB)
		return true;
	return false;
}

bool CDeal::operator<(const CDeal& rDeal) const
{	
// This method was clever but inefficient:
/*	if (rDeal.empty() && empty())
		return tiebreak < rDeal.tiebreak;
	if (empty())
		return true;
	if (rDeal.empty())
		return false;

	if (front() < rDeal.front()) return true;
	if (rDeal.front() < front()) return false;

	CDeal lsubDeal(*this);
	CDeal rsubDeal(rDeal);

	lsubDeal.pop_front();
	rsubDeal.pop_front();*/

	CDeal::const_iterator lit = begin();
	CDeal::const_iterator rit = rDeal.begin();

	while (!(lit == end() && rit == rDeal.end()))
	{
		if (lit == end())
			return true;
		if (rit == rDeal.end())
			return false;

		if (*lit < *rit) return true;
		if (*rit < *lit) return false;

		lit++; rit++;
	}

	return (tiebreak < rDeal.tiebreak);
}

void CDeal::show_deal()
{
	std::string msg;
	for(CCard& it : *this)
		msg += std::string(it);
	more(msg);
}

std::string CDeal::rank_string()
{
	std::string str;
	for(CCard& it : *this)
		str += Ranks[it.rank];
	return str;
}

void CDeal::add_cards(int num)
{
	for(int i = 0;i < num;i++)
		push_back(CCard());
	sort();
	reverse();
}

int CDeal::count(suit_type suit)
{
	int total = 0;
	
	for(CCard& it : *this)
		if (it.suit == suit)
			total++;

	return total;
}

bool CDeal::contains_king()
{
	for(CCard& it : *this)
		if (it.rank == 12)
			return true;

	return false;
}

bool CDeal::contains_ace()
{
	for(CCard& it : *this)
		if (it.rank == 0)
			return true;

	return false;
}

bool CDeal::contains_ace_or_two()
{
	for(CCard& it : *this)
		if (it.rank <= 1)
			return true;

	return false;
}

bool CDeal::contains_face_card()
{
	for(CCard& it : *this)
		if (it.rank >= 10)
			return true;

	return false;
}

void CDeal::cut_highest(int num)
{
	if (size() == 0)
		return;

	sort();
	reverse();
	
	int i = 0;
	for(; i < num && size() > 1; i++)
		pop_front();
	
	if (size() > 1)
		return;
	
	CCard card(front());
	card.face_down = false;
	pop_front();

	for(int j = 0; j < (num - i); j++)
	{
		CCard tmpcard;
		if (tmpcard < card)
			card = tmpcard;
	}

	push_back(card);
}

int CDeal::total()
{
	int total = 0;
	for(CCard& it : *this)
		total += it.suit + 1;
	return total;
}

void CDeal::turn_face_up()
{
	for(CCard& card : *this)
		card.face_down = false;
}

CDeal::CDeal(int num_cards, int d, bool s, bool show)
{
	deal_num = d;
	
	do
	{
		tiebreak = random(1 << 20);
	} while (!tiebreakers.insert(tiebreak).second);

	if (num_cards < 1)
	{

		CDeal deal( -num_cards + 2, deal_num, true, false);
		push_back(deal.back());
	}
	else
	{
		for(int i = 0; i < num_cards; i++)
			push_back(CCard());
		if (s)
		{
			sort();
			reverse();
		}
	}

#ifdef _DEBUG_SHOW_DEALS
	if (show) show_deal();
#endif
}

std::set<int> CDeal::tiebreakers;
std::mt19937 random_engine;
