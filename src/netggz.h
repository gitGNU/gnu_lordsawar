#ifndef NET_GGZ_H
#define NET_GGZ_H

#include "config.h"

#ifdef WITH_GGZ
#include <ggzmod.h>
#include <ggzcore.h>
#endif

class GGZ {
    public:
		GGZ();
		static GGZ* ref();

		void init();
		bool connect();
		void deinit();

		bool used();
		bool host();
		bool data();

		int fd();
		int controlfd();
		void dispatch();

		int seats();
		const char *name(int seat);

		bool playing();

    private:
#ifdef WITH_GGZ
		static void ggzmod_server(GGZMod *mod, GGZModEvent e, const void *data);
#endif

		bool use_ggz;
		int mod_fd;
		int control_fd;
};

#endif

