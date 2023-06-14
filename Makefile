CC      = clang
LD      = clang
PIC     = -fPIC

CCFLAGS = $(PIC) -Wall -Wextra -std=c11 -g
LDFLAGS  = 


TEST_INCLUDES = -I3rd/tau
TEST_LIBS =

INCLUDES = $(TEST_INCLUDES)

SRC_DIR = src
TEST_DIR = test

SRCS = $(filter-out $(SRC_DIR)/scheme.c, $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c))
TEST_SRCS = $(wildcard $(TEST_DIR)/*_test.c)

OBJS = $(SRCS:.c=.o)
MAIN_OBJ = $(SRC_DIR)/scheme.o
TEST_OBJS = $(TEST_SRCS:.c=.o)

TARGET = scheme
TEST_TARGETS = $(TEST_SRCS:%.c=%)

.PHONY: clean clean_all test


all: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

test: $(TEST_TARGETS)
	@for test in $(TEST_TARGETS); do \
		./$$test; \
	done

$(TEST_TARGETS): %: $(OBJS) %.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	rm -f $(OBJS) $(TEST_OBJS)

clean_all: clean
	rm -f $(TARGET) $(TEST_TARGETS)

