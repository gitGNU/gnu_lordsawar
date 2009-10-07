// Copyright (C) 2008, 2009 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "Configuration.h"
#include "File.h"

#include "pbm.h"
#include "vector.h"

using namespace std;

int max_vector_width;
int main(int argc, char* argv[])
{
  srand(time(NULL));         // set the random seed

  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);

  setlocale(LC_ALL, Configuration::s_lang.c_str());

  if (argc != 3)
    {
      fprintf (stderr, "Usage: %s saved-game-file turn-file\n", argv[0]);
      exit (1);
    }

  pbm playbymail;
  playbymail.run(argv[1], argv[2]);
  printf ("Now send the saved-game file to %s.\n", playbymail.getActiveplayerName().c_str());

  return EXIT_SUCCESS;
}
