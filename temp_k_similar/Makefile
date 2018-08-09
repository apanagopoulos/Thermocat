# a unique name for the application
APPLICATION = predict_temperature

# relative path to RIOT-OS folder
RIOTBASE ?= $(CURDIR)/../../

# if BOARD is native the binary will be executable natively (e.g in Linux).
# You can override BOARD as parameterto cmd line, e.g:
#   $ make BOARD=hamilton
BOARD ?= native

# Use the xtimer module to count time, so that we run the prediction every 15 minutes
USEMODULE += xtimer
# Not sure what rtt_stdio does. Our program seems to work without it
# USEMODULE += rtt_stdio

# Comment this out to disable code in RIOT that does safety checking which is not needed in a
# production environment but helps in the development process:
CFLAGS += -DDEVELHELP

# Not sure if we need to include anything from cpu lib,
# which btw seems to change drastically at last RIOT-OS versions
# CFLAGS += -I$(RIOTBASE)/cpu/sam0_common/include

# QUIET ?= 1

FEATURES_REQUIRED += periph_timer

include $(RIOTBASE)/Makefile.include
