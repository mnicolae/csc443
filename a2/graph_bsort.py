import matplotlib
from pylab import *
  

MB = 1024 * 1024
GB = 1024 * MB

figure(1)
title('bsort')
x = [36*MB, 72*MB, 143*MB, 286*MB, 571*MB, 1.2*GB]
y = [262, 254, 263, 535, 516, 508, 1021, 1049, 1030, 2074, 2042, 2057, 4067, 4140, 4151, 8175, 8206, 8231]
z = [260, 520, 1033, 2058, 4119, 8204] 
loglog(x, z)
xlabel('file size')
ylabel('milliseconds')
savefig('graph_bsort.png')
