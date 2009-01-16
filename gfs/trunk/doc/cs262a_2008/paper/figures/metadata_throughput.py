#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt

x = np.arange(3)
width = 0.35
hdfs = (292.97, 304.49, 305.47)
bfs = (614.57, 706.02, 847.44)

plt.subplot(111)
hdfs_rects = plt.bar(x, hdfs, width, color='b')
bfs_rects = plt.bar(x + width, bfs, width, color='g', hatch='/')

plt.xticks(x + width, ('100 ls ops', '100 touch ops', '100 0k copy ops'))
plt.ylabel('Time (sec)')
plt.title('Metadata Performance')
plt.legend((hdfs_rects[0], bfs_rects[0]), ('HDFS', 'BoomFS'), loc='upper right')
plt.ylim([0, 1200])
plt.savefig('metadata_throughput.eps')
