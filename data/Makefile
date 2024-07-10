
prefix   = $(DESTDIR)/usr/local
SERVROOT = $(DESTDIR)/usr/local

ETC_DIR  = $(DESTDIR)/etc/iserverd
SHELL    = /bin/sh
PETC     = etc/

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SECURE = ${INSTALL} -m 600
INSTALL_SCRIPT = ${INSTALL}
transform = s,x,x,

mkinstalldirs = $(SHELL) ../mkinstalldirs

ALL_TRANSLAT  = translate/ASCII \
	        translate/CP437 \
	        translate/CP850 \
	        translate/DANISH \
	        translate/DEC_MCS \
	        translate/DG_MCS \
	        translate/DUTCH \
	        translate/FINNISH \
	        translate/FRENCH \
	        translate/FRENCH_CANADIAN \
	        translate/GERMAN \
	        translate/HP_MCS \
	        translate/IRV \
	        translate/ITALIAN \
	        translate/JIS \
	        translate/MACINTOSH \
	        translate/NEXT \
	        translate/NORWEGIAN_1 \
	        translate/NORWEGIAN_2 \
	        translate/POLISH \
	        translate/POLISH_NOPL \
	        translate/PORTUGUESE \
	        translate/PORTUGUESE_COM \
	        translate/RUSSIAN \
	        translate/RUSSIAN_ALT \
	        translate/RUSSIAN_WIN \
	        translate/SPANISH \
	        translate/SWEDISH \
	        translate/SWEDISH_NAMES \
	        translate/SWISS \
	        translate/UKRAINIAN_WIN

ALL_ETC	      = texts/adm_notes.rus \
	        texts/adm_notes.txt \
	        texts/post_reg_auto.rus \
	        texts/post_reg_auto.txt \
	        texts/post_reg_man.rus \
	        texts/post_reg_man.txt \
		texts/reg_mess.ptt \
	        $(ALL_TRANSLATE)

ALL_CONF      = iserv.conf.default \
		v3_proto.conf.default \
		v5_proto.conf.default \
		v7_proto.conf.default \
		aim_proto.conf \
		actions.conf.default

all: 
	@echo "Nothing to make in translate."

clean:
	@echo "Nothing to make in translate."

distclean:
	rm -rf etc/*.conf.default
	rm -rf Makefile

install: install-etc install-translate 

install-etc:
	$(mkinstalldirs) $(ETC_DIR)/texts
	@list='$(ALL_ETC)'; for p in $$list; do \
	  if test -f $(PETC)/$$p; then \
	     $(INSTALL_DATA) $(PETC)/$$p $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done
	@list='$(ALL_CONF)'; for p in $$list; do \
	  if test -f $(PETC)/$$p; then \
	     $(INSTALL_SECURE) $(PETC)/$$p $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

install-translate:
	$(mkinstalldirs) $(ETC_DIR)/translate
	@list='$(ALL_TRANSLAT)'; for p in $$list; do \
	  if test -f $(PETC)/$$p; then \
	     $(INSTALL_DATA) $(PETC)/$$p $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

uninstall:
	@list='$(ALL_ETC)'; for p in $$list; do \
	  rm -f $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  echo " rm -f $(ETC_DIR)/`echo $$p|sed '$(transform)'`"; \
	done
	@list='$(ALL_CONF)'; for p in $$list; do \
	  rm -f $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  echo " rm -f $(ETC_DIR)/`echo $$p|sed '$(transform)'`"; \
	done
	@list='$(ALL_TRANSLAT)'; for p in $$list; do \
	  rm -f $(ETC_DIR)/`echo $$p|sed '$(transform)'`; \
	  echo "  rm -f $(ETC_DIR)/`echo $$p|sed '$(transform)'`"; \
	done
	  rm -df $(ETC_DIR)/texts;
	  rm -df $(ETC_DIR)/translate;
