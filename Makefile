# Makefile for the Gomoku project
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -O2 -Wpedantic \
  -Wunused -Wunused-function -Wunused-variable -Wunused-parameter \
  -Wunreachable-code -Wshadow -Wconversion -Wmissing-declarations

# Enable parallel compilation by default
MAKEFLAGS += -j$(shell nproc)

# Verbose mode (show full commands)
ifdef VERBOSE
	Q =
else
	Q = @
endif

# Operating system detection
UNAME_S := $(shell uname -s)

# SFML configuration based on the system
ifeq ($(UNAME_S),Darwin)
    # macOS with Homebrew
    SFML_DIR = /opt/homebrew
    SFML_INCLUDE = -I$(SFML_DIR)/include
    SFML_LIBS = -L$(SFML_DIR)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    SFML_RPATH = -Wl,-rpath,$(SFML_DIR)/lib
else
    # Linux: use a configurable prefix (default $HOME/local if it exists, otherwise /usr)
    SFML_DIR ?= $(shell if [ -d "$(HOME)/local" ]; then echo "$(HOME)/local"; else echo "/usr"; fi)
    SFML_INCLUDE = -I$(SFML_DIR)/include
    SFML_LIBS = -L$(SFML_DIR)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    SFML_RPATH = -Wl,-rpath,$(SFML_DIR)/lib
endif

# Project directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Target names
TARGET = bin/Gomoku                # GUI executable (SFML)
LIB_NAME = lib/libgomoku_logic.a   # static library logic/AI
TEST_BIN = bin/tests_runner        # test binary (without SFML)

# Source groups
CORE_SRC = \
	$(SRC_DIR)/gomoku/core/Board.cpp \
	$(SRC_DIR)/gomoku/core/Types.cpp \
	$(SRC_DIR)/gomoku/core/Logger.cpp \
	$(SRC_DIR)/gomoku/ai/MinimaxSearch.cpp \
	$(SRC_DIR)/gomoku/ai/MinimaxSearchEngine.cpp \
	$(SRC_DIR)/gomoku/ai/CandidateGenerator.cpp \
	$(SRC_DIR)/gomoku/application/SessionController.cpp \
	$(SRC_DIR)/gomoku/application/GameService.cpp \
	$(SRC_DIR)/gomoku/application/MoveValidator.cpp \

GUI_SRC = \
	main.cpp \
	$(SRC_DIR)/ui/Button.cpp \
	$(SRC_DIR)/scene/AScene.cpp \
	$(SRC_DIR)/scene/GameScene.cpp \
	$(SRC_DIR)/scene/GameSelect.cpp \
	$(SRC_DIR)/scene/Settings.cpp \
	$(SRC_DIR)/scene/MainMenu.cpp \
	$(SRC_DIR)/gui/GameWindow.cpp \
	$(SRC_DIR)/gui/GameBoardRenderer.cpp \
	$(SRC_DIR)/gui/ResourceManager.cpp \
	$(SRC_DIR)/util/Preferences.cpp

TEST_SRC = \
	tests/test_min.cpp

# Objects
CORE_OBJ = $(CORE_SRC:%.cpp=$(OBJ_DIR)/%.o)
GUI_OBJ  = $(GUI_SRC:%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJ = $(TEST_SRC:%.cpp=$(OBJ_DIR)/%.o)

# Generate dependency files (.d)
CXXFLAGS += -MMD
DEPFILES := $(CORE_OBJ:%.o=%.d) $(GUI_OBJ:%.o=%.d) $(TEST_OBJ:%.o=%.d)

# Default rule: check dependencies, build and install
all: check-deps-auto $(TARGET) install

# Internal rule to check and install SFML automatically
check-deps-auto:
	@echo "[AUTO-DEPS] Checking SFML dependencies..."
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		if ! brew list sfml >/dev/null 2>&1; then \
			echo "[AUTO-DEPS] SFML not found, installing via Homebrew..."; \
			$(MAKE) SFML; \
		else \
			echo "[AUTO-DEPS] SFML found via Homebrew"; \
		fi; \
	else \
		if [ ! -f "$(SFML_DIR)/lib/libsfml-graphics.so" ] && [ ! -f "$(SFML_DIR)/lib64/libsfml-graphics.so" ]; then \
			echo "[AUTO-DEPS] SFML not found in $(SFML_DIR), attempting installation..."; \
			$(MAKE) SFML; \
		else \
			echo "[AUTO-DEPS] SFML found in $(SFML_DIR)"; \
		fi; \
	fi

# Core static library (no SFML linking required)
$(LIB_NAME): $(CORE_OBJ)
	@mkdir -p $(dir $@)
	@echo "[AR] $@"
	$(Q)ar rcs $@ $(CORE_OBJ)

# Rule to build only (without auto-install)
build: $(TARGET)

# GUI executable: links the lib + SFML
$(TARGET): $(GUI_OBJ) $(LIB_NAME)
	@mkdir -p $(dir $@)
	@echo "[LD] $@"
	$(Q)$(CXX) $(GUI_OBJ) $(LIB_NAME) $(SFML_LIBS) $(SFML_RPATH) $(LDFLAGS) -o $@

# Tests: binary without SFML, linked against the core lib
$(TEST_BIN): $(TEST_OBJ) $(LIB_NAME)
	@echo "[LD] $@"
	$(Q)$(CXX) $(TEST_OBJ) $(LIB_NAME) -o $@

# Rule to compile objects (common)
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	$(Q)$(CXX) $(CXXFLAGS) -Isrc -I$(INCLUDE_DIR) $(SFML_INCLUDE) -c $< -o $@

# Debug rule (GUI + symbols)
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Install rule (optional)
install: $(TARGET)
	@echo "[INSTALL] Installing Gomoku"
	@echo "[INSTALL] Binary: $< -> ~/bin/"
	@mkdir -p ~/bin
	@cp $(TARGET) ~/bin/
	@cd desktop && ./install_desktop.sh

# Clean rule
clean: 
	@rm -rf $(BUILD_DIR)
	@rm -rf logs/

# Full clean rule
fclean: uninstall clean
	@rm -f $(TARGET) $(LIB_NAME) $(TEST_BIN)
	@if [ -f "$(HOME)/.config/gomoku/preferences.json" ]; then \
		rm -rf "$(HOME)/.config/gomoku"; \
		echo "[FCLEAN] Removed user config: $(HOME)/.config/gomoku/preferences.json"; \
	fi

# Uninstall rule - removes both binary and desktop integration
uninstall:
	@echo "[UNINSTALL] Removing Gomoku"
	@cd desktop && ./uninstall_desktop.sh
	@echo "[UNINSTALL] Removing binary from ~/bin/"
	@rm -f ~/bin/$(notdir $(TARGET))

# Rebuild rule
re: fclean
	@$(MAKE) all

# SFML setup - delegates to script for complex logic
SFML:
	@echo "[SFML] Setting up SFML"
	@./scripts/setup_sfml.sh

# Help rule
help:
	@echo "[HELP] Displaying help information"
	@echo "Gomoku Project Makefile"
	@echo ""
	@echo "Build Targets:"
	@echo "  all       - Check deps, build and install automatically (default)"
	@echo "  build     - Build main GUI executable only (no auto-install)"
	@echo "  lib       - Build core library only"
	@echo "  debug     - Build with debug symbols (-g -DDEBUG)"
	@echo "  test      - Build and run tests"
	@echo ""
	@echo "Clean Targets:"
	@echo "  clean     - Remove build directory"
	@echo "  fclean    - Remove build + binaries + libraries"
	@echo "  re        - Full rebuild (fclean + all)"
	@echo ""
	@echo "Setup Targets:"
	@echo "  SFML      - Check/install SFML dependencies"
	@echo "  check-deps- Check system dependencies"
	@echo ""
	@echo "Install Targets:"
	@echo "  install   - Install executable + desktop integration"
	@echo "  uninstall - Remove executable + desktop integration"
	@echo ""
	@echo "Options:"
	@echo "  VERBOSE=1 - Show full compiler commands"
	@echo ""
	@echo "System Info:"
	@echo "  System: $(UNAME_S)"
	@echo "  Compiler: $(CXX)"
	@echo "  Flags: $(CXXFLAGS)"
	@echo "  SFML: $(SFML_DIR)"
	@echo "  Parallel jobs: $(shell nproc) (automatic)"

# Rule to check dependencies
check-deps:
	@echo "[CHECK-DEPS]"
	@echo "System: $(UNAME_S)"
	@echo "SFML: $(SFML_DIR)"
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		if brew list sfml >/dev/null 2>&1; then \
			echo "SFML installed via Homebrew"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "SFML not installed via Homebrew"; \
			echo "Execute: brew install sfml"; \
		fi; \
	else \
		if [ -f "$(SFML_DIR)/lib/libsfml-graphics.so" ] || [ -f "$(SFML_DIR)/lib64/libsfml-graphics.so" ]; then \
			echo "SFML found in $(SFML_DIR)"; \
			ls $(SFML_DIR)/lib/libsfml* $(SFML_DIR)/lib64/libsfml* 2>/dev/null | head -3; \
		else \
			echo "SFML not found in $(SFML_DIR)"; \
			echo "Install SFML (ex: sudo apt install libsfml-dev) or adjust SFML_DIR"; \
		fi; \
	fi

# Convenient targets
lib: $(LIB_NAME)

# Build and run tests
test: $(TEST_BIN)
	./$(TEST_BIN) -v

# Environment variables for SFML (runtime)
# (Optional) Uncomment to propagate SFML libs path at runtime
# export LD_LIBRARY_PATH := $(SFML_DIR)/lib:$(LD_LIBRARY_PATH)
# export LD_LIBRARY_PATH := $(SFML_DIR)/lib64:$(LD_LIBRARY_PATH)

# Include dependency files only if they exist
-include $(wildcard $(DEPFILES))

# Phony rules
.PHONY: all build check-deps-auto debug clean fclean re install uninstall install-desktop uninstall-desktop help check-deps test SFML lib setup