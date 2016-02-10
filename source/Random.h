#ifndef __RANDOM_H_
#define __RANDOM_H_

#include "Misc.h"
#include "Output.h"

#include <list>
#include <random>
#include <set>
#include <string>

extern std::mt19937 random_engine;
void seed_rng();
int random(int max);

class CCard
{
public:
	int rank, suit;
	bool face_down;
	
	bool operator<(const CCard& rCard) const
	{
		if (rank < rCard.rank) return true;
		return false;
	}

	CCard(bool f = false)
	{
		rank = random(13);
		suit = random(4);
		face_down = f;
	}

	CCard(int r, int s = 0, bool f = false) : rank(r), suit(s),
		face_down(f)
	{}

	//CCard(const CCard& card) : rank(card.rank), suit(card.suit)
	//{}

	bool is_black();

	operator std::string();
};

class CDeal : public std::list<CCard>
{
	// Each deal is genereated with an unique tiebreaker
	// that is used when two identical deals are compared.
	static std::set<int> tiebreakers;

public:
	int deal_num;
	int tiebreak;

	bool operator<(const CDeal& rDeal) const;

	void show_deal();
	std::string rank_string();

	// add num cards then sort the deal.
	void add_cards(int num);

	// return value: how many cards of the suit are in the deal?
	int count(suit_type suit);
	bool contains_king();
	bool contains_ace();
	bool contains_ace_or_two();
	bool contains_face_card();
	void cut_highest(int num);
	int total();

	void turn_face_up();
	
	// Parameters:
	//	num_cards: number of cards
	//	d: deal number (for creating an indexed set of deals)
	//	sort_deal: should the cards be sorted?
	//	show: whether to show the cards when _DEBUG_SHOW_DEALS is enabled.
	CDeal(int num_cards, int d = 0, bool sort_deal = true, bool show = true);

	~CDeal()
	{
		tiebreakers.erase(tiebreak);
	}
};

#endif
