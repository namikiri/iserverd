
prefix  = $(DESTDIR)/usr/local
MAN_DIR	= $(DESTDIR)/usr/share/man
MAN1_DIR = $(MAN_DIR)/man1
MAN5_DIR = $(MAN_DIR)/man5
MAN8_DIR = $(MAN_DIR)/man8
SHELL 	= /bin/sh

INSTALL 	= /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA 	= ${INSTALL} -m 644
INSTALL_SCRIPT 	= ${INSTALL}
transform 	= s,x,x,

mkinstalldirs = $(SHELL) ../mkinstalldirs

ALL_MAN1   = db_convert.1
ALL_MAN5   = iserv.conf.5 \
	    v3_proto.conf.5 \
	    v5_proto.conf.5
ALL_MAN8   = iserverd.8

all: 
	@echo "Nothing to make in man."

clean:
	@echo "Nothing to clean in man."

distclean:
	rm -rf Makefile

install: install-man 

install-man:
	$(mkinstalldirs) $(MAN1_DIR)
	@list='$(ALL_MAN1)'; for p in $$list; do \
	  if test -f $$p; then \
	     $(INSTALL_DATA) $$p $(MAN1_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done
	$(mkinstalldirs) $(MAN5_DIR)
	@list='$(ALL_MAN5)'; for p in $$list; do \
	  if test -f $$p; then \
	     $(INSTALL_DATA) $$p $(MAN5_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done
	$(mkinstalldirs) $(MAN8_DIR)
	@list='$(ALL_MAN8)'; for p in $$list; do \
	  if test -f $$p; then \
	     $(INSTALL_DATA) $$p $(MAN8_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

uninstall:
	@list='$(ALL_MAN)'; for p in $$list; do \
	  rm -f $(MAN_DIR)/`echo $$p|sed '$(transform)'`; \
	  echo " rm -f $(MAN_DIR)/`echo $$p|sed '$(transform)'`"; \
	done
	rm -df $(MAN_DIR);
