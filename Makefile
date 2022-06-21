CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -pthread

TARGET = zzz
OBJ = threadsafe_map_test.o threadsafe_list_test.o main.o

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o

re: clean all

threadsafe_map_test.o: threadsafe_map.h threadsafe_map_test.cpp
	$(CXX) $(CXXFLAGS) -c threadsafe_map_test.cpp

threadsafe_list_test.o: threadsafe_list.h threadsafe_list_test.cpp
	$(CXX) $(CXXFLAGS) -c threadsafe_list_test.cpp

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)
