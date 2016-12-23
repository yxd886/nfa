#ifndef FIREWALL_FS_H
#define FIREWALL_FS_H

struct firewall_fs
{
	uint32_t SrcIp;
	uint32_t DstIp;
	uint16_t SrcPort;
	uint16_t DstPort;
	uint8_t protocol;
	time_t   CreatedTime;
	time_t   RefreshTime;
	int match_no;
	int drop_no;
	int pass_no;
	bool current_pass;
	int counter;
};

#endif
