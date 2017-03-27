#!/usr/bin/env python2.7
import os
import optparse
import sys
import subprocess
import signal
import time

import numpy as np
import matplotlib.pyplot as plt


def read_log(filename):
	runtimes = []
	received = []
	dropped = []
	time = []
	with open(filename) as f:
		for line in f:
			if line.find("[RESULT]") != -1:
				numbers = line.split(' ')
				received.append(float(numbers[1]))
				dropped.append(float(numbers[2]))
				time.append(float(numbers[3]))

	return runtimes, received, dropped, time 

def draw(received, dropped, time):

	lines = []
	line = []
	loop = 0
	for i in range(0,len(received)):
		line.append(received[i]/(time[i])/1000)
		loop+=1
		if loop ==6:
			loop = 0
			lines.append(line)
			line = []
	new_lines = lines
	lines = []
	lines.append(new_lines[0])
	lines.append(new_lines[2])
	lines.append(new_lines[5])
	lines.append(new_lines[6])
	
	plt.style.use('ggplot')#seaborn-white')

	runtimes = [1,2,3,4,5,6]
	colors = ['r','b','y','m','c','g','r','b','y']
	styles = ['s-.', 'o--', '<:', '^:', 'D:', 'p:', 'x:','*:']
	linelabels = ["FM-FW","FW-LB", "FM-LB","FM-HP-IDS"]


	plt.style.use('ggplot')#seaborn-white')
	fig,ax1 = plt.subplots()

	fig = plt.figure(1)

	ax1.plot(runtimes, lines[0],   'rs-', linewidth=3, label=linelabels[0], markersize=10)
	ax1.plot(runtimes, lines[1],   'bo--',linewidth=3, label=linelabels[1], markersize=10)
	ax1.plot(runtimes, lines[2],   'y<:', linewidth=3, label=linelabels[2], markersize=10)
	ax1.plot(runtimes, lines[3],   'm^:', linewidth=3, label=linelabels[3], markersize=10)

	xticks = [1, 2, 3, 4, 5, 6]
	ax1.set_xticks(xticks)	

	ax1.set_ylabel('Throughput(Mpps)', fontsize=25, style='normal', color='black')
	ax1.set_xlabel('# of runtimes', fontsize=25, style="normal", color='black')

	# Now add the legend with some customizations.
	legend = ax1.legend(loc='lower right', shadow=False)
	

	# Set the fontsize
	for label in legend.get_texts():
		label.set_fontsize(18)

	for label in legend.get_lines():
		label.set_linewidth(3)  # the legend line width

	for tl in ax1.get_xticklabels():
		tl.set_fontsize(25)
		tl.set_fontstyle('normal')
	for tl in ax1.get_yticklabels():
		tl.set_fontsize(25)
		tl.set_fontstyle('normal')

	fig.savefig('ReplicaTP.pdf', bbox_inches='tight', pad_inches=0)
	plt.show()

	return lines

def main():

	runtimes,received,dropped,time = read_log("rttemp")

	print draw(received, dropped, time) 



if __name__ == "__main__" :
	main()