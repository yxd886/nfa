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
	i = 0
	with open(filename) as f:
		for line in f:
			i+=1
			print line
			tp.append(float(line))
			if i==50 :
				break
	return tp 

def draw(l1, l2):

	lines = [l1,l2]
	
	plt.style.use('ggplot')#seaborn-white')
	plt.style.use('ggplot')#seaborn-white')
	fig,ax1 = plt.subplots()
	fig = plt.figure(1)

	timeline = np.linspace(1,150,50)
	colors = ['r','b','y','m','c','g','r','b','y']
	styles = ['-.', '--', ':', '-', 'D:', 'p:', 'x:','*:']
	index = 0;
	line_index = 0;
	linelabels = ["subflow detection enabled", "subflow detection disabled", "dynamic","HP,FW","FM,FW","FM,HP,FW"]
	
	ax1.plot(timeline, l1,  'r-.', label=linelabels[0], linewidth=3)
	ax1.plot(timeline, l2,  'b--', label=linelabels[1], linewidth=3)
	
	ax1.set_ylabel('# of correctly\ndetected MPTCP flows', fontsize=23, style='normal', color='black')
	ax1.set_xlabel('Time (s)', fontsize=23, style="normal", color='black')

	# Now add the legend with some customizations.
	legend = ax1.legend(loc='middle right', shadow=False)
	

	# Set the fontsize
	for label in legend.get_texts():
		label.set_fontsize(18)

	for label in legend.get_lines():
		label.set_linewidth(3)  # the legend line width

	for tl in ax1.get_xticklabels():
		tl.set_fontsize(18)
		tl.set_fontstyle('normal')
	for tl in ax1.get_yticklabels():
		tl.set_fontsize(20)
		tl.set_fontstyle('normal')

	ax1.set_yscale('log') # if want to linearly scale the y axis, comment this line
	
	plt.gcf().subplots_adjust(bottom=0.18)
	plt.gcf().subplots_adjust(left=0.21)
	plt.savefig("Mptcp.pdf")
	plt.show()
	return lines

def main():

	tp1 = read_log("with_mptcp_result")
	tp2 = read_log("without_mptcp")

	print draw(tp1,tp2) 



if __name__ == "__main__" :
	main()