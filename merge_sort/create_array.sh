#!/bin/bash

# Objectif : Créer un fichier, avec n suivie de n élement entiers aléatoires, 
# chaque élement est séparé d'un espace
# input : n (la taille du tableau), output_dir (le répertoire de sortie)
# output : unsorted_array.txt  (un tableau non trié de taille N)

#################### Vérification  #####################

if [ $# -ne 1 ]; then
    echo "Usage: ./$0 size_of_array"
    exit 1
fi
if ! [[ $1 =~ ^[0-9]+$ ]]; then
    echo "Error: $1 n'est pas un entier"
    exit 2
fi

################ Suppression de l'ancien fichier ###############

filename="unsorted_array_${1}.txt"

if [ -f $filename ]; then
    rm $filename 
fi

################ Création du fichier ###############
sequence=""

for ((i = 1; i <= $1; i++)); do
    sequence+=" $((RANDOM % 100))"
done

echo -n "$1$sequence" > "$filename"