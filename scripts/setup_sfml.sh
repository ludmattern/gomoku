#!/bin/bash
# SFML installation and setup script

set -e  # Exit on error

UNAME_S=$(uname -s)

echo "🔍 Checking SFML installation..."

if [ "$UNAME_S" = "Darwin" ]; then
    echo "🍎 macOS system detected"
    
    if brew list sfml >/dev/null 2>&1; then
        echo "✅ SFML already installed via Homebrew"
        echo "📚 Available libraries:"
        ls /opt/homebrew/lib/libsfml* 2>/dev/null | head -3
    else
        echo "📦 Installing SFML via Homebrew..."
        brew install sfml
        echo "✅ SFML installed successfully!"
    fi
    
    SFML_DIR="/opt/homebrew"
    
else
    # Linux
    echo "🐧 Linux system detected"
    
    # Try to detect SFML_DIR
    if [ -n "$SFML_DIR" ]; then
        echo "Using provided SFML_DIR: $SFML_DIR"
    elif [ -d "$HOME/local" ]; then
        SFML_DIR="$HOME/local"
    else
        SFML_DIR="/usr"
    fi
    
    if [ -f "$SFML_DIR/lib/libsfml-graphics.so" ] || [ -f "$SFML_DIR/lib64/libsfml-graphics.so" ]; then
        echo "✅ SFML found in $SFML_DIR"
        echo "📚 Available libraries:"
        ls "$SFML_DIR"/lib/libsfml* "$SFML_DIR"/lib64/libsfml* 2>/dev/null | head -3
    else
        echo "❌ SFML not found in $SFML_DIR"
        echo "Attempting local installation into $HOME/local..."
        mkdir -p "$HOME/local"

        if [ -f "./scripts/install_sfml.sh" ]; then
            echo "📦 Running local installer: ./scripts/install_sfml.sh"
            # run installer with bash to avoid permission issues
            bash ./scripts/install_sfml.sh || { echo "❌ Local installer failed"; }
            SFML_DIR="$HOME/local"
            if [ -f "$SFML_DIR/lib/libsfml-graphics.so" ] || [ -f "$SFML_DIR/lib64/libsfml-graphics.so" ]; then
                echo "✅ SFML installed in $SFML_DIR"
                ls "$SFML_DIR"/lib/libsfml* "$SFML_DIR"/lib64/libsfml* 2>/dev/null | head -3
            else
                echo "❌ SFML still not found after local installation"
                echo ""
                echo "💡 Installation suggestions:"
                echo "  Ubuntu/Debian: sudo apt install libsfml-dev"
                echo "  Fedora/RHEL:   sudo dnf install SFML-devel"
                echo "  Arch:          sudo pacman -S sfml"
                echo ""
                echo "Or set SFML_DIR to point to your SFML installation"
                exit 1
            fi
        else
            echo "Installer script not found: ./scripts/install_sfml.sh"
            echo "💡 Installation suggestions:"
            echo "  Ubuntu/Debian: sudo apt install libsfml-dev"
            echo "  Fedora/RHEL:   sudo dnf install SFML-devel"
            echo "  Arch:          sudo pacman -S sfml"
            echo ""
            echo "Or set SFML_DIR to point to your SFML installation"
            exit 1
        fi
    fi
fi

echo ""
echo "🎯 SFML setup complete!"
echo "Directory: $SFML_DIR"
