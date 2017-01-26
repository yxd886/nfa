#!/usr/bin/env python2.7
import os
import optparse
import sys
import subprocess
import signal
import time
import paramiko
import re

def main():

  throughput= []
  flowgen=[]
  #cmd="python ./catch_data.py | tee data.log"
  #process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, preexec_fn=os.setsid)
  
  cmd="sudo ../../deps/bess/bessctl/bessctl add connection fg6 rt6_iport_portout"
  process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, preexec_fn=os.setsid)
  time.sleep(3)  
  cmd=" ../../runtime/samples/dynamic_scale"
  process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, preexec_fn=os.setsid)
    
  for i in range(5):
    
    cmd="sudo ../../deps/bess/bessctl/bessctl add connection fg"+str(i+1)+" rt"+str(i+1)+"_iport_portout"
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, preexec_fn=os.setsid)
    time.sleep(3)
    
  for i in range(6):
    cmd="sudo ../../deps/bess/bessctl/bessctl delete connection fg"+str(i+1)+" ogate"
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, preexec_fn=os.setsid)
  
    time.sleep(3)   
  
  
if __name__ == '__main__':
    main()
