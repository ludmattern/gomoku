# Gomoku Project Architecture

This document explains the project structure and how to use the different components.

## 📁 Project Structure

```
📂 include/                 # Public headers (API)
├── 📂 gomoku/             # Core game logic (no GUI dependencies)
│   ├── Types.hpp          # Basic types and enums
│   ├── BoardView.hpp      # Abstract board interface
│   ├── Engine.hpp         # AI engine interface
│   ├── GameSession.hpp    # Game session management
│   └── Notation.hpp       # Game notation system
├── 📂 gui/                # Graphics and rendering (SFML-dependent)
│   ├── GameWindow.hpp     # Main game window
│   ├── GameBoardRenderer.hpp # Board rendering
│   └── ResourceManager.hpp  # Asset management
├── 📂 scene/              # Scene management system
│   ├── AScene.hpp         # Abstract scene base
│   ├── Context.hpp        # Scene context
│   ├── SceneManager.hpp   # Scene transitions
│   ├── MainMenu.hpp       # Main menu scene
│   ├── GameScene.hpp      # Game play scene
│   └── GameSelect.hpp     # Game selection scene
└── 📂 ui/                 # UI components
    └── Button.hpp         # Button widget

📂 src/                     # Implementation files
├── 📂 core/               # Core game logic implementation
├── 📂 ai/                 # AI algorithm implementation
├── 📂 app/                # Application layer
├── 📂 gui/                # GUI implementation
├── 📂 scene/              # Scene implementations
├── 📂 ui/                 # UI implementations
└── 📂 cli/                # Command-line interface

📂 lib/                     # Generated libraries
└── libgomoku_core.a       # Core game logic library (no SFML)
```

## 🎯 Usage Patterns

### For Core Game Logic Only (No GUI)
```cpp
#include "gomoku/Engine.hpp"
#include "gomoku/GameSession.hpp"

// Perfect for unit tests, CLI, or integrating into other UIs
```

### For Complete GUI Application
```cpp
// Specific components:
#include "gui/GameWindow.hpp"
#include "scene/MainMenu.hpp"
```

### For Custom Integration
```cpp
// Core logic only
#include "gomoku/BoardView.hpp"  
#include "gomoku/Engine.hpp"

// Add your own rendering/UI
```

## Architecture Principles

1. **Core Separation**: `gomoku/` has no GUI dependencies
2. **Modular Design**: Each folder has a clear responsibility  
3. **Interface-Based**: Public APIs use abstract interfaces
4. **Library Structure**: Core logic compiles to `libgomoku_core.a`

## Getting Started

1. **Build everything**: `make`
2. **Core library only**: `make lib`
3. **Run tests**: `make test`
4. **Clean build**: `make re`

## Development

- **Adding core logic**: Modify `src/core/` or `src/ai/`
- **Adding GUI features**: Modify `src/gui/` or `src/scene/`
- **Public API changes**: Update headers in `include/`
- **New dependencies**: Update `Makefile` CORE_SRC or GUI_SRC
