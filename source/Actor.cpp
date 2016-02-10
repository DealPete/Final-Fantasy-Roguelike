#include "Actor.h"
#include "Combat.h"
#include "Misc.h"

std::string CActor::objp()
{
	switch(sex)
	{
	case SEX_MALE:
		return "him";
	
	case SEX_FEMALE:
		return "her";

	case SEX_NONE:
		return "it";
	}

	return "it";
}

std::string CActor::subp()
{
	switch(sex)
	{
	case SEX_MALE:
		return "he";
	
	case SEX_FEMALE:
		return "she";

	case SEX_NONE:
		return "it";
	}

	return "it";
}

std::string CActor::posp()
{
	switch(sex)
	{
	case SEX_MALE:
		return "his";
	
	case SEX_FEMALE:
		return "her";

	case SEX_NONE:
		return "its";
	}

	return "its";
}

std::string CActor::refp()
{
	switch(sex)
	{
	case SEX_MALE:
		return "himself";
	
	case SEX_FEMALE:
		return "herself";

	case SEX_NONE:
		return "itself";
	}

	return "itself";
}

int CActor::eVit()
{
	if (Vit < 1)
		return 1;
	return Vit;
}

int CActor::eAgi()
{
	return Agi;
}

int CActor::eInt()
{
	return Int;
}

int CActor::eAtk()
{
	return Atk;
}

int CActor::eHit()
{
	return Hit;
}

int CActor::eEva()
{
	return Eva;
}

bool CActor::defeated()
{
	return (status & (ST_KO | ST_PETRIFY) || cflag & CF_FLED);
}

unsigned long CActor::operator+=(unsigned long flag)
{
	return status |= flag;
}

long CActor::operator+=(long flag)
{
	return cflag |= flag;
}

unsigned long CActor::operator-=(unsigned long flag)
{
	return status &= ~flag;
}

long CActor::operator-=(long flag)
{
	return cflag &= ~flag;
}

bool CActor::operator==(unsigned long flag)
{
	return (~status & flag) == 0;
}

bool CActor::operator==(long flag)
{
	return (~cflag & flag) == 0;
}

bool CActor::operator!=(unsigned long flag)
{
	if ((flag == ST_NONE) && (status == ST_NONE))
		return false;
	return (status & flag) == 0;
}

bool CActor::operator!=(long flag)
{
	if ((flag == CF_NONE) && (cflag == CF_NONE))
		return false;
	return (cflag & flag) == 0;
}

unsigned long CActor::operator^=(unsigned long flag)
{
	return (status ^= flag);
}

long CActor::operator^=(long flag)
{
	return (cflag ^= flag);
}

unsigned long CActor::operator&=(unsigned long flag)
{
	return (status &= flag);
}

long CActor::operator&=(long flag)
{
	return (cflag &= flag);
}

bool CActor::operator&(unsigned long flag)
{
	return (status & flag) != 0;
}

bool CActor::operator&(long flag)
{
	return (cflag & flag) != 0;
}