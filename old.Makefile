# Build simple (GNU Make). Génère l'exécutable 'Gomoku'.
CXX      := g++
CXXFLAGS := -std=c++23 -O2 -Wall -Wextra -Wpedantic -MMD -MP
INCLUDES := -Iinclude
SRC_DIRS := src/core src/ai src/app src/cli
OBJS     := $(patsubst src/%.cpp,build/%.o,$(shell find src -name '*.cpp'))
NAME     := Gomoku

all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf build

fclean: clean
	@rm -f $(NAME)

re: fclean all

# Debug build with AddressSanitizer/UndefinedBehaviorSanitizer
DBG_CXXFLAGS := -std=c++23 -O0 -g -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS \
				-fsanitize=address,undefined

.PHONY: debug asan
debug: CXXFLAGS := $(DBG_CXXFLAGS)
debug: clean $(NAME)

asan: debug

.PHONY: all clean fclean re

# Auto-deps
-include $(OBJS:.o=.d)

# --- Tests ---
TEST_EXE := build/tests/test_min
TEST_SRCS := $(wildcard tests/*.cpp)

.PHONY: test
test: $(TEST_EXE)
	@$(TEST_EXE) $(ARGS)

$(TEST_EXE): $(TEST_SRCS) $(filter-out build/cli/main.o,$(OBJS))
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(filter-out build/cli/main.o,$(OBJS)) $(TEST_SRCS)
