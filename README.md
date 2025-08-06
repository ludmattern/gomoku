# 🧠 Gomoku IA - Intelligence Artificielle de Jeu de Stratégie

**Version:** 3.2  
**Statut:** 🚧 **IN PROGRESS** 🚧

> *"Yeah, well, your brain has to fry sometime"*

## 📋 Description

Ce projet consiste à développer une intelligence artificielle capable de battre des joueurs humains au **Gomoku**, un jeu de stratégie traditionnel joué sur un plateau de Go. L'IA utilise l'algorithme **Min-Max** avec une heuristique optimisée pour prendre des décisions rapides et efficaces.

## 🎯 Objectifs

- Créer une IA imbattable au Gomoku
- Implémenter l'algorithme Min-Max avec une heuristique performante
- Développer une interface graphique intuitive et agréable
- Respecter les contraintes de performance (≤ 0.5s par coup en moyenne)

## 🎮 Règles du Jeu

### Règles de base
- **Plateau :** 19x19 (Goban standard)
- **Victoire :** Aligner 5 pierres ou plus
- **Joueurs :** 2 joueurs alternent pour placer leurs pierres

### Règles spéciales implémentées

#### 🎯 **Captures (Ninuki-renju/Pente)**
- Capturer une paire de pierres adverses en les flanquant
- **Condition de victoire :** Capturer 10 pierres adverses
- Les intersections libérées peuvent être rejouées

#### 🏁 **Capture en fin de partie**
- Un alignement de 5 ne gagne que si l'adversaire ne peut pas le briser par capture
- Si un joueur a déjà perdu 4 paires et que l'adversaire peut en capturer une 5ème, victoire par capture

#### 🚫 **Interdiction des double-trois**
- Interdiction de jouer un coup qui crée deux alignements de trois libres simultanément
- Exception : Les double-trois créés par capture sont autorisés

## 🛠️ Architecture Technique

### Algorithme Principal
- **Min-Max** avec élagage alpha-bêta
- **Heuristique personnalisée** pour l'évaluation des positions
- **Arbre de recherche** optimisé pour la performance

### Composants principaux
```
📦 Gomoku/
├── 🧠 ia/                 # Intelligence artificielle
│   ├── minimax.c         # Algorithme Min-Max
│   ├── heuristic.c       # Fonction d'évaluation
│   └── search.c          # Arbre de recherche
├── 🎮 game/              # Logique de jeu
│   ├── board.c           # Gestion du plateau
│   ├── rules.c           # Règles du Gomoku
│   └── capture.c         # Système de captures
├── 🖼️  gui/               # Interface graphique
│   ├── display.c         # Affichage
│   ├── events.c          # Gestion des événements
│   └── timer.c           # Timer de performance
└── 📁 utils/             # Utilitaires
    ├── debug.c           # Mode debug
    └── utils.c           # Fonctions utilitaires
```

## ⚡ Fonctionnalités

### Partie Obligatoire
- [x] **Exécutable :** `Gomoku`
- [ ] **IA vs Humain :** Jeu contre l'intelligence artificielle
- [ ] **Humain vs Humain :** Mode deux joueurs avec suggestions de l'IA
- [ ] **Interface graphique :** Interface utilisable et agréable
- [ ] **Timer de performance :** Affichage du temps de réflexion de l'IA
- [ ] **Mode debug :** Visualisation du processus de décision de l'IA
- [ ] **Makefile :** Compilation avec règles standard

### Partie Bonus (à implémenter après validation de la partie obligatoire)
- [ ] **Choix des règles :** Sélection des règles de jeu au démarrage
- [ ] **Conditions de départ :** Standard, Pro, Swap, Swap2...
- [ ] **Fonctionnalités supplémentaires :** À définir

## 🔧 Compilation et Installation

```bash
# Compilation
make

# Compilation complète
make all

# Nettoyage des fichiers objets
make clean

# Nettoyage complet
make fclean

# Recompilation
make re
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

- [Règles du Gomoku](https://en.wikipedia.org/wiki/Gomoku)
- [Algorithme Min-Max](https://en.wikipedia.org/wiki/Minimax)
- [Ninuki-renju (Captures)](https://en.wikipedia.org/wiki/Ninuki-renju)

---

**🎯 Statut actuel :** Analyse du projet et planification de l'architecture  
**⏳ Prochaine étape :** Implémentation de la structure de base et du plateau de jeu

*Projet réalisé dans le cadre du cursus 42*
