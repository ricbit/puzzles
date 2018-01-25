all : akari 0hh1


% : %.mip.cc
	g++ -std=c++11 $< -o $@ \
	-m64 -mtune=native -fomit-frame-pointer -O3 -Wall \
	-lm -lscip
