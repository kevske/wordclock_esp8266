# Host-side build for NTP client unit tests
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 \
	-I../mocks \
	-I../../../
LDFLAGS ?=

# Individual test programs
BINS = \
	test_ntp_disconnect \
	test_ntp_recovery

COMMON_SRCS = \
	../mocks/Arduino_time.cpp

all: $(BINS)

# test_ntp_disconnect
TEST_NTP_DISCONNECT_SRCS = \
	test_ntp_disconnect.cpp \
	../../../ntp_client_plus.cpp \
	$(COMMON_SRCS)

test_ntp_disconnect: $(TEST_NTP_DISCONNECT_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# test_ntp_recovery
TEST_NTP_RECOVERY_SRCS = \
	test_ntp_recovery.cpp \
	../../../ntp_client_plus.cpp \
	$(COMMON_SRCS)

test_ntp_recovery: $(TEST_NTP_RECOVERY_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

run: all
	./test_ntp_disconnect
	./test_ntp_recovery

clean:
	rm -f $(BINS)

.PHONY: all run clean
