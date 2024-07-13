# name of your application
APPLICATION = buzzer_system

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT_AIT/

FEATURES_REQUIRED += picolibc # newlib

#CFLAGS+="-DCONFIG_IEEE802154_DEFAULT_CHANNEL=24"
#CFLAGS+="-DCONFIG_GNRC_IPV6_NIB_SLAAC=1"

# Uncomment these lines if you want to use platform support from external
# repositories:
#RIOTCPU ?= $(CURDIR)/../../RIOT/thirdparty_cpu
#EXTERNAL_BOARD_DIRS ?= $(CURDIR)/../../RIOT/thirdparty_boards

# Uncomment this to enable scheduler statistics for ps:
#USEMODULE += schedstatistics

# If you want to use native with valgrind, you should recompile native
# with the target all-valgrind instead of all:
# make -B clean all-valgrind

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1
# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += netdev_default

ifeq (,$(filter 1, $(LWIP_IPV4) $(LWIP_IPV6)))
  USEMODULE += auto_init_gnrc_netif
  # Specify the mandatory networking modules
  USEMODULE += gnrc_ipv6_default
  # Additional networking modules that can be dropped if not needed
  USEMODULE += gnrc_icmpv6_echo
else
  USEMODULE += lwip_netdev

  ifeq (1,$(LWIP_IPV4))
    USEMODULE += ipv4_addr

    USEMODULE += lwip_arp
    USEMODULE += lwip_ipv4
    USEMODULE += lwip_dhcp_auto
    CFLAGS += -DETHARP_SUPPORT_STATIC_ENTRIES=1
  endif

  ifeq (1,$(LWIP_IPV6))
    USEMODULE += ipv6_addr

    USEMODULE += lwip_ipv6
    USEMODULE += lwip_ipv6_autoconfig
  endif
endif

# Modules to include:
USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps
USEMODULE += gcoap
USEMODULE += cord_ep
# Run the simple CoRE resource directory
USEMODULE += cord_epsim

# include and auto-initialize all available sensors
USEMODULE += saul_default
USEMODULE += shell_cmds_default
USEMODULE += uri_parser
USEMODULE += od
USEMODULE += xtimer
USEMODULE += timex
USEMODULE += sntp

USEMODULE += periph_gpio_irq

include $(RIOTBASE)/tests/Makefile.boards.netif
ifneq (,$(filter $(BOARD),$(BOARD_PROVIDES_NETIF)))
  # Use modules for networking
  # gnrc is a meta module including all required, basic gnrc networking modules
  USEMODULE += gnrc
  # use the default network interface for the board
  USEMODULE += netdev_default
  # automatically initialize the network interface
  USEMODULE += auto_init_gnrc_netif
  # shell command to send L2 packets with a simple string
  USEMODULE += gnrc_txtsnd
  # the application dumps received packets to stdout
  USEMODULE += gnrc_pktdump
endif



FEATURES_OPTIONAL += periph_rtc

ifneq (,$(filter msba2,$(BOARD)))
  USEMODULE += mci
  USEMODULE += random
endif

include $(RIOTBASE)/Makefile.include

ifneq (,$(filter $(BOARD),$(BOARD_PROVIDES_NETIF)))
  # We use only the lower layers of the GNRC network stack, hence, we can
  # reduce the size of the packet buffer a bit
  # Set GNRC_PKTBUF_SIZE via CFLAGS if not being set via Kconfig.
  ifndef CONFIG_GNRC_PKTBUF_SIZE
    CFLAGS += -DCONFIG_GNRC_PKTBUF_SIZE=512
  endif
endif

DEFAULT_CHANNEL ?= 24

# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk

CFLAGS +="-DCONFIG_IEEE802154_DEFAULT_CHANNEL=24"

RD_ADDR ?= \"[fe80::3867:9b25:ae2:ea8b]\" 
# RD_ADDR ?= \"[ff02:fe]\" 
CFLAGS += -DRD_ADDR=$(RD_ADDR)

ifeq ($(BOARD),pba-d-01-kw2x)
  CFLAGS += "-DCONFIG_CORD_EP=\"RIOT-RASPI-4-NODE\""
else ifeq ($(BOARD),feather-nrf52840-sense)
  CFLAGS += "-DCONFIG_CORD_EP=\"RIOT-RASPI-4-NODE-FEATHER\""
endif

CFLAGS += -DCONFIG_GNRC_IPV6_NIB_SLAAC=1
CFLAGS += -DCONFIG_GNRC_IPV6_NIB_6LBR=1
CFLAGS += -DREUSE_TAP=1
CFLAGS += -DBOARD_FEATHER=1
CFLAGS += -03
