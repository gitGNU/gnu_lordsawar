#include "Server.h"

#include <iostream>

#ifdef __WIN32__
#include "../win32.h"
#endif

int main(int argc, char **argv)
{
	Server *server;

	std::cout << "LordsAWar Server: started" << std::endl;

	server = new Server();
	server->connect(true, false);

	std::cout << "LordsAWar Server: stopped" << std::endl;

	return 0;
}

