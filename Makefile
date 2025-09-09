# Makefile pour le projet Gomoku
# Compilateur et flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -O2

# Détection du système d'exploitation
UNAME_S := $(shell uname -s)

# Configuration SFML selon le système
ifeq ($(UNAME_S),Darwin)
    # macOS avec Homebrew
    SFML_DIR = /opt/homebrew
    SFML_INCLUDE = -I$(SFML_DIR)/include
    SFML_LIBS = -L$(SFML_DIR)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    SFML_RPATH = -Wl,-rpath,$(SFML_DIR)/lib
else
	# Linux: utiliser un préfixe configurable (par défaut $HOME/local s'il existe, sinon /usr)
	SFML_DIR ?= $(shell if [ -d "$(HOME)/local" ]; then echo "$(HOME)/local"; else echo "/usr"; fi)
	SFML_INCLUDE = -I$(SFML_DIR)/include
	SFML_LIBS = -L$(SFML_DIR)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	SFML_RPATH = -Wl,-rpath,$(SFML_DIR)/lib
endif

# Dossiers du projet
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Noms des cibles
TARGET = bin/Gomoku                # exécutable GUI (SFML)
LIB_NAME = lib/libgomoku_core.a        # bibliothèque statique logique/IA
TEST_BIN = bin/tests_runner        # binaire de tests (sans SFML)
CLI_BIN = bin/cli                  # exécutable CLI (sans SFML)

# Groupes de sources
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

# Générer les fichiers de dépendances (.d)
CXXFLAGS += -MMD
DEPFILES := $(CORE_OBJ:%.o=%.d) $(GUI_OBJ:%.o=%.d) $(TEST_OBJ:%.o=%.d) $(CLI_OBJ:%.o=%.d)
-include $(DEPFILES)

# Règle par défaut: construire l'exécutable GUI
all: $(TARGET)

# Bibliothèque statique du core (pas de lien SFML requis)
$(LIB_NAME): $(CORE_OBJ)
	@mkdir -p $(dir $@)
	@echo "[AR] $@"
	ar rcs $@ $(CORE_OBJ)

# Exécutable GUI: lie la lib + SFML
$(TARGET): $(GUI_OBJ) $(LIB_NAME)
	@mkdir -p $(dir $@)
	@echo "[LD] $@"
	$(CXX) $(GUI_OBJ) $(LIB_NAME) $(SFML_LIBS) $(SFML_RPATH) $(LDFLAGS) -o $@

# Tests: binaire sans SFML, lié contre la lib core
$(TEST_BIN): $(TEST_OBJ) $(LIB_NAME)
	@echo "[LD] $@"
	$(CXX) $(TEST_OBJ) $(LIB_NAME) -o $@

# CLI: exécutable sans SFML, lié contre la lib core
$(CLI_BIN): $(CLI_OBJ) $(LIB_NAME)
	@echo "[LD] $@"
	$(CXX) $(CLI_OBJ) $(LIB_NAME) -o $@

# Règle pour compiler les objets (commune)
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	$(CXX) $(CXXFLAGS) -Isrc -I$(INCLUDE_DIR) $(SFML_INCLUDE) -c $< -o $@

# Règle debug (GUI + symboles)
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Règle clean
clean:
	@rm -rf $(BUILD_DIR)

# Règle fclean
fclean: clean
	@rm -f $(TARGET) $(LIB_NAME) $(TEST_BIN) $(CLI_BIN)

# Règle re
re: fclean all

# Règle install (optionnelle)
install: $(TARGET)
	@echo "[INSTALL] $< -> ~/bin/"
	@mkdir -p ~/bin
	@cp $(TARGET) ~/bin/

# Règle SFML - Installation de SFML et dépendances
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

# Règle uninstall
uninstall:
	@rm -f ~/bin/$(notdir $(TARGET))

# Règle help
help:
	@echo "Makefile Gomoku"
	@echo "Targets: all | lib | $(TARGET) | $(TEST_BIN) | $(CLI_BIN) | test | debug | clean | fclean | re | SFML | install | uninstall | help"
	@echo "System: $(UNAME_S)  CXX: $(CXX)  CXXFLAGS: $(CXXFLAGS)  SFML: $(SFML_DIR)"

# Règle pour vérifier les dépendances
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

# Cibles pratiques
lib: $(LIB_NAME)

# Construire et exécuter les tests
test: $(TEST_BIN)
	./$(TEST_BIN) -v

cli: $(CLI_BIN)
	./$(CLI_BIN) -v

# Variables d'environnement pour SFML (runtime)
# (Optionnel) Décommentez pour propager le chemin des libs SFML au runtime
# export LD_LIBRARY_PATH := $(SFML_DIR)/lib:$(LD_LIBRARY_PATH)
# export LD_LIBRARY_PATH := $(SFML_DIR)/lib64:$(LD_LIBRARY_PATH)

# Inclure les fichiers de dépendances (.d)
-include $(shell find $(OBJ_DIR) -name "*.d" 2>/dev/null)

# Règles phony
.PHONY: all debug clean fclean re install uninstall help check-deps test SFML lib cli