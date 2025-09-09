# Gomoku project Makefile
# flags and Compiler
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -O2

# Operating system detection
UNAME_S := $(shell uname -s)

# SFML detection: rely on pkg-config (assumed present on target systems)
# This simplifies flags and avoids manual SFML_DIR handling.
SFML_CFLAGS := $(shell pkg-config --cflags sfml-graphics sfml-window sfml-system sfml-audio)
SFML_LIBS   := $(shell pkg-config --libs  sfml-graphics sfml-window sfml-system sfml-audio)
SFML_RPATH  :=

# Project directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Target names
TARGET = bin/Gomoku                # GUI executable (SFML)
LIB_NAME = lib/libgomoku_core.a        # Static library for logic/AI
TEST_BIN = bin/tests_runner        # Test binary (without SFML)
CLI_BIN = bin/cli                  # CLI executable (without SFML)

# Source groups
CORE_SRC = \
	$(SRC_DIR)/core/Board.cpp \
	$(SRC_DIR)/ai/Search.cpp \
	$(SRC_DIR)/app/Engine.cpp \
	$(SRC_DIR)/app/GameSession.cpp \
	$(SRC_DIR)/app/Notation.cpp

GUI_SRC = \
	main.cpp \
	$(SRC_DIR)/gui/GameWindow.cpp \
	$(SRC_DIR)/gui/GameBoardRenderer.cpp \
	$(SRC_DIR)/scene/AScene.cpp \
	$(SRC_DIR)/scene/GameScene.cpp \
	$(SRC_DIR)/scene/GameSelect.cpp \
	$(SRC_DIR)/scene/MainMenu.cpp \
	$(SRC_DIR)/ui/Button.cpp \
	$(SRC_DIR)/gui/RessourceManager.cpp

TEST_SRC = \
	tests/test_min.cpp

CLI_SRC = \
	src/cli/cli.cpp

# Objets
CORE_OBJ = $(CORE_SRC:%.cpp=$(OBJ_DIR)/%.o)
GUI_OBJ  = $(GUI_SRC:%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJ = $(TEST_SRC:%.cpp=$(OBJ_DIR)/%.o)
CLI_OBJ = $(CLI_SRC:%.cpp=$(OBJ_DIR)/%.o)

# Generate dependency files (.d)
CXXFLAGS += -MMD
DEPFILES := $(CORE_OBJ:%.o=%.d) $(GUI_OBJ:%.o=%.d) $(TEST_OBJ:%.o=%.d) $(CLI_OBJ:%.o=%.d)
-include $(DEPFILES)

# Default rule: build the GUI executable
all: $(TARGET)

# Static library for core (no SFML linking required)
$(LIB_NAME): $(CORE_OBJ)
	@mkdir -p $(dir $@)
	@echo "[AR] $@"
	ar rcs $@ $(CORE_OBJ)

# GUI executable: links the lib + SFML
$(TARGET): $(GUI_OBJ) $(LIB_NAME)
	@mkdir -p $(dir $@)
	@echo "[LD] $@"
	$(CXX) $(GUI_OBJ) $(LIB_NAME) $(SFML_LIBS) $(SFML_RPATH) $(LDFLAGS) -o $@

# Tests: binary without SFML, linked against the core lib
$(TEST_BIN): $(TEST_OBJ) $(LIB_NAME)
	@echo "[LD] $@"
	$(CXX) $(TEST_OBJ) $(LIB_NAME) -o $@

# CLI: executable without SFML, linked against the core lib
$(CLI_BIN): $(CLI_OBJ) $(LIB_NAME)
	@echo "[LD] $@"
	$(CXX) $(CLI_OBJ) $(LIB_NAME) -o $@

# Rule to compile objects (common)
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	$(CXX) $(CXXFLAGS) -Isrc -I$(INCLUDE_DIR) $(SFML_CFLAGS) -c $< -o $@

# Debug rule (GUI + symbols)
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Clean rule
clean:
	@rm -rf $(BUILD_DIR)

# Fclean rule
fclean: clean
	@rm -f $(TARGET) $(LIB_NAME) $(TEST_BIN) $(CLI_BIN)

# Re rule
re: fclean all

# Install rule (optional)
install: $(TARGET)
	@echo "[INSTALL] $< -> ~/bin/"
	@mkdir -p ~/bin
	@cp $(TARGET) ~/bin/

# SFML rule - Install SFML and dependencies
SFML:
	@echo "Vérification de SFML..."
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		echo "Système macOS détecté"; \
		if brew list sfml >/dev/null 2>&1; then \
			echo "SFML installé via Homebrew"; \
			echo "Bibliothèques disponibles:"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "Installation de SFML via Homebrew..."; \
			brew install sfml; \
			echo "SFML installé avec succès !"; \
		fi; \
	else \
		if [ -d "$(SFML_DIR)" ] && [ -f "$(SFML_DIR)/lib/libsfml-graphics.so" ]; then \
			echo "SFML est déjà installé dans $(SFML_DIR)"; \
			echo "Bibliothèques disponibles:"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "SFML non trouvé, installation en cours..."; \
			if [ -f "./scripts/install_sfml.sh" ]; then \
				./scripts/install_sfml.sh; \
			else \
				echo "Script scripts/install_sfml.sh non trouvé !"; \
			fi; \
		fi; \
	fi
	@echo "Vérification des dépendances..."; \
	make check-deps

# Uninstall rule
uninstall:
	@rm -f ~/bin/$(notdir $(TARGET))

# Help rule
help:
	@echo "Makefile Gomoku"
	@echo "Targets: all | lib | $(TARGET) | $(TEST_BIN) | $(CLI_BIN) | test | debug | clean | fclean | re | SFML | install | uninstall | help"
	@echo "System: $(UNAME_S)  CXX: $(CXX)  CXXFLAGS: $(CXXFLAGS)  SFML: $(SFML_DIR)"

# RRule to check dependencies
check-deps:
	@echo "Check deps"
	@echo "System: $(UNAME_S)"
	@echo "SFML: $(SFML_DIR)"
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		if brew list sfml >/dev/null 2>&1; then \
			echo "SFML installé via Homebrew"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "SFML non installé via Homebrew"; \
			echo "Executez: brew install sfml"; \
		fi; \
	else \
		if [ -f "$(SFML_DIR)/lib/libsfml-graphics.so" ] || [ -f "$(SFML_DIR)/lib64/libsfml-graphics.so" ]; then \
			echo "SFML trouvé dans $(SFML_DIR)"; \
			ls $(SFML_DIR)/lib/libsfml* $(SFML_DIR)/lib64/libsfml* 2>/dev/null | head -3; \
		else \
			echo "SFML non trouvé dans $(SFML_DIR)"; \
			echo "Installez SFML (ex: sudo apt install libsfml-dev) ou ajustez SFML_DIR"; \
		fi; \
	fi

# Practical targets
lib: $(LIB_NAME)

# Build and run tests
test: $(TEST_BIN)
	./$(TEST_BIN) -v

cli: $(CLI_BIN)
	./$(CLI_BIN) -v

# Environment variables for SFML (runtime)
# (Optional) Uncomment to propagate the SFML libs path at runtime
# export LD_LIBRARY_PATH := $(SFML_DIR)/lib:$(LD_LIBRARY_PATH)
# export LD_LIBRARY_PATH := $(SFML_DIR)/lib64:$(LD_LIBRARY_PATH)

# Include dependency files (.d)
-include $(shell find $(OBJ_DIR) -name "*.d" 2>/dev/null)

# Phony rules
.PHONY: all debug clean fclean re install uninstall help check-deps test SFML lib cli