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
	numbers = []
	with open(filename) as f:
		for line in f:
			number = []
			if line.find("[RESULT]") != -1:
				number = map(float, line.split(' ')[1:-1])
				numbers.append(number)
	return numbers

def draw(numbers):
	n = []
	eb = []
	for i in range(0,len(numbers)):
		n.append(sum(numbers[i])/len(numbers[i]))
		eb.append(np.std(numbers[i]))
	
	print n
	plt.style.use('ggplot')
	labels= ["10K","20K","30K","40K","50K"]
	colors = ['r','b','y','m','c','g','r','b','y']
	styles = ['-.', '--', ':', '-', '--', ':', '-','--']
	index = 0;
	line_index = 0;

	fig,ax1 = plt.subplots()
	loss = [10,20,30,40,50]
	x = np.arange(5)
	width = 0.25
	ax1.plot(x,n,'D',color='r', linewidth=3, markersize=10)
	ax1.errorbar(x, n, fmt=':',color='r', yerr=eb, linewidth=3)
	plt.xticks(x, labels)
	ax1.set_xlabel("# of recovered flows per runtime", fontsize=25, style='normal', color='black')
	ax1.set_ylabel("Total recovery time (ms)", fontsize=25, style='normal', color='black')

	for tl in ax1.get_xticklabels():
		tl.set_fontsize(20)
		tl.set_fontstyle('normal')
	for tl in ax1.get_yticklabels():
		tl.set_fontsize(20)
		tl.set_fontstyle('normal')

	#ax2 =ax1.twinx()
	#ax2.set_ylim([0,1000])
	#l2=ax2.bar(x+width, loss, width, color='b')
	#ax2.set_ylabel("Packet Loss")
	plt.savefig("Recover.pdf", bbox_inches='tight', pad_inches=0)
	plt.show()
	#legend([l1[0], l2[0]], ['Migration Time', 'Migration Loss'], loc="upper left")
	return n

def main():

	numbers = read_log("r_test.txt")

	print draw(numbers)	 



if __name__ == "__main__" :
	main()