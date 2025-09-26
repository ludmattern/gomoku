#!/bin/bash

# Script d'installation de l'icône et du lanceur Gomoku
# Ce script configure Gomoku pour être accessible depuis le bureau et le menu applications

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DESKTOP_DIR="$SCRIPT_DIR"
BIN_PATH="$PROJECT_DIR/bin/Gomoku"
ICON_DEST="$DESKTOP_DIR/gomicon.png"
DESKTOP_FILE="$DESKTOP_DIR/gomoku.desktop"

echo "[INSTALL-DESKTOP] Setting up desktop integration"

# Vérifier que l'exécutable existe
if [ ! -f "$BIN_PATH" ]; then
    echo "[INSTALL-DESKTOP] Error: Executable not found at $BIN_PATH"
    echo "[INSTALL-DESKTOP] Please compile first with 'make'"
    exit 1
fi

# Vérifier que l'icône existe
if [ ! -f "$ICON_DEST" ]; then
    echo "[INSTALL-DESKTOP] Error: Icon file not found at $ICON_DEST"
    echo "[INSTALL-DESKTOP] Make sure gomicon.png is in desktop/ directory"
    exit 1
fi

echo "[INSTALL-DESKTOP] Icon found: $ICON_DEST"

# Mettre à jour le fichier .desktop avec les chemins absolus
echo "[INSTALL-DESKTOP] Configuring launcher..."
sed -i "s|Exec=.*|Exec=$BIN_PATH|g" "$DESKTOP_FILE"
sed -i "s|Icon=.*|Icon=$ICON_DEST|g" "$DESKTOP_FILE"
sed -i "s|Path=.*|Path=$PROJECT_DIR|g" "$DESKTOP_FILE"

# Ajouter Path si elle n'existe pas
if ! grep -q "^Path=" "$DESKTOP_FILE"; then
    sed -i "/^Icon=.*$/a Path=$PROJECT_DIR" "$DESKTOP_FILE"
fi

# Rendre le fichier .desktop exécutable
chmod +x "$DESKTOP_FILE"

echo "[INSTALL-DESKTOP] Desktop file configured: $DESKTOP_FILE"

# Copier le fichier .desktop vers les emplacements appropriés
USER_DESKTOP_DIR="$HOME/Desktop"
USER_APPLICATIONS_DIR="$HOME/.local/share/applications"

# Créer le répertoire applications s'il n'existe pas
mkdir -p "$USER_APPLICATIONS_DIR"

# Copier vers le répertoire des applications
cp "$DESKTOP_FILE" "$USER_APPLICATIONS_DIR/"
echo "[INSTALL-DESKTOP] Launcher installed to applications menu"

# Copier vers le bureau si le répertoire existe
if [ -d "$USER_DESKTOP_DIR" ]; then
    cp "$DESKTOP_FILE" "$USER_DESKTOP_DIR/"
    chmod +x "$USER_DESKTOP_DIR/gomoku.desktop"
    echo "[INSTALL-DESKTOP] Launcher added to desktop"
fi

# Mettre à jour la base de données des applications
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$USER_APPLICATIONS_DIR"
    echo "[INSTALL-DESKTOP] Application database updated"
fi

echo "[INSTALL-DESKTOP] Installation completed successfully"