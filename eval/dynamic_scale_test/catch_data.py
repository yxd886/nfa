#!/usr/bin/env python2.7
import os
import optparse
import sys
import subprocess
import signal
import time
import paramiko
import re

def read_pkts(ssh,rt_num):
  cmd="sudo ~/nfa/deps/bess/bessctl/bessctl show port rt"+str(rt_num)+"_iport"
  stdin,stdout,stderr = ssh.exec_command(cmd);

  received_pkts_line = ''
  dropped_pkts_line = ''

  i = 0
  for line in stdout:
    if i == 6:
      received_pkts_line = line
    if i == 7:
      dropped_pkts_line = line
    i=i+1


  return long(received_pkts_line.split(":")[1].replace(',', '')), long(dropped_pkts_line.split(":")[1].replace(',', ''))

def local_read_pkts(rt_num):
  cmd="sudo ~/nfa/deps/bess/bessctl/bessctl show port rt"+str(rt_num)+"_iport"
  process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, preexec_fn=os.setsid)
  output1, error1=process.communicate()
  
  received_pkts_line = ''
  dropped_pkts_line = ''

  i = 0
  for line in output1:
    if i == 6:
      received_pkts_line = line
    if i == 7:
      dropped_pkts_line = line
    i=i+1


  return long(received_pkts_line.split(":")[1].replace(',', '')), long(dropped_pkts_line.split(":")[1].replace(',', ''))
def test():

  ssh_r2 = paramiko.SSHClient()
  ssh_r2.set_missing_host_key_policy(paramiko.AutoAddPolicy())
  ssh_r2.connect('202.45.128.155',username='net',password='netexplo')
  ssh_r2.exec_command('cd ~/nfa/eval/r_test')

  ssh_r3 = paramiko.SSHClient()
  ssh_r3.set_missing_host_key_policy(paramiko.AutoAddPolicy())
  ssh_r3.connect('202.45.128.156',username='net',password='netexplo')
  ssh_r3.exec_command('cd ~/nfa/eval/r_test')

  recovery_time = "";
  migration_time = "";
  before_received = 0;
  before_dropped = 0;
  after_received = 0;
  after_dropped = 0;
  before_time = 0;
  after_time = 0;

#  if options.test_type == "THROUGHPUT":
  print "Start Testing Throughput"

  tmp1,tmp2 = read_pkts(ssh_r2,1)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,2)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,3)
  before_received +=tmp1;
  before_dropped +=tmp2;
  
  tmp1,tmp2 = read_pkts(ssh_r2,4)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,5)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,6)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,1)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,2)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,3)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,4)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,5)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,6)
  before_received +=tmp1;
  before_dropped +=tmp2;
  
  before_time = time.time() * 1000

  time.sleep(3)

  tmp1,tmp2 = read_pkts(ssh_r2,1)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,2)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,3)
  after_received +=tmp1;
  after_dropped +=tmp2;
  tmp1,tmp2 = read_pkts(ssh_r2,4)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,5)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r2,6)
  after_received +=tmp1;
  after_dropped +=tmp2;
  
  tmp1,tmp2 = read_pkts(ssh_r3,1)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,2)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,3)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,4)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,5)
  after_received +=tmp1;
  after_dropped +=tmp2;

  tmp1,tmp2 = read_pkts(ssh_r3,6)
  after_received +=tmp1;
  after_dropped +=tmp2;

  after_time = time.time() * 1000

  print "Stop testing..."
  ssh_r2.close()
  ssh_r3.close()
  #stop_traffic_gen(options)
  time.sleep(1)
  return (after_received-before_received)/3, after_dropped-before_dropped, after_time-before_time


def local_test():

  recovery_time = "";
  migration_time = "";
  before_received = 0;
  before_dropped = 0;
  after_received = 0;
  after_dropped = 0;
  before_time = 0;
  after_time = 0;

#  if options.test_type == "THROUGHPUT":
  print "Start Testing Throughput"

  tmp1,tmp2 = local_read_pkts(1)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(2)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(3)
  before_received +=tmp1;
  before_dropped +=tmp2;
  
  tmp1,tmp2 = local_read_pkts(4)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(5)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(6)
  before_received +=tmp1;
  before_dropped +=tmp2;


  
  before_time = time.time() * 1000

  time.sleep(3)

  tmp1,tmp2 = local_read_pkts(1)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(2)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(3)
  before_received +=tmp1;
  before_dropped +=tmp2;
  
  tmp1,tmp2 = local_read_pkts(4)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(5)
  before_received +=tmp1;
  before_dropped +=tmp2;

  tmp1,tmp2 = local_read_pkts(6)
  before_received +=tmp1;
  before_dropped +=tmp2;


  after_time = time.time() * 1000

  return (after_received-before_received)/3, after_dropped-before_dropped, after_time-before_time

def main():

  throughput= []
  flowgen=[]
    
  for i in range(50):
    packet_out, packet_dropped, duration_time = test()
    local_packet_out, local_packet_dropped, local_duration_time = local_test()
    print str(packet_out)
    print str(local_packet_out)
    throughput.append(packet_out)
    flowgen.append(local_packet_out)
    time.sleep(1)
    
  print "throughput:"
  for i in range(len(throughput)):
    print throughput[i]
      
  print "flowgen:"
  for i in range(len(flowgen)):
    print flowgen[i]

  
  
if __name__ == '__main__':
    main()
