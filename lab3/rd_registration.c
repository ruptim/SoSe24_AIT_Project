
#include "rd_registration.h"

#include "net/gnrc.h"
#include "net/ipv6.h"
#include "net/gnrc/ipv6/nib.h"


#define BUFSIZE             (64U)
#define STARTUP_DELAY       (3U)    /* wait 3s before sending first request*/


#define ENABLE_DEBUG 1

static char riot_info[BUFSIZE];



void *register_at_resource_directory(void *arg)
{
    (void) arg;

    gnrc_ipv6_nib_init();

    gnrc_ipv6_nib_abr_t entry;
    void *state = NULL;
    (void) state;
    // uint32_t border_router = 0;

    while (gnrc_ipv6_nib_abr_iter(&state, &entry)) {
        // gnrc_ipv6_nib_abr_print(&entry);
        // printf("Entry %ld \n",	);
        // border_router = byteorder_ntohl(entry.addr.u32[0]);
        break;
    }

    char rd_address[IPV6_ADDR_MAX_STR_LEN+3] = "";
    char addr_str[IPV6_ADDR_MAX_STR_LEN];

    ipv6_addr_to_str(addr_str, &entry.addr, sizeof(addr_str));
    snprintf(rd_address,IPV6_ADDR_MAX_STR_LEN+3,"[%s]",addr_str);
    

    // gncr_ipv6_nib

    char ep_str[CONFIG_SOCK_URLPATH_MAXLEN];
    uint16_t ep_port;

    /* fill riot info */
    sprintf(riot_info, "{\"ep\":\"%s\",\"lt\":%lu}",
            cord_common_get_ep(), CONFIG_CORD_LT);

    /* parse RD address information */
    sock_udp_ep_t rd_ep;

    
    if (sock_udp_name2ep(&rd_ep, rd_address) < 0) {
        printf("error: unable to parse RD address from %s \n",rd_address);
        return (void*) 1; 
    }

    /* if netif is not specified in addr and it's link local */
    if ((rd_ep.netif == SOCK_ADDR_ANY_NETIF) &&
         ipv6_addr_is_link_local((ipv6_addr_t *) &rd_ep.addr.ipv6)) {
        /* if there is only one interface we use that */
        if (gnrc_netif_numof() == 1) {
            rd_ep.netif = (uint16_t)gnrc_netif_iter(NULL)->pid;
        }
        /* if there are many it's an error */
        else if ((rd_ep.netif == SOCK_ADDR_ANY_NETIF) &&
            ipv6_addr_is_multicast((ipv6_addr_t *) &rd_ep.addr.ipv6)){
            /// todo?
        }
        else {
            puts("error: must specify an interface for a link local address");
            return (void*) 1; 
        }
    }

    if (rd_ep.port == 0) {
        rd_ep.port = COAP_PORT;
    }

    sock_udp_ep_fmt(&rd_ep, ep_str, &ep_port);


    /* print RD client information */
    puts("epsim configuration:");
    printf("         ep: %s\n", cord_common_get_ep());
    printf("         lt: %is\n", (int)CONFIG_CORD_LT);
    printf(" RD address: [%s]:%u\n\n", ep_str, ep_port);

    xtimer_sleep(STARTUP_DELAY);

    while (1) {
        int res = cord_epsim_state();
        switch (res) {
            case CORD_EPSIM_OK:
                puts("state: registration active");
                break;
            case CORD_EPSIM_BUSY:
                puts("state: registration in progress");
                break;
            case CORD_EPSIM_ERROR:
            default:
                puts("state: not registered");
                break;
        }

        printf("updating registration with RD [%s]:%u\n", ep_str, ep_port);
        res = cord_epsim_register(&rd_ep);
        if (res == CORD_EPSIM_BUSY) {
            puts("warning: registration already in progress");
        }
        else if (res == CORD_EPSIM_ERROR) {
            puts("error: unable to trigger simple registration process");
        }
        xtimer_sleep(CONFIG_CORD_UPDATE_INTERVAL);
    }

    puts("Resource registered! Exiting!");

}