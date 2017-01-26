#!/usr/bin/env python2.7
import os
import optparse
import sys
import subprocess
import signal
import time
import paramiko
import re
cmd_stop = './stop.sh'
cmd_start = './start.sh'
FNULL = open(os.devnull, 'w')
def parse_arguments():
  parser = optparse.OptionParser()



  parser.add_option('', '--ip', action="store", type="string", dest="ip", help="ip", default="202.45.128.155")

  parser.add_option('', '--local_id', action="store", type="int", dest="local_id", help="local_id", default="1")


  options, args = parser.parse_args()

  return options,args







def read_pkts(ssh,rt_num):
  cmd="sudo ~/nfa/deps/bess/bessctl/bessctl show port rt"+str(rt_num)+"_iport"
  
  print cmd
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


def test():
  options,args = parse_arguments()
  #print "Start Test with the following options:"
  #print options

  #print "Creating SSH to R2 & R3"

  ssh = paramiko.SSHClient()
  ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
  ssh.connect(options.ip,username='net',password='netexplo')
  ssh.exec_command('cd ~/nfa/eval/r_test')
  
  recovery_time = "";
  migration_time = "";
  before_received = 0;
  before_dropped = 0;
  after_received = 0;
  after_dropped = 0;
  before_time = 0;
  after_time = 0;

  tmp1,tmp2 = read_pkts(ssh,options.local_id)
  before_received +=tmp1;
  before_dropped +=tmp2;
  

  before_time = time.time() * 1000

  time.sleep(3)

  tmp1,tmp2 = read_pkts(ssh,options.local_id)
  after_received +=tmp1;
  after_dropped +=tmp2;


  after_time = time.time() * 1000

  ssh.close()
  return (after_received-before_received)/3, after_dropped-before_dropped, after_time-before_time


def main():


  packet_out, packet_dropped, duration_time = test()
  print str(packet_out)
  print str(packet_dropped)

if __name__ == '__main__':
    main()
