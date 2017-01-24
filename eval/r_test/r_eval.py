#!/usr/bin/env python2.7
import os
import optparse
import sys
import subprocess
import signal
import time
import paramiko
import re


#I0123 21:05:52.516784 124278 coordinator_mp.cc:138] Successful recovery : 100002
#I0123 21:05:52.516810 124278 coordinator_mp.cc:139] Failed recovery : 0
#I0123 21:05:52.516814 124278 coordinator_mp.cc:144] Recovery takes 237ms.
     


def parse_arguments():
  parser = optparse.OptionParser()

  parser.add_option('', '--r2', action="store", type="int", dest="r2_number", help="How many rts in r2", default="1")

  parser.add_option('', '--r3', action="store", type="int", dest="r3_number", help="How many rts in r3", default="1")

  parser.add_option('', '--flow', action="store", type="int", dest="flow_number", help="Flows to migrate", default="50000")

  options, args = parser.parse_args()

  return options,args

def start_recovery():
  cmd="sudo ~/nfa/runtime/samples/replication/r_recover"
  process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
  output, error = process.communicate()
  print "Recover started and wait 5 seconds"

  time.sleep(5)


def detect_result(options, ssh, runtime):
  #for x in range(1, options.r3_number+1):
  cmd="cat /home/net/nfa/eval/r_test/rt"+str(runtime)+"_log.log"
  stdin,stdout,stderr =  ssh.exec_command(cmd)
  success_flag = False
  time_result = ""
  flow_result = ""
  number = ""
  for line in stdout:

    if line.find("Successful recovery")!=-1:
      number = line[line.find("Successful recovery"):]
      success_flag = True
    if line.find("Recovery takes") > 0:
      time_result = line[line.find("Recovery takes")+15:-2]
  time.sleep(1)
  return str(success_flag)+" "+str(number)+" "+str(time_result)

def test_recovery():

  options,args = parse_arguments()

  ssh_r2 = paramiko.SSHClient()
  ssh_r2.set_missing_host_key_policy(paramiko.AutoAddPolicy())
  ssh_r2.connect('202.45.128.155',username='net',password='netexplo')


  ssh_r3 = paramiko.SSHClient()
  ssh_r3.set_missing_host_key_policy(paramiko.AutoAddPolicy())
  ssh_r3.connect('202.45.128.156',username='net',password='netexplo')


  start_recovery()
  
  print "r3 rt1 recovery - "+str(detect_result(options, ssh_r3, 1))
  print "r3 rt2 recovery - "+str(detect_result(options, ssh_r3, 2))
  print "r3 rt3 recovery - "+str(detect_result(options, ssh_r3, 3))




def main():


  test_recovery()


if __name__ == "__main__" :
  main()
