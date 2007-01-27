if GGZSUPPORT
gamedir = $(ggzdconfdir)/games
game_DATA = freelords-server.dsc

roomdir = $(ggzdconfdir)/rooms
room_DATA = freelords-server.room

install-data-local:
	$(GGZ_CONFIG) -D --force --install --modfile=freelords.dsc

uninstall:
	$(GGZ_CONFIG) --remove --modfile=freelords.dsc
endif 
EXTRA_DIST = README.GGZ freelords-server.room

