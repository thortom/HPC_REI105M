from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

nodes = 10
x = np.arange(0, nodes, 1)
t = np.arange(0, 4, 0.001)

k = 2 * np.pi / 1
w = 2 * np.pi / 2
a = 10

y = []
for i in x:
    y.append(a * np.sin(2 * np.pi * (k * i - w * t)))

for i in range(nodes):
    plt.plot(t, y[i])
    plt.show()