#! /bin/sh

sudo rm -f ./core

ulimit -c unlimited

service_chain='firewall,http_parser,http_parser' #pkt_counter firewall

if ["$0" -ge "1"]
then
sudo nohup ~/nfa/runtime/samples/real_rpc_basic/server_main --runtime_id=4 --input_port_mac="52:54:02:01:00:01" \
--output_port_mac="52:54:02:01:00:02" --control_port_mac="52:54:02:01:00:03" --rpc_ip="202.45.128.155" --rpc_port=10241 \
--input_port="rt1_iport" --output_port="rt1_oport" --control_port="rt1_cport" --worker_core=1 --service_chain="${service_chain}" >/dev/null 2>&1 &
fi

if ["$0" -ge "2"]
then
sudo nohup ~/nfa/runtime/samples/real_rpc_basic/server_main --runtime_id=5 --input_port_mac="52:54:02:02:00:01" \
--output_port_mac="52:54:02:02:00:02" --control_port_mac="52:54:02:02:00:03" --rpc_ip="202.45.128.155" --rpc_port=10242 \
--input_port="rt2_iport" --output_port="rt2_oport" --control_port="rt2_cport" --worker_core=2 --service_chain="${service_chain}" >/dev/null 2>&1 &
fi

if ["$0" -ge "3"]
then
sudo nohup ~/nfa/runtime/samples/real_rpc_basic/server_main --runtime_id=6 --input_port_mac="52:54:02:03:00:01" \
--output_port_mac="52:54:02:03:00:02" --control_port_mac="52:54:02:03:00:03" --rpc_ip="202.45.128.155" --rpc_port=10243 \
--input_port="rt3_iport" --output_port="rt3_oport" --control_port="rt3_cport" --worker_core=3 --service_chain="${service_chain}" > /dev/null 2>&1 &
fi
