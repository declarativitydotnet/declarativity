#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt

xvalues = [1,4,7,16]

plt.xlabel('Concurrency Factor')
plt.ylabel('Time (sec)')
plt.title('Write Performance (100MB file size)')
plt.errorbar(xvalues, [14.37,26.41,28.02,28.04], [2.1,2.74,2.58,2.58], fmt='o-', label='HDFS')
plt.errorbar(xvalues, [9.81,12.68,11.27,15.11], [0.47,3.82,3.69,17.31], fmt='^-', label='BoomFS')
plt.xlim([0,17])
plt.ylim([0,40])
plt.legend(loc='upper right')
plt.savefig('big_write_throughput.eps')
