# 컴파일러 및 옵션
CXX = g++
CXXFLAGS = -std=c++17
LDLIBS = -lncurses

# 소스 및 타겟
SRC = main.cpp SnakeGame.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = SnakeGame

# 기본 규칙
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDLIBS) $(CXXFLAGS)

%.o: %.cpp SnakeGame.h Constants.h Point.h
	$(CXX) $(CXXFLAGS) -c $<
	
clean:
	rm -f $(TARGET) $(OBJ) 
# 실행 규칙
run :
	./$(TARGET)
