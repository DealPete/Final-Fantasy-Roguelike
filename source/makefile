CC = g++
CXXFLAGS = -std=c++14 -ggdb
OBJECTS = Action.o Actor.o Armor.o Combat.o FFCG.o Init.o InitData.o Input.o Items.o Jobs.o Map.o Message.o Misc.o Monsters.o Output.o Party.o Player.o Quests.o Random.o Savefile.o Spells.o Target.o Town.o Weapons.o Window.o

all : ffcg

ffcg : $(OBJECTS) Output_curses.o
#   $(CC) $(CXXFLAGS) $(OBJECTS) Output_tcod.o -ltcodxx -o ../ffcg
	$(CC) $(CXXFLAGS) $(OBJECTS) Output_curses.o -lncurses -o ../ffcg

clean:
	rm -f *.o
