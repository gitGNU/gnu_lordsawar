if GGZSUPPORT
gamedir = $(ggzdconfdir)/games
game_DATA = lordsawar-server.dsc

roomdir = $(ggzdconfdir)/rooms
room_DATA = lordsawar-server.room

install-data-local:
	$(GGZ_CONFIG) -D --force --install --modfile=lordsawar.dsc

uninstall:
	$(GGZ_CONFIG) --remove --modfile=lordsawar.dsc
endif 
EXTRA_DIST = README.GGZ lordsawar-server.room

