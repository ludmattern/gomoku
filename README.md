# 🧠 Gomoku IA - Intelligence Artificielle de Jeu de Stratégie

**Statut:** 🚧 **IN PROGRESS** 🚧

## 📋 Description

Ce projet consiste à développer un jeu **Gomoku** jouable, integrant une intelligence artificielle capable de battre des joueurs humains. **Gomoku** un jeu de stratégie traditionnel joué sur un plateau de Go. L'IA utilise l'algorithme **Min-Max** avec une heuristique optimisée pour prendre des décisions rapides et efficaces.

**Technologie :** Développé en **C++** avec une architecture orientée objet moderne.

## 🎯 Objectifs

- Developper le jeu Gomoku en c++
- Développer une interface graphique intuitive et agréable
- Créer une IA imbattable au Gomoku
- Implémenter l'algorithme Min-Max avec une heuristique performante
- Respecter les contraintes de performance (≤ 0.5s par coup en moyenne)

## 🎮 Règles du Jeu

### Règles de base
- **Plateau :** 19x19
- **Victoire :** Aligner 5 pierres ou plus
- **Joueurs :** 2 joueurs alternent pour placer leurs pierres

### Règles spéciales implémentées

#### 🎯 **Captures (Ninuki-renju/Pente)**
- Capturer une paire de pierres adverses en les encerclant
- **Condition de victoire :** Capturer 10 pierres adverses au total
- Les intersections libérées peuvent être rejouées

#### 🏁 **Capture en fin de partie**
- Un alignement de 5 ne gagne que si l'adversaire ne peut pas le briser par capture
- Si un joueur a déjà perdu 4 paires et que l'adversaire peut en capturer une 5ème, victoire par capture

#### 🚫 **Interdiction des double-trois**
- Interdiction de jouer un coup qui crée deux alignements de trois libres simultanément
- Exception : Les double-trois créés par capture sont autorisés

## 🛠️ Architecture Technique

### Technologies utilisées
- **Langage :** C++
- **Bibliothèques graphiques :** A definir 
- **Compilation :** Compilateur a definir

### Algorithme Principal
- **Min-Max** avec élagage alpha-bêta
- **Heuristique personnalisée** pour l'évaluation des positions
- **Arbre de recherche** optimisé pour la performance
- **Classes modulaires** pour une maintenance facilitée

### Composants principaux
```
📦 Gomoku/
├── 🧠 src/ia/             # Intelligence artificielle
├── 🎮 src/game/          # Logique de jeu
├── 🖼️  src/gui/           # Interface graphique
├── 📁 src/utils/         # Utilitaires
├── 📁 include/           # Headers publics
└── 📄 main.cpp           # Point d'entrée
```

### Architecture des Classes (OOP)
```cpp
🏗️  Architecture Orientée Objet
├── 🎯 class Game          # Contrôleur principal du jeu
├── 🏁 class Board         # Représentation du plateau 19x19
├── 🧠 class AI            # Intelligence artificielle
├── 👤 class Player        # Gestion des joueurs (Humain/IA)
├── 🎮 class GameEngine    # Logique du jeu et règles
├── 🖼️  class GUI           # Interface graphique
├── ⏱️  class Timer         # Mesure de performance
└── 🐛 class Debug         # Outils de débogage
```

## ⚡ Fonctionnalités

### Partie Obligatoire
- [ ] **Exécutable :** `Gomoku`
- [ ] **Menu :** Menu Principal
- [ ] **Humain vs Humain :** Mode deux joueurs avec suggestions de l'IA
- [ ] **IA vs Humain :** Jeu contre l'intelligence artificielle
- [ ] **Interface graphique :** Interface utilisable et agréable
- [ ] **Timer de performance :** Affichage du temps de réflexion de l'IA
- [ ] **Mode debug :** Visualisation du processus de décision de l'IA
- [ ] **Makefile :** Compilation avec règles standard

### Partie Bonus (à implémenter après validation de la partie obligatoire)
- [ ] **Fonctionnalités supplémentaires :** À définir

## 🔧 Compilation et Installation

### Prérequis
- **Compilateur :** g++ ou clang++ (support C++17)
- **Bibliothèque graphique :** SDL2 / SFML / Qt
- **Make :** Version récente

### Compilation
```bash
# Compilation standard
make

# Compilation avec optimisations
make all

# Compilation en mode debug
make debug

# Nettoyage des fichiers objets
make clean

# Nettoyage complet
make fclean

# Recompilation complète
make re
```

### Flags de compilation recommandés
```makefile
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -O2
```

## 🚀 Utilisation

```bash
# Lancement du jeu
./Gomoku

# Mode debug (si implémenté)
./Gomoku --debug

# Aide
./Gomoku --help
```

## 📊 Contraintes de Performance

- **Temps de réflexion :** ≤ 0.5 seconde par coup (en moyenne)
- **Robustesse :** Aucun crash autorisé, même en cas de manque de mémoire
- **Optimisation :** Profondeur de recherche et heuristique optimisées

## 🎯 Critères d'Évaluation

### Algorithme Min-Max
- Implémentation correcte et complète
- Compréhension approfondie du fonctionnement
- Explication détaillée lors de la soutenance

### Heuristique
- Précision de l'évaluation des positions
- Rapidité d'exécution
- Capacité d'adaptation aux stratégies adverses

### Règles de jeu
- Implémentation correcte de toutes les règles spécifiées
- Gestion des cas particuliers (captures, double-trois, etc.)

## 🐛 Debug et Développement

Le projet inclut un système de debug permettant de :
- Visualiser l'arbre de recherche de l'IA
- Analyser les évaluations heuristiques
- Comprendre le processus de décision
- Optimiser les performances

## 🤝 Contribution

Ce projet est développé dans le cadre d'un cursus académique. Les contributions externes ne sont pas acceptées pour des raisons d'intégrité académique.

## 📚 Ressources

### Gomoku et Algorithmes
- [Règles du Gomoku](https://en.wikipedia.org/wiki/Gomoku)
- [Algorithme Min-Max](https://en.wikipedia.org/wiki/Minimax)
- [Ninuki-renju (Captures)](https://en.wikipedia.org/wiki/Ninuki-renju)
- [Alpha-Beta Pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning)

### C++ et Développement
- [Modern C++ Guide](https://github.com/AnthonyCalandra/modern-cpp-features)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [SFML Documentation](https://www.sfml-dev.org/documentation/)

---

**🎯 Statut actuel :** Analyse du projet et planification de l'architecture  
**⏳ Prochaine étape :** Implémentation de la structure de base et du plateau de jeu

*Projet réalisé dans le cadre du cursus 42* 