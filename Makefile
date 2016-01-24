CC = clang++
CXX = clang++
CXXFLAGS = -O3 -std=c++11
LDFLAGS = -O3
SRCS = btv_main.cc challenge_loader.cc dice_roller.cc challenge.cc config_evaluator.cc

btv_main: $(SRCS:.cc=.o)

clean:
	rm -f btv_main *.o *~

.PHONY: clean
