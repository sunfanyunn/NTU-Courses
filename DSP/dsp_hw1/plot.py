# coding: utf-8
import matplotlib.pyplot as plt
with open('process', 'r') as f:
        a= f.readlines()

plt.plot([float(x.strip()) for x in a])
plt.xlabel('Iterations')
plt.ylabel('Accuracy on testing_data1.txt')
plt.plot([float(x.strip()) for x in a])
plt.savefig('result')
plt.show()
