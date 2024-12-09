import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
from math import sqrt

# the first element is the size, the rest is the adjacency list
f = open("adj_list.csv", "r")

size = int(f.readline())
mat_size = int(sqrt(size))
print(size)
print(mat_size)
list_from_file = f.read().strip().split(',')
mat = np.array(list_from_file, dtype=int).reshape(mat_size, mat_size)

G = nx.from_numpy_matrix(mat, create_using=nx.DiGraph)

pos = nx.spring_layout(G)  # positions for all nodes

# Draw the graph
nx.draw(G, pos, with_labels=True, node_size=700, node_color="skyblue", font_size=8, font_weight="bold", font_color="black")

# Draw edge labels
edge_labels = nx.get_edge_attributes(G, 'weight')
nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels)

plt.show()