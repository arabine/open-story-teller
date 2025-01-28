#!/bin/bash

# Variables
APP_NAME="OpenStoryEditor"
APP_ID="eu.d8s.OpenStoryEditor"
VERSION="1.0.0"
ICON_NAME="stoty-editor-logo-256x256.png"
BUILD_DIR="build" # Répertoire où se trouve l'exécutable généré
FLATPAK_DIR="flatpak_build"
FLATPAK_MANIFEST="${FLATPAK_DIR}/${APP_ID}.yaml"

# Vérifier les prérequis
if ! command -v flatpak-builder &> /dev/null; then
    echo "Erreur : flatpak-builder n'est pas installé. Installez-le avant de continuer."
    exit 1
fi

if [ ! -f "${BUILD_DIR}/${APP_NAME}" ]; then
    echo "Erreur : L'exécutable '${BUILD_DIR}/${APP_NAME}' est introuvable."
    exit 1
fi

if [ ! -f "${ICON_NAME}" ]; then
    echo "Erreur : L'icône '${ICON_NAME}' est introuvable. Placez une icône PNG de 256x256 pixels à la racine."
    exit 1
fi

# Préparation du répertoire Flatpak
echo "Création du répertoire Flatpak..."
rm -rf "${FLATPAK_DIR}"
mkdir -p "${FLATPAK_DIR}"

# Génération du fichier manifeste YAML
echo "Création du fichier manifeste Flatpak (${FLATPAK_MANIFEST})..."
cat > "${FLATPAK_MANIFEST}" <<EOL
app-id: ${APP_ID}
runtime: org.freedesktop.Platform
runtime-version: "23.08"
sdk: org.freedesktop.Sdk
command: ${APP_NAME}
finish-args:
  - --share=network
  - --share=ipc
  - --device=dri
  - --socket=x11
  - --socket=wayland
  - --filesystem=home

modules:
  - name: ${APP_NAME}
    buildsystem: simple
    build-commands:
      - install -Dm755 story-editor /app/bin/story-editor
      - install -Dm644 story-editor.desktop /app/share/applications/story-editor.desktop
      - install -Dm644 ${ICON_NAME} /app/share/icons/hicolor/256x256/apps/${APP_ID}.png
    sources:
      - type: dir
        path: ../${BUILD_DIR}
EOL

# Création du fichier .desktop
echo "Création du fichier .desktop..."
cat > "${BUILD_DIR}/${APP_NAME}.desktop" <<EOL
[Desktop Entry]
Type=Application
Name=${APP_NAME}
Exec=${APP_NAME}
Icon=${APP_ID}
Terminal=false
Categories=Utility;
EOL

# Construction du Flatpak
echo "Construction du Flatpak..."
flatpak-builder --force-clean \
    "${FLATPAK_DIR}/build-dir" \
    "${FLATPAK_MANIFEST}"

if [ $? -ne 0 ]; then
    echo "Erreur : Échec de la construction du Flatpak."
    exit 1
fi

# Exportation du Flatpak en un fichier .flatpak
echo "Exportation du Flatpak..."
flatpak build-bundle "${FLATPAK_DIR}/build-dir" \
    "${FLATPAK_DIR}/${APP_NAME}-${VERSION}.flatpak" \
    "${APP_ID}" "${VERSION}"

if [ $? -eq 0 ]; then
    echo "Flatpak généré avec succès : ${FLATPAK_DIR}/${APP_NAME}-${VERSION}.flatpak"
else
    echo "Erreur : Échec de l'exportation du Flatpak."
    exit 1
fi
