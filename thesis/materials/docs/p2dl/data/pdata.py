import os
from math import sqrt

if __name__ == "__main__":
  start_time = 0
  time_step  = 60.
  step       = 0 

  file = open("hops21_dynamic.dat", "r")
  for line in file:
    result = line.split(" ") 
    result[0] = float(result[0])
    result[1] = float(result[1])
    if not start_time:
      hop_list = []
      start_time = result[0]
      sum   = 0
      count = 0
      step += 1
    hop_list.append(result[1])
    sum += result[1]
    count += 1.0

    if result[0] - start_time >= time_step:
      mean = sum / count
      var = 0.
      for hop in hop_list:
        var += (hop - mean)**2.0
      var = var/count
      stddev = sqrt(var)
      print "%d %f %f" % ((step*time_step), mean, stddev)
      start_time = 0
