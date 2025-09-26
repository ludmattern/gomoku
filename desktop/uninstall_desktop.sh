#!/bin/bash

# Script de désinstallation du lanceur Gomoku

set -e

USER_DESKTOP_DIR="$HOME/Desktop"
USER_APPLICATIONS_DIR="$HOME/.local/share/applications"
DESKTOP_FILE="gomoku.desktop"

echo "[UNINSTALL-DESKTOP] Removing desktop integration"

# Supprimer du menu des applications
if [ -f "$USER_APPLICATIONS_DIR/$DESKTOP_FILE" ]; then
    rm "$USER_APPLICATIONS_DIR/$DESKTOP_FILE"
    echo "[UNINSTALL-DESKTOP] Launcher removed from applications menu"
else
    echo "[UNINSTALL-DESKTOP] No launcher found in applications menu"
fi

# Supprimer du bureau
if [ -f "$USER_DESKTOP_DIR/$DESKTOP_FILE" ]; then
    rm "$USER_DESKTOP_DIR/$DESKTOP_FILE"
    echo "[UNINSTALL-DESKTOP] Launcher removed from desktop"
else
    echo "[UNINSTALL-DESKTOP] No launcher found on desktop"
fi

# Mettre à jour la base de données des applications
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$USER_APPLICATIONS_DIR"
    echo "[UNINSTALL-DESKTOP] Application database updated"
fi

echo "[UNINSTALL-DESKTOP] Uninstallation completed"