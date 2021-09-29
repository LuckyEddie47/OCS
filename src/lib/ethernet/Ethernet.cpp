// global ethernet variables
#include "Ethernet.h"

#if OPERATIONAL_MODE == ETHERNET_W5100 || OPERATIONAL_MODE == ETHERNET_W5500

  // flag that ethernet initialization has been completed
  bool eth_active = false;

  // a unique MAC address for your controller if you like:
  byte eth_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

  // the IP addresses below dependent on the local network:
  bool eth_dhcp_enabled = STA_DHCP_ENABLED;
  IPAddress eth_ip = IPAddress STA_IP_ADDR;
  IPAddress eth_dns= IPAddress STA_GW_ADDR;
  IPAddress eth_gw = IPAddress STA_GW_ADDR;
  IPAddress eth_sn = IPAddress STA_SN_MASK;
#endif