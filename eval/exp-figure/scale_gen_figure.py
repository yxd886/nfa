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
	tp = []
	flow = []
	lr21 = []
	lr22 = []
	lr23 = []
	lr31 = []
	lr32 = []
	lr33 = []

	i = 0
	r21 = 0
	r22 = 0
	r23 = 0
	r31 = 0
	r32 = 0
	r33 = 0
	total = 0
	with open(filename) as f:
		for line in f:
			if line.find("r21throughput")!=-1:
				r21 = float(line.split(':')[1])
				lr21.append(r21/1000000)
			elif line.find("r22throughput")!=-1:
				r22 = float(line.split(':')[1])
				lr22.append(r22/1000000)
			elif line.find("r23throughput")!=-1:
				r23 = float(line.split(':')[1])
				lr23.append(r23/1000000)
			elif line.find("r31throughput")!=-1:
				r31 = float(line.split(':')[1])
				lr31.append(r31/1000000)
			elif line.find("r32throughput")!=-1:
				r32 = float(line.split(':')[1])
				lr32.append(r32/1000000)
			elif line.find("r33throughput")!=-1:
				r33 = float(line.split(':')[1])
				lr33.append(r33/1000000)
			elif line.find("total")!=-1:
				total = float(line.split(':')[1])
				flow.append(total/1000000)
				tp.append((r21+r22+r23+r31+r32+r33)/1000000)
			#print line
			
	return tp,flow,lr21,lr22,lr23,lr31,lr32,lr33

def draw(tp, flow,l1,l2,l3,l4,l5,l6):

	#print "l1 len"+str(len(l1))
	#print "tp len"+str(len(tp))
	#print "l1"+str(l1)

	lines = [tp,flow,l1,l2,l3,l4,l5,l6]
	
	plt.style.use('ggplot')#seaborn-white')
	plt.style.use('ggplot')#seaborn-white')
	fig,ax1 = plt.subplots()

	fig = plt.figure(1)

	timeline = np.linspace(1,3*len(tp),len(tp))
	colors = ['r','b','y','m','c','g','r','b','c']
	styles = ['-.', '--', ':', '-', ':', ':', ':',':']
	index = 0;
	line_index = 0;
	linelabels = ["Throughput", "incoming traffic", "runtime1","runtime2","runtime3","runtime3","runtime4","runtime5"]
	ax1.plot(timeline, tp,  'r-', label=linelabels[0] ,linewidth=2)
	ax1.plot(timeline, flow,  'b--', label=linelabels[1],linewidth=2)
	ax1.plot(timeline, l1,  'y:', label=linelabels[2],linewidth=2)
	ax1.plot(timeline, l2,  'm-', label=linelabels[3],linewidth=2)
	#ax1.plot(timeline, l3,  'c:', label=linelabels[4],linewidth=2)
	ax1.plot(timeline, l4,  'g:', label=linelabels[5],linewidth=2)
	ax1.plot(timeline, l5,  'b:', label=linelabels[6],linewidth=2)
	ax1.plot(timeline, l6,  'y-', label=linelabels[7],linewidth=2)

	ax1.set_ylabel('Overall Throughput(Mpps)', fontsize=25, style='normal', color='black')
	ax1.set_xlabel('Time (s)', fontsize=25, style="normal", color='black')

	legend = ax1.legend(loc='top right', shadow=False)

	# Set the fontsize
	for label in legend.get_texts():
		label.set_fontsize(13)

	for label in legend.get_lines():
		label.set_linewidth(3)  # the legend line width

	for tl in ax1.get_xticklabels():
		tl.set_fontsize(15)
		tl.set_fontstyle('normal')
	for tl in ax1.get_yticklabels():
		tl.set_fontsize(15)
		tl.set_fontstyle('normal')

	plt.gcf().subplots_adjust(bottom=0.18)
	plt.savefig("Scale.pdf")
	plt.show()
	return lines

def main():

	tp,flow,l1,l2,l3,l4,l5,l6 = read_log("dynamic_scale")

	print draw(tp,flow,l1,l2,l3,l4,l5,l6) 



if __name__ == "__main__" :
	main()