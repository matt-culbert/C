#include <linux/config.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <stdio.h>
#include "structs.h"
// from linux mag
static unsigned int hook(unsigned int hook, struct sk_buff **pskb, const struct net_device *indev, const struct net_device *outdev, int (*okfn)(struct sk_buff *));

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

struct EthHeader GetHeader(char buffer[14]){
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

static unsigned int hook(unsigned int hook, struct sk_buff **pskb, const struct net_device *indev, const struct net_device *outdev, int (*okfn)(struct sk_buff *)) {

  /* Get a handle to the packet data */
  unsigned char *data = (void *)(*pskb)->nh.iph + (*pskb)->nh.iph->ihl*4;//(pskb pointing to nh.iph(header of packet))+(pskb points to nh.iph points to ihl multiplied by 4)
  (*pskb)->nfcache |= NFC_UNKNOWN; // Bitwise OR assignment | compare what matches between pskb and nf_cache and output that to nfcache

  if (data == 100){ // ping packet
        printk(“moddifying d_mac\n”); //kernel mode because f is not available this low level

        struct EthHeader header = GetHeader(data); // Grab the header of data before we change it

        char *buff[] = header; // read the header into a character array
        /*
        I need to modify the header to change (at layer 2) the destination MAC to a new machine
        d_mac is the first element in the header, change this to FF:FF:FF:FF:FF:FF to broadcast for some reason
        */
        for(i=0; i < 6; i++){
          // I want to iterate through a character array containing the modified destination and then write that to the buff
          *temp_array = "FF:FF:FF:FF:FF:FF";
          buff[i] = temp_array[i];

        }
        /*
        corrupts data
        data[99]++; // data , add 99 to data
        (*pskb)->nfcache |= NFC_ALTERED;
        return NF_ACCEPT;
        */
  else if (data == 200){
        printk(“dropping packet\n”); // evident how droppign can work for firewalls in the future
        return NF_DROP; // Dropping packet if case 200 met
      }

  else {
        return NF_ACCEPT;
      }
   }
}
