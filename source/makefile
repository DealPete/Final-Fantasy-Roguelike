CC = g++
CXXFLAGS = -std=c++11 -ggdb -I"/host/programming/c++/include"
OBJECTS = Action.o Actor.o Armor.o Combat.o FFCG.o Init.o InitData.o Input.o Items.o Jobs.o Map.o Message.o Misc.o Monsters.o Output.o Party.o Player.o Quests.o Random.o Savefile.o Spells.o Target.o Town.o Weapons.o

all : ffcg

ffcg : $(OBJECTS)
	$(CC) $(CXXFLAGS) $(OBJECTS) -ltcodxx -o ../ffcg
#	$(CC) $(CXXFLAGS) $(OBJECTS) -lncurses -o ../ffcg

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
