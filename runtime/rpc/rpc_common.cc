#include "rpc_common.h"


int parse_mac_addr(char *addr, const char *str ){
	if (str != NULL && addr != NULL) {
		int r = sscanf(str,
				"%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
			       addr,
			       addr+1,
			       addr+2,
			       addr+3,
			       addr+4,
			       addr+5);

		if (r != 6)
			return -EINVAL;
	}

	return 0;
}

int encode_mac_addr(char *str, char *addr){
	if (str != NULL && addr != NULL) {
		int r = sprintf(str,
			       "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
			       addr,
			       addr+1,
			       addr+2,
			       addr+3,
			       addr+4,
			       addr+5);

		if (r != 6)
			return -EINVAL;
	}

	return 0;
}

int parse_ip_addr(char *addr, const char *str ){
	if (str != NULL && addr != NULL) {
		strcpy(addr,str);
	}

	return 0;
}



int encode_ip_addr( char *str ,char *addr )
{
	if (str != NULL && addr != NULL) {
		strcpy(str,addr);
	}

	return 0;
}


void view_rpc2local(Local_view* local_view, View source){
	local_view->worker_id=source.worker_id();
	parse_mac_addr(local_view->control_port_mac,source.control_port_mac().c_str());
	parse_mac_addr(local_view->input_port_mac,source.input_port_mac().c_str());
	parse_mac_addr(local_view->output_port_mac,source.output_port_mac().c_str());
	parse_ip_addr(local_view->rpc_ip,source.rpc_ip().c_str());
	local_view->rpc_port=source.rpc_port();
	}

void view_local2rpc(View* rpc_view_ptr, Local_view local_view ){
	char str_tmp[20];
	rpc_view_ptr->set_worker_id(local_view.worker_id);
	encode_mac_addr(str_tmp,local_view.control_port_mac);
	rpc_view_ptr->set_control_port_mac(str_tmp);
	encode_mac_addr(str_tmp,local_view.input_port_mac);
	rpc_view_ptr->set_input_port_mac(str_tmp);
	encode_mac_addr(str_tmp,local_view.output_port_mac);
	rpc_view_ptr->set_output_port_mac(str_tmp);
	encode_ip_addr(str_tmp,local_view.rpc_ip);
	rpc_view_ptr->set_rpc_ip(str_tmp);
	rpc_view_ptr->set_rpc_port(local_view.rpc_port);
}
