include ./config.mk

# ifeq ($(opt),t113)
# SUBDIRS = librtdb share Protocol/SocketFtp  procman rdbms BayLayer Protocol/ModBus Protocol/IEC104 Protocol/UPS Protocol/IEC103 Protocol/LFP Protocol/DDB Protocol/Dlt645 Protocol/ModBusMaster Protocol/IEC101S Protocol/ModBusTcp Protocol/ModBusSlave Protocol/DataTransS Protocol/ESDCMMI cgi Protocol/CDT Protocol/ModBusTcp-Transfer Protocol/ModBusTcp_Gather Protocol/SPA Protocol/IEC61850
# endif

SUBDIRS = librtdb share Protocol/SocketFtp procman rdbms BayLayer Protocol/ModBusMaster cgi Protocol/ModBus

all:
	@echo "Makefile absolute directory: $(SRC_ROOT_DIR)"
	 @list='$(SUBDIRS)'; for subdir in $$list; do \
                echo "Making all in $$list"; \
                (cd $$subdir && make all -j4 opt=$(opt) ver=r ); \
		if [ $$? -ne 0 ]; then echo stoped all complie ; break; \
                 fi; \
		echo -e "\n" ; \
        done;

.PHONY: clean
clean:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
                echo "Clean in $$subdir"; \
                (cd $$subdir && make clean -j4 opt=$(opt) ver=r ); \
				if [ $$? -ne 0 ]; then echo stoped clean complie ; break; \
                fi; \
				echo -e "\n" ; \
        done
distclean:
	echo $(SUBDIRS)
	echo distclean
	@list='$(SUBDIRS)'; for subdir in $$list; do \
                echo "Clean in $$subdir"; \
                (cd $$subdir && make distclean -j4 opt=$(opt) ver=r ); \
				if [ $$? -ne 0 ]; then echo stoped distclean complie ; break; \
                 fi; \
				echo -e "\n" ; \
        done
