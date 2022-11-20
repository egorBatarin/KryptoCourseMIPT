#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np # импорт бибилиотеки numpy
import matplotlib.pyplot as plt # импорт модуля matplotlib.pyplot
import pandas as pd # csv
#import seaborn
from scipy import interpolate
from scipy.optimize import curve_fit

data = pd.read_csv("avalanche_stat.log", sep='\t', skipfooter=4, engine='python')
global_stat = pd.read_csv("avalanche_stat.log", sep='\t', skiprows=len(data)+1, skipfooter=2, engine='python')
params = pd.read_csv("avalanche_stat.log", sep='\t', skiprows=len(data)+3)
mod_bits = data[' ~bits']

# intializing the list
arr = list(mod_bits)
# initializing dict to store frequency of each element
elements_count = {}
# iterating over the elements for frequency
for element in arr:
   # checking whether it is in the dict or not
   if element in elements_count:
      # incerementing the count by 1
      elements_count[element] += 1
   else:
      # setting the count to 1
      elements_count[element] = 1
# printing the elements frequencies
# elements_count = sorted(elements_count)
x = []
y = []
for key, value in sorted(elements_count.items()):
   x = x + [key]
   y = y + [value]
   #print(f"{key}: {value}")


xmax = params['hash_len(bits)'][0]
ymax = global_stat['total(#)'][0]
x = np.array(x) *100. / xmax
y = np.array(y) * 100. / ymax

plt.figure(figsize=[20,8])
plt.bar(x, y, width = 0.2, color='red',alpha=1)
plt.xlim(0, 100)
plt.grid(alpha=0.75)
plt.xlabel('modified bits, %',fontsize=15)
plt.ylabel('iterations, %',fontsize=15)
plt.xticks(np.arange(0, 105, 5))
plt.minorticks_on()
plt.title(f"Avalanche stat\n(hash size: {params['hash_len(bits)'][0]}, rounds: {params['rounds'][0]}, iterations: {global_stat['total(#)'][0]})",fontsize=15)
plt.text(80, 2, f"Average: {global_stat['average(%)'][0]}%", fontsize=15)
filename = f"plot_{params['hash_len(bits)'][0]}_{params['rounds'][0]}_{global_stat['total(#)'][0]}.png"
plt.savefig(filename)
#plt.show()
print("plot has been stored in ./", filename)