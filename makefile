CXX= g++
CXXFLAGS= -std=c++11 -Wall -Wextra -Wreturn-type

all: a4tasks

a4tasks: a4tasks.cpp
	$(CXX) $(CXXFLAGS) a4tasks.cpp -o a4tasks

tar:
	tar -cvf submit.tar a4tasks.cpp makefile a4report.pdf Tokenizer.h 

clean:
	rm -rf *.o a4tasks 

# a3sdn: a3sdn.cpp Controller.cpp Switch.cpp Controller.h Switch.h FlowTableEntry.h SwitchTableEntry.h Tokenizer.h
# 	$(CXX) $(CXXFLAGS) a3sdn.cpp Controller.cpp Switch.cpp -o a3sdn
# tar:
# 	tar -cvf submit.tar a3sdn.cpp makefile a3report.pdf Controller.h Controller.cpp Switch.h Switch.cpp SwitchTableEntry.h FlowTableEntry.h Tokenizer.h makefifos.sh
# clean:
# 	rm -rf *.o a3sdn 
