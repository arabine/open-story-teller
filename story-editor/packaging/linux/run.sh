#!/bin/bash

EXECDIR=$(pwd)
SCRIPTDIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
pushd "$SCRIPTDIR/../../release"
BUILDDIR=$(pwd)

DESKTOP_ENTRY=$(cat <<-END
[Desktop Entry]
Type=Application
Name=Open Story Teller Editor
Comment=A tool to create stories with graphical nodes
Exec=story-editor
Icon=story-editor
Categories=Graphics;2DGraphics;
END

)

# echo "exec dir: $EXECDIR"
# echo "script dir: $SCRIPTDIR"
# echo "build dir: $BUILDDIR"

# start with clean directory
rm -rf *

# ====================================================================
# Build application
# ====================================================================
echo "Building Story Editor..."
mkdir -p build
cd build
cmake ../.. .
make
cd ..

# ====================================================================
# Create AppImage folder structure
# ====================================================================

mkdir -p usr/bin
mkdir -p usr/lib
mkdir -p usr/share/applications
mkdir -p usr/share/icons/hicolor/512x512 

# ====================================================================
# Fill with files
# ====================================================================
mv build/story-editor usr/bin
rm -rf build
echo $DESKTOP_ENTRY > usr/share/story-editor.desktop

~/Applications/linuxdeployqt-continuous-x86_64.AppImage usr/share/applications/story-editor.desktop -appimage


popd
