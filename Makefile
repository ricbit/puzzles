PUZZLES = akari takuzu stars hashi slither nurikabe chaos domino suguru \
          torto branches
OPT = -m64 -mtune=native -fomit-frame-pointer -O3 -Wall -g
all : $(PUZZLES)


clear :
	rm $(PUZZLES)

% : %.cover.cc
	g++ --std=c++11 $< -o $@ $(OPT)

% : %.cp.cc
	g++ --std=c++11 $< -o $@ $(OPT)

% : %.mip.cc easyscip/easyscip.h
	g++ -std=c++11 $< -o $@ $(OPT) -lm -lscip
