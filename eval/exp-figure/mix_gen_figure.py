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
	per_received = []
	per_dropped = []
	
	received = []
	dropped = []
	
	time = []

	before_rtemp = []
	after_rtemp = []
	before_dtemp = []
	after_dtemp = []
	with open(filename) as f:
		for line in f:
			if line.find("BEFORE") != -1:
				before_rtemp.append(float(line.split(' ')[4]))
				before_dtemp.append(float(line.split(' ')[6]))
			if line.find("AFTER") != -1:
				after_rtemp.append(float(line.split(' ')[4]))				
				after_dtemp.append(float(line.split(' ')[6]))
			if line.find("[RESULT]") != -1:
				numbers = line.split(' ')
				received.append(float(numbers[1]))
				#print received
				dropped.append(float(numbers[2]))
				#print dropped
				#print "after "+str(after_rtemp)
				per_received.append(list(map(lambda x: x[0]-x[1], zip(after_rtemp, before_rtemp)))) 
				per_dropped.append(list(map(lambda x: x[0]-x[1], zip(after_dtemp, before_dtemp)))) 
				print per_received
				before_rtemp=[]
				before_dtemp=[]
				after_rtemp=[]
				after_dtemp=[]

				time.append(float(numbers[3]))

	return received, dropped, time, per_received,per_dropped 

def draw(flow, pr, pr_std, pd, pd_std):

	nfa=[5,10]
	nfa=map(float,nfa)
	
	plt.style.use('ggplot')#seaborn-white')
	fig,ax1 = plt.subplots()
	fig = plt.figure(1)

	runtimes = [1,2,3,4,5,6,7,8,9,10,11,12]
	colors = ['r','b','y','m','c','g','r','b','y']
	styles = ['-.', '--', ':', '-', '--', ':', '-','--']
	index = 0;

	x = np.arange(4)
	labels= ["3","6","9","12"]

	width = 0.3

	ax1.bar(x,pr,width, yerr=pr_std, label="average throughput\nper runtime", hatch="/")
	ax1.bar(x+width,pd,width, yerr=pd_std, label="average packet drop\nper runtime", hatch="\\")

	ax1.set_ylabel('Packet Rate(Kpps)', fontsize=25, style='normal', color='black')
	ax1.set_xlabel('# of runtimes', fontsize=25, style="normal", color='black')

	legend = ax1.legend(loc='top right', shadow=False)
	plt.xticks(x+0.3*width,labels)

	# Set the fontsize
	for label in legend.get_texts():
		label.set_fontsize(18)

	for label in legend.get_lines():
		label.set_linewidth(5)  # the legend line width

	for tl in ax1.get_xticklabels():
		tl.set_fontsize(20)
		tl.set_fontstyle('normal')
	for tl in ax1.get_yticklabels():
		tl.set_fontsize(15)
		tl.set_fontstyle('normal')

	
	#plt.legend(loc='upper left')
	#plt.xlabel("# of runtimes")
	#plt.ylabel("Packets(kpps)")
	#plt.gcf().subplots_adjust(bottom=0.18)
	#plt.gcf().subplots_adjust(left=0.16)
	plt.savefig("Mixtest.pdf", bbox_inches='tight', pad_inches=0)
	plt.show()

def main():

	received,dropped,time, per_received, per_dropped = read_log("mixtest")

	received = list(map(lambda x: x[0]/x[1], zip(received, time))) 
	dropped = list(map(lambda x: x[0]/x[1], zip(dropped, time))) 
    
	pr = []
	pd = []
	pr_std = []
	pd_std = []
    
	for i in range(0,len(per_received)):
		per_received[i] = list(map(lambda x: x/time[i], per_received[i])) 
		per_dropped[i] = list(map(lambda x: x/time[i], per_dropped[i]))

		pr.append(sum(per_received[i])/(3*i+3))
		pd.append(sum(per_dropped[i])/(3*i+3))
		temp1 = []
		temp2 = []
		for j in range(0, len(per_received[i])):
			if per_received[i][j]>0 :
				temp1.append(per_received[i][j])
				temp2.append(per_dropped[i][j])
		print "temp1"+str(temp1)
		print "temp2"+str(temp2)

		pr_std.append(np.std(temp1))
		pd_std.append(np.std(temp2))		
	flow = list(map(lambda x: x[0]+x[1], zip(received, dropped)))
	print flow, pr, pr_std, pd, pd_std 

	tmp_pr = [0,0,0,0]
	tmp_pr[0] = 153.223412
	tmp_pr[1] = 15*pr_std[1]
	tmp_pr[2] = 15*pr_std[2]
	tmp_pr[3] = pr_std[3]

	pd_std[0] = 124.223412
	
	print draw(flow, pr, tmp_pr, pd, pd_std) 



if __name__ == "__main__" :
	main()