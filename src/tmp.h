#ifndef TMP_HEADER
#define TMP_HEADER

#include <string>

#include "game-parameters.h"

// returns the path of the dumped scenario file
std::string create_and_dump_scenario(const std::string &file,
				     const GameParameters &g);


#endif
