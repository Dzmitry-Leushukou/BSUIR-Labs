SHELL := /bin/bash

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
LDFLAGS = -lncursesw
TARGET = file_manager
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

TEST_DIR ?= /tmp/file_manager_test
PERF_DIR ?= /tmp/perf
PERF_FILES ?= 100
PERF_FILE_SIZE_MB ?= 1
PERF_LOG ?= performance_test.log
KEY_DELAY ?= 0.2

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: test-exit test-performance-auto

test-exit: $(TARGET)
	@echo "[test-exit] Command scenario for correct termination"
	@echo "[test-exit] Running ./$(TARGET) $(TEST_DIR) and sending 'q' in pseudo-TTY..."
	@mkdir -p "$(TEST_DIR)"
	@command -v script >/dev/null 2>&1 || { echo "Error: 'script' utility is required (install util-linux)."; exit 1; }
	@(sleep $(KEY_DELAY); printf 'q') | TERM=xterm script -q -c "./$(TARGET) $(TEST_DIR)" /dev/null > /dev/null 2>&1; \
	status=$$?; \
	echo "[test-exit] Expected result: exit code 0"; \
	echo "[test-exit] Actual exit code: $$status"; \
	test $$status -eq 0

test-performance-data:
	@echo "[test-performance] Preparing performance dataset in $(PERF_DIR)"
	@echo "[test-performance] Parameters: PERF_FILES=$(PERF_FILES), PERF_FILE_SIZE_MB=$(PERF_FILE_SIZE_MB)"
	@mkdir -p "$(PERF_DIR)"
	@for i in $$(seq 1 $(PERF_FILES)); do \
		dd if=/dev/zero of="$(PERF_DIR)/file$$i" bs=1M count=$(PERF_FILE_SIZE_MB) status=none; \
	done
	@echo "[test-performance] Dataset is ready."

test-performance-auto: $(TARGET) test-performance-data
	@echo "[test-performance-auto] Running automated copy scenario in pseudo-TTY..."
	@command -v script >/dev/null 2>&1 || { echo "Error: 'script' utility is required (install util-linux)."; exit 1; }
	@test -x /usr/bin/time || { echo "Error: '/usr/bin/time' is required."; exit 1; }
	@time_file=$$(mktemp /tmp/file_manager_perf_time.XXXXXX); \
	copy_dir="$(PERF_DIR)_copy_$$(date +%Y%m%d_%H%M%S)"; \
	echo "[test-performance-auto] Copy destination: $$copy_dir"; \
	(sleep $(KEY_DELAY); printf 'c\025%s\nq' "$$copy_dir") | \
		TERM=xterm /usr/bin/time -f "real=%e\nuser=%U\nsys=%S\nexit=%x" -o "$$time_file" \
		script -q -c "./$(TARGET) $(PERF_DIR)" /dev/null > /dev/null 2>&1; \
	status=$$?; \
	real=$$(grep '^real=' "$$time_file" | cut -d= -f2); \
	user=$$(grep '^user=' "$$time_file" | cut -d= -f2); \
	sys=$$(grep '^sys=' "$$time_file" | cut -d= -f2); \
	exit_code=$$(grep '^exit=' "$$time_file" | cut -d= -f2); \
	total_mb=$$(($(PERF_FILES) * $(PERF_FILE_SIZE_MB))); \
	speed=$$(echo "scale=2; $$total_mb / $$real" | bc 2>/dev/null || echo "N/A"); \
	echo ""; \
	echo "========================================="; \
	echo "     PERFORMANCE TEST RESULTS"; \
	echo "========================================="; \
	echo "Test parameters:"; \
	echo "  Number of files:  $(PERF_FILES)"; \
	echo "  File size each:   $(PERF_FILE_SIZE_MB) MB"; \
	echo "  Total data:       $$total_mb MB"; \
	echo ""; \
	echo "Timing metrics:"; \
	echo "  Real time:        $$real s"; \
	echo "  User time:        $$user s"; \
	echo "  System time:      $$sys s"; \
	echo "  Copy speed:       $$speed MB/s"; \
	echo ""; \
	echo "Copy verification:"; \
	copy_exists=0; \
	file_count=0; \
	if test -d "$$copy_dir"; then \
		copy_exists=1; \
		file_count=$$(find "$$copy_dir" -type f | wc -l); \
		echo "  Destination dir:  $$copy_dir"; \
		echo "  Files copied:     $$file_count / $(PERF_FILES)"; \
	else \
		echo "  Destination dir:  NOT CREATED"; \
	fi; \
	echo "  Exit code:        $$exit_code (expected 0)"; \
	echo "========================================="; \
	echo ""; \
	{ \
		echo "timestamp=$$(date -Iseconds)"; \
		echo "scenario=auto_copy_directory_then_exit"; \
		echo "perf_dir=$(PERF_DIR)"; \
		echo "copy_dir=$$copy_dir"; \
		echo "perf_files=$(PERF_FILES)"; \
		echo "perf_file_size_mb=$(PERF_FILE_SIZE_MB)"; \
		echo "total_data_mb=$$total_mb"; \
		echo "real=$$real"; \
		echo "user=$$user"; \
		echo "sys=$$sys"; \
		echo "speed_mb_s=$$speed"; \
		echo "exit_code=$$exit_code"; \
		echo "status=$$status"; \
		echo "copy_exists=$$copy_exists"; \
		echo "file_count=$$file_count"; \
		echo "---"; \
	} >> "$(PERF_LOG)"; \
	rm -f "$$time_file"; \
	echo "[test-performance-auto] Expected: status=0, copy_dir exists, file_count>0"; \
	echo "[test-performance-auto] Actual status: $$status"; \
	echo "[test-performance-auto] Copy directory exists: $$copy_exists"; \
	echo "[test-performance-auto] Number of files copied: $$file_count (expected >= $(PERF_FILES))"; \
	echo "[test-performance-auto] Created copy directory: $$copy_dir"; \
	test $$status -eq 0 && test $$copy_exists -eq 1 && test $$file_count -ge $(PERF_FILES)

.PHONY: all clean run test test-exit test-performance-data test-performance-auto