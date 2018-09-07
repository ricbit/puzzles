PUZZLES = akari.mip takuzu.mip stars.cover hashi.cp slither.cp nurikabe.mip \
          chaos.cover domino.cover suguru.mip torto.mip branches.mip \
          branches.cover agents.greedy agents.mip snail.human
OPT = -m64 -mtune=native -fomit-frame-pointer -Wall -g -O3
all : $(PUZZLES)

clear :
	rm -f $(PUZZLES) log.txt lixo *.dot

%.cover : %.cover.cc
	g++ --std=c++17 $< -o $@ $(OPT)

%.human : %.human.cc
	g++ --std=c++17 $< -o $@ $(OPT)

%.greedy : %.greedy.cc
	g++ --std=c++17 $< -o $@ $(OPT)

%.cp : %.cp.cc
	g++ --std=c++17 $< -o $@ $(OPT)

%.mip : %.mip.cc easyscip/easyscip.h
	g++ -std=c++17 $< -o $@ $(OPT) -lm -lscip
