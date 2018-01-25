PUZZLES = akari 0hh1 stars hashi slither nurikabe

all : $(PUZZLES)


clear :
	rm $(PUZZLES)

% : %.cover.cc
	g++ --std=c++11 $< -o $@ \
	-m64 -mtune=native -fomit-frame-pointer -O3 -Wall

% : %.cp.cc
	g++ --std=c++11 $< -o $@ \
	-m64 -mtune=native -fomit-frame-pointer -O3 -Wall

% : %.mip.cc
	g++ -std=c++11 $< -o $@ \
	-m64 -mtune=native -fomit-frame-pointer -O3 -Wall \
	-lm -lscip
