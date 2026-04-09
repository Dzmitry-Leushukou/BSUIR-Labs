CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./src
TARGET = morsec
SRCDIR = src
BUILDDIR = build
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/morse.c
OBJECTS = $(BUILDDIR)/main.o $(BUILDDIR)/morse.o

TEST_INPUT = tests/test_input.txt
TEST_OUTPUT = tests/test_output.txt
TEST_EXPECTED = tests/test_output_expected.txt

.PHONY: all clean distclean test run help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

distclean: clean
	rm -f $(TEST_OUTPUT)

test: $(TARGET)
	@echo "Running test..."
	@mkdir -p tests
	@./$(TARGET) $(TEST_INPUT) -o $(TEST_OUTPUT)
	@if diff -q $(TEST_OUTPUT) $(TEST_EXPECTED) > /dev/null; then \
		echo "Test PASSED"; \
	else \
		echo "Test FAILED: output differs from expected"; \
		diff $(TEST_OUTPUT) $(TEST_EXPECTED); \
		exit 1; \
	fi

run: $(TARGET)
	@echo "=== Example usage ==="
	@echo "Input: HELLO WORLD"
	@echo "HELLO WORLD" | ./$(TARGET)
	@echo ""
	@echo "Input from file $(TEST_INPUT):"
	@./$(TARGET) $(TEST_INPUT)

help:
	@echo "Available targets:"
	@echo "  all      - build $(TARGET) (default)"
	@echo "  clean    - remove object files and executable"
	@echo "  distclean- clean and remove test output"
	@echo "  test     - run test with predefined input/expected output"
	@echo "  run      - demonstrate program on sample input"
	@echo "  help     - show this message"