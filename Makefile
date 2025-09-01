# Makefile pour le projet Gomoku
# Compilateur et flags
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -O2

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
    # Linux (configuration originale)
    SFML_DIR = /home/jgavairo/local
    SFML_INCLUDE = -I$(SFML_DIR)/include
    SFML_LIBS = -L$(SFML_DIR)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    SFML_RPATH = -Wl,-rpath,$(SFML_DIR)/lib
endif

# Dossiers du projet
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Nom de l'exécutable
TARGET = Gomoku

# Sources
SRC_FILES = $(wildcard *.cpp) \
            $(wildcard $(SRC_DIR)/*.cpp) \
            $(wildcard $(SRC_DIR)/*/*.cpp) \
            $(wildcard $(SRC_DIR)/*/*/*.cpp)

# Objets
OBJ_FILES = $(SRC_FILES:%.cpp=$(OBJ_DIR)/%.o)

# Règles principales
all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@echo "🔗 Linking $(TARGET)..."
	@mkdir -p $(dir $@)
	$(CXX) $(OBJ_FILES) $(SFML_LIBS) $(SFML_RPATH) -o $@
	@echo "✅ $(TARGET) créé avec succès !"

# Règle pour compiler les objets
$(OBJ_DIR)/%.o: %.cpp
	@echo "🔨 Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SFML_INCLUDE) -I$(INCLUDE_DIR) -c $< -o $@

# Règle debug
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)
	@echo "🐛 Version debug créée !"

# Règle clean
clean:
	@echo "🧹 Nettoyage des fichiers objets..."
	@rm -rf $(BUILD_DIR)
	@echo "✅ Nettoyage terminé !"

# Règle fclean
fclean: clean
	@echo "🗑️  Suppression de l'exécutable..."
	@rm -f $(TARGET)
	@echo "✅ Nettoyage complet terminé !"

# Règle re
re: fclean all
	@echo "🔄 Recompilation complète terminée !"

# Règle install (optionnelle)
install: $(TARGET)
	@echo "📦 Installation de $(TARGET)..."
	@mkdir -p ~/bin
	@cp $(TARGET) ~/bin/
	@echo "✅ $(TARGET) installé dans ~/bin/"

# Règle SFML - Installation de SFML et dépendances
SFML:
	@echo "🎮 Vérification de SFML..."
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		echo "🍎 Système macOS détecté"; \
		if brew list sfml >/dev/null 2>&1; then \
			echo "✅ SFML installé via Homebrew"; \
			echo "📚 Bibliothèques disponibles:"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "📦 Installation de SFML via Homebrew..."; \
			brew install sfml; \
			echo "✅ SFML installé avec succès !"; \
		fi; \
	else \
		if [ -d "$(SFML_DIR)" ] && [ -f "$(SFML_DIR)/lib/libsfml-graphics.so" ]; then \
			echo "✅ SFML est déjà installé dans $(SFML_DIR)"; \
			echo "📚 Bibliothèques disponibles:"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "📦 SFML non trouvé, installation en cours..."; \
			if [ -f "./scripts/install_sfml.sh" ]; then \
				./scripts/install_sfml.sh; \
			else \
				echo "❌ Script scripts/install_sfml.sh non trouvé !"; \
			fi; \
		fi; \
	fi
	@echo "🔍 Vérification des dépendances..."; \
	make check-deps

# Règle uninstall
uninstall:
	@echo "🗑️  Désinstallation de $(TARGET)..."
	@rm -f ~/bin/$(TARGET)
	@echo "✅ $(TARGET) désinstallé !"

# Règle help
help:
	@echo "🎮 Makefile pour le projet Gomoku"
	@echo ""
	@echo "📋 Règles disponibles :"
	@echo "  all      - Compile le projet (défaut)"
	@echo "  debug    - Compile en mode debug"
	@echo "  clean    - Supprime les fichiers objets"
	@echo "  fclean   - Supprime tout (objets + exécutable)"
	@echo "  re       - Recompilation complète"
	@echo "  SFML     - Installe SFML et dépendances"
	@echo "  install  - Installe dans ~/bin/"
	@echo "  uninstall- Désinstalle de ~/bin/"
	@echo "  help     - Affiche cette aide"
	@echo ""
	@echo "🔧 Configuration :"
	@echo "  Système: $(UNAME_S)"
	@echo "  Compilateur: $(CXX)"
	@echo "  Flags: $(CXXFLAGS)"
	@echo "  SFML: $(SFML_DIR)"

# Règle pour vérifier les dépendances
check-deps:
	@echo "🔍 Vérification des dépendances..."
	@echo "Système: $(UNAME_S)"
	@echo "SFML: $(SFML_DIR)"
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		if brew list sfml >/dev/null 2>&1; then \
			echo "✅ SFML installé via Homebrew"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "❌ SFML non installé via Homebrew"; \
			echo "💡 Exécutez: brew install sfml"; \
		fi; \
	else \
		if [ -d "$(SFML_DIR)" ]; then \
			echo "✅ SFML trouvé dans $(SFML_DIR)"; \
			ls $(SFML_DIR)/lib/libsfml* 2>/dev/null | head -3; \
		else \
			echo "❌ SFML non trouvé dans $(SFML_DIR)"; \
		fi; \
	fi

# Règle pour tester la compilation
test: $(TARGET)
	@echo "🧪 Test de compilation réussi !"
	@echo "Exécutable créé: $(TARGET)"

# Variables d'environnement pour SFML
export LD_LIBRARY_PATH := $(SFML_DIR)/lib:$(LD_LIBRARY_PATH)

# Règle par défaut
.PHONY: all debug clean fclean re install uninstall help check-deps test SFML 