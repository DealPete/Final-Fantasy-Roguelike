#include <string>
#include <list>

#include "Map.h"

class CTown: public CFeature
{
public:
	static void load_towns(buffer& category);

	std::string name;
	int QL;

	CTown();
	CTown(const CTown& town);
	CTown(std::ifstream& savefile, feature_type ftype);
	
	void enter();
	void out(std::ofstream& savefile);

private:
	static std::list<std::string>::iterator it;
	static std::list<std::string> namelist;
};
