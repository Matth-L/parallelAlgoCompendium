#!/bin/bash

# Objectif : Créer un fichier, avec n suivie de n élement entiers aléatoires, 
# chaque élement est séparé d'un espace
# input : n
# output : array.txt

#################### Vérification  #####################

if [ $# -ne 1 ]; then
    echo "Usage: ./$0 n"
    exit 1
fi
if ! [[ $1 =~ ^[0-9]+$ ]]; then
    echo "Error: $1 n'est pas un entier"
    exit 2
fi

################ Suppression de l'ancien fichier ###############

if [ -f array.txt ]; then
    rm array.txt
fi

################ Création du fichier ###############

touch array.txt ; echo -n $1 > array.txt

for i in $(seq 1 $1); do
    echo -n " $((RANDOM % 100))" >> array.txt
done

echo "" >> array.txt
