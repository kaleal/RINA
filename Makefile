#
# OMNeT++/OMNEST Makefile for rina
#
# This file was generated with the command:
#  opp_makemake -f --deep --nolink -O out -d policies -d src -X. -L./out/$$\(CONFIGNAME\)/policies -L./out/$$\(CONFIGNAME\)/src
#

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target

all:  
	cd src && $(MAKE) all
	cd policies && $(MAKE) all

submakedirs:  src_dir policies_dir

.PHONY: all clean cleanall depend msgheaders src policies
policies: policies_dir
src: src_dir

policies_dir:
	cd policies && $(MAKE) all

src_dir:
	cd src && $(MAKE) all

msgheaders:
	$(Q)cd policies && $(MAKE) msgheaders
	$(Q)cd src && $(MAKE) msgheaders

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f rina rinasim.exe librinasim.so librinasimcore.a librinasim.dll librinasim.dylib

	-$(Q)cd src && $(MAKE) clean
	-$(Q)cd policies && $(MAKE) clean	

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)-cd src && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd policies && if [ -f Makefile ]; then $(MAKE) depend; fi
	

