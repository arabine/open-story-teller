#!/bin/bash

# Nom de l'exécutable à analyser (à remplacer par le votre)
executable="build-linux/story-editor"

# Commande ldd et stockage de la sortie dans une variable
ldd_output=$(ldd "$executable"  | awk '/ => / { print $1 }')

echo $ldd_output



# repertoire_courant=$(pwd)

# # Récupère les noms des bibliothèques en filtrant sur le répertoire courant
# bibliotheques_courant=$(ldd "$executable" | awk -v dir="$repertoire_courant" '/ => / && $3 ~ "^"dir"/" { print $3 }')

# # Affiche les résultats
# echo "Bibliothèques dans le répertoire courant :"
# echo "$bibliotheques_courant"
