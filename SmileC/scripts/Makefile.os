
#------------------------------------------------------------------------
# This Makefile include contains simple OS-detection rules.
#
# This script is expected to assign these names meaningfully:
#
#   OS_NAME ("Linux")
#   OS_NAME_CLEAN ("linux")
#   PROCESSOR_NAME ("x86_64")
#   PROCESSOR_NAME_CLEAN ("x86-64")
#   PLATFORM_NAME ("linux-x86-64")
#
# In addition, it is expected to set these flags:
#
#   OS_LINUX ("true" or blank)
#   OS_CYGWIN ("true" or blank)

#------------------------------------------------------------------------
# In general, you should not need to change this.

MAKEFILE_OS_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

# The name of your OS, like "Linux".
OS_NAME := $(shell $(MAKEFILE_OS_DIR)detect-os.sh)
OS_NAME_CLEAN := $(shell echo $(OS_NAME) \
	| $(SED) -e 's/[^a-zA-Z0-9]\+/-/g; y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/;')

# The processor/CPU you're running on, like "x64".
PROCESSOR_NAME := $(shell $(MAKEFILE_OS_DIR)detect-proc.sh)
PROCESSOR_NAME_CLEAN := $(shell echo $(PROCESSOR_NAME) \
	| $(SED) -e 's/[^a-zA-Z0-9]\+/-/g; y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/;')

# A combination of the OS_NAME_CLEAN and PLATFORM_NAME_CLEAN, like "linux-x86-64".
PLATFORM_NAME := $(OS_NAME_CLEAN)-$(PROCESSOR_NAME_CLEAN)

# Detect Linux systems.
ifneq '$(findstring linux,$(OS_NAME_CLEAN))' ''
    OS_LINUX := true
else
    OS_LINUX := false
endif

# Detect Cygwin (GNU-on-Windows) systems.
ifneq '$(findstring cygwin,$(OS_NAME_CLEAN))' ''
    OS_CYGWIN := true
else
    OS_CYGWIN := false
endif

