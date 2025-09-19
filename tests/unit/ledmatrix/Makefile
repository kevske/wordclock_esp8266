# Simple host-side build for LEDMatrix unit tests
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 \
	-I../mocks \
	-I../../../
LDFLAGS ?=

SRCS = \
	test_color_math.cpp \
	ledmatrix_unit_translation.cpp

OBJS = $(SRCS:.cpp=.o)
BIN  = test_color_math

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJS) $(BIN)

.PHONY: all run clean
