#!/bin/bash

# Objectif : Créer un fichier, avec n suivie de n élement entiers aléatoires, 
# chaque élement est séparé d'un espace
# input : n (la taille du tableau), output_dir (le répertoire de sortie)
# output : unsorted_array.txt  (un tableau non trié de taille N)

#################### Vérification  #####################

if [ $# -ne 2 ]; then
    echo "Usage: ./$0 n output_dir"
    exit 1
fi
if ! [[ $1 =~ ^[0-9]+$ ]]; then
    echo "Error: $1 n'est pas un entier"
    exit 2
fi

################ Suppression de l'ancien fichier ###############

output_dir=$2
mkdir -p $output_dir
filename="${output_dir}/unsorted_array_${1}.txt"

if [ -f $filename ]; then
    rm $filename 
fi

################ Création du fichier ###############
mkdir -p $output_dir
touch $filename ; echo -n $1 > $filename

for i in $(seq 1 $1); do
    echo -n " $((RANDOM % 10000))" >> $filename
done