CXX= g++
CXXFLAGS= -std=c++11 -Wall -Wextra -Wreturn-type

all: a4tasks

a4tasks: a4tasks.cpp
	$(CXX) $(CXXFLAGS) -pthread a4tasks.cpp -o a4tasks

tar:
	tar -cvf submit.tar a4tasks.cpp Resource.h Task.h makefile a4report.pdf Tokenizer.h 

clean:
	rm -rf *.o a4tasks 

