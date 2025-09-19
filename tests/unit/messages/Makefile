# Host-side build for random message/night mode unit tests
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 \
	-I../mocks \
	-I../../../
LDFLAGS ?=

BINS = \
	test_random_message_nightmode \
	test_random_message_stability

COMMON_SRCS = \
	../mocks/Arduino_time.cpp

all: $(BINS)

# test_random_message_nightmode
TEST_RANDOM_MESSAGE_SRCS = \
	test_random_message_nightmode.cpp \
	$(COMMON_SRCS)

test_random_message_nightmode: $(TEST_RANDOM_MESSAGE_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# test_random_message_stability
TEST_RANDOM_MESSAGE_STABILITY_SRCS = \
	test_random_message_stability.cpp \
	$(COMMON_SRCS)

test_random_message_stability: $(TEST_RANDOM_MESSAGE_STABILITY_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

run: all
	./test_random_message_nightmode
	./test_random_message_stability

clean:
	rm -f $(BINS)

.PHONY: all run clean
