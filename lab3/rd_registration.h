#ifndef RD_REGISTRATION_H
#define RD_REGISTRATION_H

#include "net/gcoap.h"
#include "net/cord/common.h"
#include "net/cord/epsim.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/netif.h"
#include "net/sock/util.h"
#include "xtimer.h"


void * register_at_resource_directory(void *args);

#endif /* RD_REGISTRATION_H */

