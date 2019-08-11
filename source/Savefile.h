#include <fstream>

#include <string>
#include <vector>

const int max_save_file_num = 255;

class CSaveHeader
{
public:
	std::vector<CPlayer> player;

	std::string fname;

	int Gil;
	double playing_time;

	std::string savetime;

	CSaveHeader(std::ifstream&);

	void describe(_wintype*);
};

class CSaveHeaderWindow : public CSelectWindow
{
public:
	std::vector<CSaveHeader> SaveHeader;

	CSaveHeaderWindow(_wintype* w, bool p = true, int m = 18)
		: CSelectWindow(w, p, true, m, INPUT_START)
	{}

	void describe(_wintype*);
	void empty_menu(_wintype*);
};

void set_new_savefile();
bool save_game();
bool restore_game();
