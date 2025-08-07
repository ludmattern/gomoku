#!/bin/bash

# =============================================================================
# Script d'installation de SFML avec toutes les dépendances audio
# Usage: ./scripts/install_sfml.sh
# =============================================================================

set -e  # Arrêter le script en cas d'erreur

# Configuration
SFML_VERSION="2.6.1"
OPENAL_VERSION="1.23.1"
OGG_VERSION="1.3.5"
VORBIS_VERSION="1.3.7"
FLAC_VERSION="1.4.3"
INSTALL_DIR="$HOME/local"
BUILD_JOBS=4

# Couleurs pour les messages
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Fonctions utilitaires
log_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

log_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

log_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

log_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Vérification des prérequis
check_prerequisites() {
    log_info "Vérification des prérequis..."
    
    # Vérifier cmake
    if ! command -v cmake &> /dev/null; then
        log_error "cmake n'est pas installé. Veuillez l'installer."
        exit 1
    fi
    
    # Vérifier make
    if ! command -v make &> /dev/null; then
        log_error "make n'est pas installé. Veuillez l'installer."
        exit 1
    fi
    
    # Vérifier wget
    if ! command -v wget &> /dev/null; then
        log_error "wget n'est pas installé. Veuillez l'installer."
        exit 1
    fi
    
    log_success "Tous les prérequis sont satisfaits"
}

# Fonction pour télécharger et compiler une bibliothèque
compile_library() {
    local name=$1
    local url=$2
    local filename=$(basename $url)
    local dirname=$3
    
    log_info "Installation de $name..."
    
    # Vérifier si déjà installé
    if [ -f "$INSTALL_DIR/lib/lib$4" ]; then
        log_warning "$name est déjà installé, passage à la suivante..."
        return 0
    fi
    
    # Télécharger
    log_info "Téléchargement de $name..."
    wget -q "$url" -O "$filename"
    
    # Extraire
    log_info "Extraction de $name..."
    tar -xf "$filename"
    cd "$dirname"
    
    # Compiler avec -fPIC
    log_info "Compilation de $name..."
    mkdir -p build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    make -j"$BUILD_JOBS"
    make install
    
    # Nettoyer
    cd "$INSTALL_DIR"
    rm -f "$filename"
    log_success "$name installé avec succès"
}

# Installation principale
main() {
    echo "🚀 Installation de SFML $SFML_VERSION avec dépendances audio..."
    echo "📁 Installation dans: $INSTALL_DIR"
    echo ""
    
    # Vérifier les prérequis
    check_prerequisites
    
    # Créer le dossier d'installation
    mkdir -p "$INSTALL_DIR"
    cd "$INSTALL_DIR"
    
    # Installation des dépendances audio
    log_info "Installation des dépendances audio..."
    
    # OpenAL
    compile_library "OpenAL" \
        "https://github.com/kcat/openal-soft/archive/refs/tags/$OPENAL_VERSION.tar.gz" \
        "openal-soft-$OPENAL_VERSION" \
        "openal.so"
    
    # libogg
    compile_library "libogg" \
        "https://github.com/xiph/ogg/releases/download/v$OGG_VERSION/libogg-$OGG_VERSION.tar.gz" \
        "libogg-$OGG_VERSION" \
        "ogg.a"
    
    # libvorbis
    compile_library "libvorbis" \
        "https://github.com/xiph/vorbis/releases/download/v$VORBIS_VERSION/libvorbis-$VORBIS_VERSION.tar.gz" \
        "libvorbis-$VORBIS_VERSION" \
        "vorbis.a"
    
    # FLAC
    compile_library "FLAC" \
        "https://github.com/xiph/flac/releases/download/$FLAC_VERSION/flac-$FLAC_VERSION.tar.xz" \
        "flac-$FLAC_VERSION" \
        "FLAC.a"
    
    # Installation de SFML
    log_info "Installation de SFML $SFML_VERSION..."
    cd "$INSTALL_DIR"
    
    # Vérifier si SFML est déjà installé
    if [ -f "$INSTALL_DIR/lib/libsfml-graphics.so" ]; then
        log_warning "SFML est déjà installé, passage à la vérification..."
    else
        # Télécharger SFML
        log_info "Téléchargement de SFML..."
        wget -q "https://github.com/SFML/SFML/archive/refs/tags/$SFML_VERSION.tar.gz"
        tar -xf "$SFML_VERSION.tar.gz"
        cd "SFML-$SFML_VERSION"
        
        # Compiler SFML avec toutes les dépendances
        log_info "Compilation de SFML..."
        mkdir -p build
        cd build
        cmake .. -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                 -DCMAKE_PREFIX_PATH="$INSTALL_DIR" \
                 -DCMAKE_LIBRARY_PATH="$INSTALL_DIR/lib" \
                 -DCMAKE_INCLUDE_PATH="$INSTALL_DIR/include"
        
        make -j"$BUILD_JOBS"
        make install
        
        # Nettoyer
        cd "$INSTALL_DIR"
        rm -f "$SFML_VERSION.tar.gz"
        log_success "SFML installé avec succès !"
    fi
    
    # Vérification finale
    echo ""
    log_info "Vérification de l'installation..."
    echo "📁 Installation dans: $INSTALL_DIR"
    echo "📚 Bibliothèques installées:"
    ls "$INSTALL_DIR"/lib/libsfml* 2>/dev/null | head -3 || echo "  - SFML"
    ls "$INSTALL_DIR"/lib/libopenal* 2>/dev/null | head -1 || echo "  - OpenAL"
    ls "$INSTALL_DIR"/lib/libvorbis* 2>/dev/null | head -1 || echo "  - Vorbis"
    ls "$INSTALL_DIR"/lib/libFLAC* 2>/dev/null | head -1 || echo "  - FLAC"
    echo ""
    log_success "Installation terminée avec succès !"
}

# Exécution du script
main "$@" 