// This library has been derived from the GGZ project, an update might exist!
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef GGZ_GAMESERVER_H
#define GGZ_GAMESERVER_H

#include "config.h"

#ifdef WITH_GGZ

// GGZdMod includes
#include <ggzdmod.h>

// General virtual game server class (similar to Zone)
class GGZGameServer
{
	public:
		GGZGameServer();
		virtual ~GGZGameServer();
		void connect(bool loop, bool async);
		void process();

	protected:
		virtual void idleEvent();
		virtual void stateEvent();
		virtual void joinEvent(int player);
		virtual void leaveEvent(int player);
		virtual void dataEvent(int player);
		virtual void spectatorJoinEvent(int spectator);
		virtual void spectatorLeaveEvent(int spectator);
		virtual void spectatorDataEvent(int spectator);
		virtual void errorEvent();
		int fd(int player);
		int players();
		int spectatorfd(int spectator);
		int spectators();
		int open();

		void proceedToPlaying();

	private:
		static void handle_state(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		static void handle_join(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		static void handle_leave(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		static void handle_data(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		static void handle_error(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		static void handle_spectator_join(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		static void handle_spectator_leave(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		static void handle_spectator_data(GGZdMod* ggzdmod, GGZdModEvent event, const void *data);
		int m_connected;
};

#else

// Compiled without GGZ support
class GGZGameServer
{
	public:
		GGZGameServer();
		void connect(bool loop, bool async);
		void process();
		int fd(int player);
};

#endif

#endif

