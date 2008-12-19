#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt

xvalues = [1,4,7,16]

plt.xlabel('Concurrency Factor')
plt.ylabel('Time (sec)')
plt.title('Read Performance (100MB file size)')
plt.errorbar(xvalues, [7.95,8.14,8.16,11.53], [1.07, 1.05, 1.05, 1.09], fmt='o-', label='HDFS')
plt.errorbar(xvalues, [8.97,9.31,8.78,9.3], [0.35, 1.29, 1.33, 3.37], fmt='^-', label='BoomFS')
plt.xlim([0,17])
plt.ylim([5,14])
plt.legend(loc='upper right')
plt.savefig('big_read_throughput.eps')
