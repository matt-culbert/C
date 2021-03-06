// Putting a shell script infront of this would be nice. It would manage modifying certain elements and the reloading of it into kernel space
// test use case: ping client 1, but redirect to client 2
// have ip in decimal, try getting our sbnet in decimal form, then & these two to see where it lines up then compare with our subnet to block

/*

What this does:
For every incoming packets 
Run the below code on it 
execute options based on what is seen
subnet connections 
Firewall subnet communications

What this doesn't do:
Add routes
Dole out DHCP leases
Deep packet analysis and breaking of SSL
Deal with DNS
This is not IPTables
This is not a router

This is a firewall

*/

// Define the dotted decimal versus binary format for incoming packets 
// if dotted decimal isn't defined, default to defining as off
// This is normally defined when calling it, but I'm hard coding it to off for now
#ifndef DOTTED_DECIMAL
#define DOTTED_DECIMAL  0 // I want to return the array as a decimal char string 
#endif

// Bits versus bytes definition 
#if DOTTED_DECIMAL
#   define NBITS    8
#   define SEP      "."
#else /* BINARY */
#   define NBITS 1
#   define SEP      ""
#endif

#define MASK    ((1 << NBITS) - 1)   /* 100..00 - 1 = 011..11, with NBITS `1` bits */
// ------------------------------------------------------------------------------------
// Standard include, hardest thing was keeping this in order so it was called correctly
// ------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/inet.h>
// ------------------------------------------------------------------------------------
// command line argument | called using insmod kernel_firewall.ko dropicmp=1 
// Add protocol blocking over subnet
// ------------------------------------------------------------------------------------
// Here we are defining variables that we are using in our code later on
// Theres no exact order to how it is defined, just adding definitions as I go along
static struct 	nf_hook_ops nfho; // This defines our hook operation for first run  
struct 			iphdr *iph; // This is the IP header pointer
struct 			tcphdr *tcp_header; // Our TCP pointer 
struct 			sk_buff *sock_buff; // Socket buffer pointer
unsigned int 	sport, dport; // The destination port and source port for identifying purposes 
long 			mask; // Our mask, this has gone through many different type declarations to figure out the best type 
int 			dropicmp = 0; // These are for our module parameters deciding whether to drop traffic 
int 			droptcp = 0; // ^^ 
int 			bufa [32]; // This is part of a function in testing to deny subnets 
int 			y;// *
int				i;// *
int 			z;// These are also for testing subnets 
unsigned int 	*bufd; // This is used for comparing packets in our subnetting portion 
unsigned 		num = 0; // This is for the subnetting portion it's a counter iterating during ticks over the mask 

// ------------------------------------------------------------------------------------
// change to array, use for loop and go through and & each of them <- notes to self
char *ip_formatted(long ip, char *sep, char *buff, size_t buffsz);

// ------------------------------------------------------------------------------------
// Module params are how things are passed from the command line when calling insmod
// These are defined here for no particular reason, just need to be in front of the main body function
module_param(dropicmp, int , 0444); // takes in an int from command line | (name, variable, permissions)
module_param(droptcp, int , 0444); 

// ------------------------------------------------------------------------------------
// Here is our main body
// Of note is that it is one function. 
// The reason being is that it is called for every packet coming through the machine, 
// so there is no reason for a while true or anything like that
// ------------------------------------------------------------------------------------

unsigned int hook_func(unsigned int hooknum,
                       struct sk_buff **skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn)(struct sk_buff *)){

// ------------------------------------------------------------------------------------
// This is where we begin testing the subnetting functionality of this project
// This uses a mask array and a mask subnet. Both parts are needed so as to know what range we are blocking. 
// With only one part or the other we would not be blocking properly
    unsigned mask_array = 0b11111111111111111111111100000000; // mask of /24
    unsigned masked_sub = 0b11000000101010001000001100000000; // 192.168.67.0 | we are comparing the output of a binary & to this

    sock_buff = skb; // renaming things is fun 

// ---------------------------------------------------------------------------
// This stuff is fairly simple if then procedurals identifying protocols
    if (!sock_buff) { // if there is no socket buffer, accept
        return NF_ACCEPT;
    }

    iph = (struct iphdr *)skb_network_header(sock_buff); // using the socket buffer, create our ip header structure out of packets in it
    tcp_header = (struct tcphdr *)skb_network_header(sock_buff); // this was implemented for some form of future deep packet analysis

    if (!iph) {
        printk(KERN_INFO "no ip header, dropping\n");
        return NF_DROP;
    }
    
	// This stuff has to do with the subnetting portion. 
	// We convert it to a binary format and use this later.
	// Right now it does nothing
    char line[1024];
    unsigned long ip_netfmt = iph->saddr; // convert the gibberish to netformat 
    unsigned long ip_hostfmt = ntohl(ip_netfmt); /* this is the ip in host byte order */
    bufd = ip_formatted(ip_hostfmt, SEP, line, sizeof line);

    if(iph->protocol==IPPROTO_ICMP) { // if ICMP
        printk(KERN_INFO "SADDR: %d", iph->saddr);
        if(dropicmp == 1){
            return NF_DROP; // drop our ICMP traffic if required
        }
        else{
            return NF_ACCEPT;
        }
    }
    if(iph->protocol==IPPROTO_TCP) { // if TCP
        printk(KERN_INFO "SADDR: %d", iph->saddr);
        if(droptcp == 1){ // This is working 3/12/2018
            return NF_DROP; 
        }
        else{
            return NF_ACCEPT;
        }
    }
	if(iph->sport==80) { // if http
        return NF_DROP; 
	}
	if(ip_hostfmt!=whitelisted){
		printk(KERN_INFO "Bad address detected: %d", ip_hostfmt);
		
	}
// ---------------------------------------------------------------------------
// This section is under testing. If running, just leave this alone or delete. It won't matter
// This is eventually going to be the subnetting 
// Our mask is 32 bits long. 
// We iterate over it 32 times (arrays start at 0)
// For each item in the array i, we & it to z and append to y
    
	for(final = 0; final<32; final++){
		if (ip_hostfmt[final] == '1'){
			num = num + mask_array;
		}
		mask_array = mask_array / 2;
	}

	int output = num & mask;
	
    int val;
    sscanf(bufa, "%d", &val);
    printk(KERN_INFO "VAL: %d MASK: %d", val, masked_sub);
    if (val == masked_sub){
        return NF_DROP;
    }
	
    return NF_ACCEPT; // default to accept
}

// ------------------------------------------------------------------------------------
// This function is very important for formatting IP's into a human readable format, or binary 
// We format into binary so as to perform an & operation on an address and compare with the range we are blocking 
// The address gets &'d with first the subnet range, then the mask 
// This function takes care of converting from gibberish to network length then host length 

char *ip_formatted(long ip, char *sep, char *buff, size_t buffsz){
    size_t n;
    char *s = buff;
    int i;
    for (i = 32 - NBITS; i >= 0; i -= NBITS) {
        int digit = (ip >> i) & MASK;
        n = snprintf(s, buffsz, "%s%d", i == 32 - NBITS ? "" : sep, digit);
        s += n; 
        buffsz -= n;
    }
    return buff;
}

// initialize with insmod 
static int __init initialize(void) {
    nfho.hook = hook_func;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho);
    return 0;
}

// rmmod 
static void __exit teardown(void) {
    nf_unregister_hook(&nfho);
}

module_init(initialize);
module_exit(teardown);
