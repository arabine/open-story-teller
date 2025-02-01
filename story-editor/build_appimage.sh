#!/bin/bash

# Variables
APP_NAME="OpenStoryEditor"
APP_DIR="AppDir"
APP_EXE="story-editor"
BUILD_DIR="build-linux"
APPIMAGE_TOOL="appimagetool-x86_64.AppImage"
APPIMAGE_URL="https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"

# Vérification des dépendances
if ! command -v cmake &> /dev/null; then
    echo "Erreur : cmake n'est pas installé. Installez-le avant de continuer."
    exit 1
fi

if ! command -v wget &> /dev/null; then
    echo "Erreur : wget n'est pas installé. Installez-le avant de continuer."
    exit 1
fi

# Étape 1 : Préparer l'AppDir
echo "Préparation de l'AppDir..."
rm -rf "${APP_DIR}"
mkdir -p "${APP_DIR}/usr/bin"
mkdir -p "${APP_DIR}/usr/share/applications"
mkdir -p "${APP_DIR}/usr/share/icons/hicolor/256x256/apps"

# Copier les fichiers nécessaires
cp "${BUILD_DIR}/${APP_EXE}" "${APP_DIR}/usr/bin/"
cat > "${APP_DIR}/${APP_NAME}.desktop" <<EOL
[Desktop Entry]
Type=Application
Name=${APP_NAME}
Exec=${APP_EXE}
Icon=${APP_NAME}
Terminal=false
Categories=Utility;
EOL

cat > "${APP_DIR}/AppRun" <<EOL
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="$HERE/usr/bin:$PATH"
export LD_LIBRARY_PATH="$HERE/usr/lib:$LD_LIBRARY_PATH"
exec "$HERE/usr/bin/${APP_EXE}" "$@"
EOL

chmod +x ${APP_DIR}/AppRun

# Ajouter une icône (optionnel, remplacer par une vraie icône si disponible)
cp delivery/story-editor-logo-256x256.png "${APP_DIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
cp delivery/story-editor-logo-256x256.png "${APP_DIR}/${APP_NAME}.png"
cp "${APP_DIR}/${APP_NAME}.desktop" "${APP_DIR}/usr/share/applications/${APP_NAME}.desktop"

# Étape 2 : Télécharger l'outil AppImage
if [ ! -f "${APPIMAGE_TOOL}" ]; then
    echo "Téléchargement de l'outil AppImageTool..."
    wget "${APPIMAGE_URL}" -O "${APPIMAGE_TOOL}"
    chmod +x "${APPIMAGE_TOOL}"
fi

# Étape 3 : Générer l'AppImage
echo "Génération de l'AppImage..."
ARCH=x86_64 ./"${APPIMAGE_TOOL}" "${APP_DIR}" || { echo "Erreur : échec de la création de l'AppImage."; exit 1; }

echo "AppImage générée avec succès : ${APP_NAME}-x86_64.AppImage"
