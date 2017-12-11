import numpy as np
import matplotlib.pyplot as plt

data = []
for line in open("data_l3.csv", "r+").readlines():
   line = line.strip()
   print line
   if "stat" in line:
      data += [ tuple(line.split(",")[1:]) ]

def get_trend(utilization):
   x = []
   y = []
   for llc,util,ws,rounds,slowdown in data:
      if int(util) == utilization:
         x += [int(ws)]
         y += [float(slowdown)]
   print "Trend " + str(utilization)
   print x
   print y
   return x, y

x1, y1 = get_trend(1)
x2, y2 = get_trend(2)
x3, y3 = get_trend(3)
x4, y4 = get_trend(4)

# Create a Figure object.
fig = plt.figure(figsize=(5, 4))

# Create an Axes object.
ax = fig.add_subplot(1,1,1) # one row, one column, first plot

# Plot the data.
ax.plot(x1, y1, color="blue", linestyle="solid", linewidth="3", label="MUtil=1/2")
ax.plot(x2, y2, color="green", linestyle="solid", linewidth="3", label="MUtil=1/4")
ax.plot(x3, y3, color="red", linestyle="dashed", linewidth="3", label="MUtil=1/8")
ax.plot(x4, y4, color="black", linestyle="dotted", linewidth="3", label="MUtil=1/16")

legend = ax.legend(loc='upper left', shadow=True)

plt.xticks(x1)
ax.grid(True)

# Add a title.
#ax.set_title("Simple Figure of $y=x^{1.6}$")

# Add some axis labels.
ax.set_xlabel("Lg2(Working set) (in Bytes)")
ax.set_ylabel("Slowdown over native (X)")

# Produce an image.
fig.savefig("ws_slowdown_stride_L3.png")

