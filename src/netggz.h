// Copyright (C) 2005, 2006 Josef Spillner
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

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

