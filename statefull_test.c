#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <stdio.h>
#include "structs.h"
// from linux mag
static unsigned int s_hook(unsigned int hook, struct sk_buff **pskb, const struct net_device *indev, const struct net_device *outdev, int (*okfn)(struct sk_buff *));

static int main(int argc, char *argv[]){

}

static int __init init(void) {
  return nf_register_hook(&ops);
}

static void __exit fini(void){
  nf_unregister_hook(&ops);
}

module_init(init);
module_exit(fini);
// ^ from linux mag

static struct nf_hook_ops ops = { { NULL, NULL }, hook, PF_INET, NF_IP_LOCAL_OUT, NF_IP_PRI_FILTER-1 };

struct EthHeader GetHeader(char buffer[14]){ // I should fork the process to get the header and reduce over head
  int i;

  struct EthHeader eth = new struct EthHeader;

  for (i = 0; i < 6; i++) {
    eth.d_mac[i] = buffer[i];
  }
  for (i = 0; i < 6; i++) {
    eth.s_mac[i] = buffer[i+6];
  }
  for (i = 0; i < 2; i++) {
    eth.type[i] = buffer[i+12];
  }
  return eth;
}

static unsigned int s_hook(unsigned int hook, struct sk_buff **pskb, const struct net_device *indev, const struct net_device *outdev, int (*okfn)(struct sk_buff *)) {

  /* Get a handle to the packet data */
  unsigned char *data = (void *)(*pskb)->nh.iph + (*pskb)->nh.iph->ihl*4;//(pskb pointing to nh.iph(header of packet))+(pskb points to nh.iph points to ihl multiplied by 4)
  (*pskb)->nfcache |= NFC_UNKNOWN; // Bitwise OR assignment | compare what matches between pskb and nf_cache and output that to nfcache

  /*
  I need to modify the header to change (at layer 2) the destination MAC to a new machine
  d_mac is the first element in the header, change this to FF:FF:FF:FF:FF:FF to broadcast for some reason
  Final goal is to modify the IP header, need to find out how deep in that is
  Need to also make this specific to packet types, ping is for control right now
  */
  if (len(data) == 100){ // ping packet size
        //printk(“moddifying d_mac\n”); //kernel mode because f is not available this low level

        char *buff[]  = GetHeader(data); // Grab the header of data before we change it, read the header into a character array

        int i;

        for(i=0; i < 6; i=i+1){
          // I want to iterate through a character array containing the modified destination and then write that to the buff
          char *temp_array[] = {"FF:FF:FF:FF:FF:FF"};
          buff[i] = temp_array[i]; // iterate through buffer, replacing it with what temp_array has in that position
        }
        return NF_ACCEPT;
    }

  else if (len(data) == 200){
        // evident how droppign can work for firewalls in the future
        return NF_DROP; // Dropping packet if size 200
      }

  else {
        return NF_ACCEPT;

   }
}
