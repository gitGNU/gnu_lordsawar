#include "netggz.h"

#ifdef WITH_GGZ
#include <ggz.h>
#endif
#include <stdio.h>
#include <unistd.h>

static GGZ *ggzobj = 0;
#ifdef WITH_GGZ
static GGZMod *mod = 0;
#endif

GGZ::GGZ()
{
	use_ggz = false;
	mod_fd = -1;
	control_fd = -1;
}

GGZ* GGZ::ref()
{
	if(!ggzobj) ggzobj = new GGZ();
	return ggzobj;
}

void GGZ::init()
{
	use_ggz = true;
	printf(">> GGZ: initialized\n");
}

void GGZ::deinit()
{
#ifdef WITH_GGZ
	if(mod_fd != -1)
	{
		close(mod_fd);
		mod_fd = -1;
	}
	if(mod)
	{
		ggzmod_disconnect(mod);
		ggzmod_free(mod);
		mod = 0;
	}
#endif

	use_ggz = false;
	printf(">> GGZ: de-initialized\n");
}

bool GGZ::used()
{
	return use_ggz;
}

bool GGZ::connect()
{
#ifdef WITH_GGZ
	int ret;

	if(!used()) return false;
	if(mod) return true;

	printf("GGZ ## connect\n");
	mod = ggzmod_new(GGZMOD_GAME);
	ggzmod_set_handler(mod, GGZMOD_EVENT_SERVER, &GGZ::ggzmod_server);
	ret = ggzmod_connect(mod);
	if(ret)
	{
		printf("GGZ ## connection failed\n");
		return false;
	}

	control_fd = ggzmod_get_fd(mod);
	printf("GGZ ## connection fd %i\n", control_fd);
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(control_fd, &fdset);
	/*while(ggzmod_get_state(mod) != GGZMOD_STATE_WAITING)*/
	while(seats() == 0)
	{
		ret = select(control_fd + 1, &fdset, NULL, NULL, &timeout);
		if(ret == -1)
		{
			printf("GGZ ## select error\n");
			return false;
		}
		ggzmod_dispatch(mod);
	}

	printf("GGZ ## dispatch hack\n");
	//for(int i = 0; i < 100; i++)
//	while(seats() != 2)
//	{
//		ggzmod_dispatch(mod);
//	}

	return true;
#else
	return false;
#endif
}

void GGZ::dispatch()
{
#ifdef WITH_GGZ
	ggzmod_dispatch(mod);
#endif
}

#ifdef WITH_GGZ
void GGZ::ggzmod_server(GGZMod *mod, GGZModEvent e, const void *data)
{
	printf("GGZ ## ggzmod_server\n");
	int fd = *(int*)data;
	ggzobj->mod_fd = fd;
	printf("GGZ ## got fd: %i\n", fd);
//	ggzmod_set_state(mod, GGZMOD_STATE_PLAYING);
}
#endif

bool GGZ::data()
{
#ifdef WITH_GGZ
	if(!used()) return false;
	if(mod_fd == -1) return false;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(mod_fd, &fdset);
	int ret = select(mod_fd + 1, &fdset, NULL, NULL, &timeout);
	if(ret <= 0)
	{
		if(ret < 0)
		{
			printf("GGZ ## select() returns: %i for fd %i\n", ret, mod_fd);
			deinit();
		}
		return false;
	}

	return true;
#else
	return false;
#endif
}

bool GGZ::host()
{
#ifdef WITH_GGZ
	int spectator, seat;

	if(!used()) return false;
	if(!mod) return false;

	do
	{
		ggzmod_dispatch(mod);
		ggzmod_get_player(mod, &spectator, &seat);
	}
	while(seat == -1);

	printf("GGZ ## host? seat=%i\n", seat);
	if(!seat) return true;
	return false;
#else
	return false;
#endif
}

int GGZ::fd()
{
	return mod_fd;
}

int GGZ::controlfd()
{
	return control_fd;
}

int GGZ::seats()
{
#ifdef WITH_GGZ
	printf("GGZ ## seats? used=%i mod=%p state=%i\n", used(), mod, ggzmod_get_state(mod));
	if(!used()) return 0;
	if(!mod) return 0;

	printf("GGZ ## seats? return %i\n", ggzmod_get_num_seats(mod));
	return ggzmod_get_num_seats(mod);
#else
	return 0;
#endif
}

const char *GGZ::name(int seat)
{
#ifdef WITH_GGZ
	GGZSeat ggzseat = ggzmod_get_seat(mod, seat);
	const char *n = ggzseat.name;
	if(!n)
	{
		n = "(---)";
		if(ggzseat.type == GGZ_SEAT_OPEN) n = "(open)";
		if(ggzseat.type == GGZ_SEAT_BOT) n = "(AI player)";
	}
	return n;
#else
	return NULL;
#endif
}

bool GGZ::playing()
{
#ifdef WITH_GGZ
	if(!used()) return false;
	if(!mod) return false;

	if(ggzmod_get_state(mod) == GGZMOD_STATE_PLAYING) return true;
	return false;
#else
	return false;
#endif
}

