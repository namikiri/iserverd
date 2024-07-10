
prefix	 = $(DESTDIR)/usr/local
SERVROOT = $(DESTDIR)/usr/local
BIN_DIR  = $(DESTDIR)/usr/bin
ETC_DIR  = $(DESTDIR)/etc/iserverd
SHELL = /bin/sh

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
transform = s,x,x,

mkinstalldirs = $(SHELL) ../mkinstalldirs

BIN_SCRIPT  =  iserver_db_manage.sh \
	       iserver_convert_db.sh \
	       iserver_post_mess.cgi

DB_SCRIPT   =  iserver_icquser
DB_SUPP     =  icquser.sql

ALL_SCRIPT  =  iserverd.sh \
	       iserverd.sh.asp 

all: 
	@echo "Nothing to do in scripts"

clean:
	@echo "Cleaning in scripts"

distclean: 
	@list='$(ALL_SCRIPT)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "Deleting script $$p"; \
	    rm -rf $$p ; \
	  else :; fi ; \
	done
	@list='$(BIN_SCRIPT)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "Deleting script $$p"; \
	    rm -rf $$p ; \
	  else :; fi ; \
	done
	@list='$(DB_SCRIPT)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "Deleting script $$p"; \
	    rm -rf $$p ; \
	  else :; fi ; \
	done
	@echo "Deleting Makefile"; rm -rf Makefile

install: 
	@list='$(ALL_SCRIPT)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_SCRIPT) $$p $(ETC_DIR)/`echo $$p|sed '$(transform)'`"; \
	     $(INSTALL_SCRIPT) $$p $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done
	$(mkinstalldirs) $(BIN_DIR);
	@list='$(BIN_SCRIPT)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_SCRIPT) $$p $(BIN_DIR)/`echo $$p|sed '$(transform)'`"; \
	     $(INSTALL_SCRIPT) $$p $(BIN_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done
	$(mkinstalldirs) $(ETC_DIR)/db;
	@list='$(DB_SCRIPT)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_SCRIPT) $$p $(ETC_DIR)/db/`echo $$p|sed '$(transform)'`"; \
	     $(INSTALL_SCRIPT) $$p $(ETC_DIR)/db/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done
	@list='$(DB_SUPP)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_DATA) $$p $(ETC_DIR)/db/`echo $$p|sed '$(transform)'`"; \
	     $(INSTALL_DATA) $$p $(ETC_DIR)/db/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

uninstall:
	@list='$(ALL_SCRIPT)'; for p in $$list; do \
	  rm -f $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  echo "  rm -f $(ETC_DIR)/$$p"; \
	done;
	@list='$(BIN_SCRIPT)'; for p in $$list; do \
	  rm -f $(BIN_DIR)/`echo $$p|sed '$(transform)'`; \
	  echo "  rm -f $(BIN_DIR)/$$p"; \
	done;
	@list='$(DB_SUPP)'; for p in $$list; do \
	  rm -f $(ETC_DIR)/db/`echo $$p|sed '$(transform)'`; \
	  echo "  rm -f $(ETC_DIR)/db/$$p"; \
	done;
	@list='$(DB_SCRIPT)'; for p in $$list; do \
	  rm -f $(ETC_DIR)/db/`echo $$p|sed '$(transform)'`; \
	  echo "  rm -f $(ETC_DIR)/db/$$p"; \
	done;
	 rm -df $(ETC_DIR)/db;
	 rm -df $(ETC_DIR);

