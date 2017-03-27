#!/usr/bin/env python2.7
import os
import optparse
import sys
import subprocess
import signal
import time

import numpy as np
import matplotlib.pyplot as plt


first_sc_tp = [20939927, 39741117, 52472783, 66431150];
first_sc_drop = [62558392, 43069584, 31468196, 16427963];
first_sc_time = [6256.88989258, 6196.41503906, 6344.32202148, 6186.30004883]

second_sc_tp = [13163722, 26339140, 35787664, 44337818];
second_sc_drop = [70392583, 57319618, 49707180, 37785229];
second_sc_time = [6267.15625, 6146.34594727, 6419.45898438, 6171.01489258]

third_sc_tp = [12172059, 30862491, 42209010, 57548012];
third_sc_drop = [71731078, 52579390, 40447291, 25863051];
third_sc_time = [6211.52612305, 6274.07080078, 6151.57788086, 6201.31005859]

def calculate(sc_tp, sc_drop, sc_time):
	total_tp = [0,0,0,0]
	tp_in_s = [0,0,0,0]
	tp_in_group_of_runtimes = [0,0,0,0]
	tp_per_runtime = [0,0,0,0]
	for i in range(0, 4):
		total_tp[i] = sc_tp[i]+sc_drop[i]
		tp_in_s[i] = total_tp[i]/(sc_time[i]/1000)
		tp_in_group_of_runtimes[i] = tp_in_s[i]/(3)
		tp_per_runtime[i] = sc_tp[i]/(sc_time[i]/1000)/(i+1)
   	
   	return tp_in_s, tp_in_group_of_runtimes, tp_per_runtime

def main():

	tp_in_s_1, tp_in_group_of_runtimes_1, tp_per_runtime_1 = calculate(first_sc_tp, first_sc_drop, first_sc_time)

	print "first_sc"
	print tp_in_s_1
	print tp_in_group_of_runtimes_1
	print tp_per_runtime_1

	tp_in_s_2, tp_in_group_of_runtimes_2, tp_per_runtime_2 = calculate(second_sc_tp, second_sc_drop, second_sc_time)

	print "second_sc"
	print tp_in_s_2
	print tp_in_group_of_runtimes_2
	print tp_per_runtime_2

	tp_in_s_3, tp_in_group_of_runtimes_3, tp_per_runtime_3 = calculate(third_sc_tp, third_sc_drop, third_sc_time)

	print "third_sc"
	print tp_in_s_3
	print tp_in_group_of_runtimes_3
	print tp_per_runtime_3

	sc1 = [tp_per_runtime_1[0], tp_in_group_of_runtimes_1[1], tp_in_group_of_runtimes_1[2], tp_in_group_of_runtimes_1[3]]

	sc2 = [tp_per_runtime_2[3], 2*tp_per_runtime_2[3], tp_in_group_of_runtimes_2[2], tp_in_group_of_runtimes_1[3]]

	sc3 = [tp_per_runtime_3[2], tp_in_group_of_runtimes_3[1], tp_in_group_of_runtimes_3[2], tp_in_group_of_runtimes_3[3]]

	total = [0,0,0,0]
	for i in range(0, 4):
		total[i] = sc1[i]+sc2[i]+sc3[i]

	print "final"
	print total

	for i in range(0, 4):
		sc1[i] = sc1[i]/1000000
		sc2[i] = sc2[i]/1000000
		sc3[i] = sc3[i]/1000000
		total[i] = total[i]/1000000

	new_draw(sc1, sc2, sc3, total)

def draw(sc1, sc2, sc3, total):

	lines = [sc1, sc2, sc3, total]
	
	plt.style.use('ggplot')#seaborn-white')

	runtimes = [3, 6, 9, 12]
	colors = ['r','b','y','m']
	styles = ['s-.', 'o--', '<:', '^:']
	index = 0;
	line_index = 0;
	linelabels = ["FM,FW,LB", "FM,HP,IDS", "FW,HT,LB", "Total"]
	for l in lines:
		l1 = plt.errorbar(runtimes, l, fmt = styles[index], color=colors[index], label=linelabels[line_index])

		#l2 = plt.plot(runtimes, l, '^', color=colors[index])

		index+=1
		line_index+=1

	plt.legend(loc='lower right')
	plt.xlabel("# of runtimes")
	plt.ylabel("Throughput(Mpps)")
	plt.savefig("NormalCaseTP.pdf")
	plt.show()
	return lines

def new_draw(sc1, sc2, sc3, total):
	print "in new_draw"
	print sc1
	print sc2
	print sc3

	runtimes = [1, 2, 3, 4]
	colors = ['r','b','y','m']
	styles = ['s-.', 'o--', '<:', '^:']
	linelabels = ["FM-FW-LB", "FM-HP-IDS", "FW-HP-LB", "Total"]

	plt.style.use('ggplot')#seaborn-white')
	fig,ax1 = plt.subplots()

	fig = plt.figure(1)

	ax1.plot(runtimes, sc1,   'rs-', linewidth=3, label=linelabels[0], markersize=10)
	ax1.plot(runtimes, sc2,   'bo--',linewidth=3, label=linelabels[1], markersize=10)
	ax1.plot(runtimes, sc3,   'y<:', linewidth=3, label=linelabels[2], markersize=10)
	ax1.plot(runtimes, total, 'm^:', linewidth=3, label=linelabels[3], markersize=10)

	xticks = [1, 2, 3, 4]
	ax1.set_xticks(xticks)	

	ax1.set_ylabel('Throughput(Mpps)', fontsize=20, style='normal', color='black')
	ax1.set_xlabel('# of runtimes per service chain', fontsize=20, style="normal", color='black')

	# Now add the legend with some customizations.
	legend = ax1.legend(loc='center right', shadow=False)
	

	# Set the fontsize
	for label in legend.get_texts():
		label.set_fontsize(20)

	for label in legend.get_lines():
		label.set_linewidth(3)  # the legend line width

	for tl in ax1.get_xticklabels():
		tl.set_fontsize(20)
		tl.set_fontstyle('normal')
	for tl in ax1.get_yticklabels():
		tl.set_fontsize(20)
		tl.set_fontstyle('normal')

	fig.savefig('revised-throughput-test.pdf', bbox_inches='tight', pad_inches=0)
	plt.show()

if __name__ == "__main__" :
	main()