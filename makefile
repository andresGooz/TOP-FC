all: TOP.cpp 
	g++ -Wall -o myprog TOP.cpp
	./myprog

clean: 
	$(RM) myprog

