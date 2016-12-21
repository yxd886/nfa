#ifndef FIREWALL_FS_H
#define FIREWALL_FS_H

struct firewall_fs
{
    firewall_fs(): counter(0){
			Reset();
    }

    void Reset()
    {
      current_pass=false;
      drop_no=0;
      match_no=0;
      pass_no=0;
    }

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
