#!/bin/bash
for i in {0..14}; do
  scp -r sexy_number mpiuser18@dim-openmpi$i.uqac.ca:.
done
