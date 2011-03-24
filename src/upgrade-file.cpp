// Copyright (C) 2011 Ben Asselstine
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
#include "Configuration.h"
#include "xmlhelper.h"
#include "tarhelper.h"
#include "File.h"
#include "profilelist.h"
#include "recently-played-game-list.h"
#include "gamelist.h"
#include "ucompose.hpp"
#include "file-compat.h"

using namespace std;
int max_vector_width;
int main(int argc, char* argv[])
{
  std::string filename;
  std::string rewrite;
  bool identify_file = false;
  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);

  #if ENABLE_NLS
  setlocale(LC_ALL, Configuration::s_lang.c_str());
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
  #endif

  if (argc > 1)
    {
      for (int i = 2; i <= argc; i++)
	{
          string parameter(argv[i-1]); 
	  if (parameter == "--identify" || parameter == "-i")
            {
              identify_file = true;
            }
          else if (parameter == "--rewrite" || parameter == "-r")
            {
              rewrite = parameter;
            }
	  else if (parameter == "--help" || parameter == "-?")
	    {
	      cout << File::get_basename(argv[0], true) << " [OPTION]... FILE" << endl << 
                endl;
	      cout << "LordsAWar! File Upgrading Tool " << _("version") << 
                " " << VERSION << endl << endl;
	      cout << _("Options:") << endl << endl; 
	      cout << "  -?, --help                 " << _("Display this help and exit") <<endl;
	      cout << "  -i, --identify             " << _("Show the file type instead of upgrading") << endl;
	      //cout << "  -r, --rewrite VERSION      " << _("Just change the version instead of upgrading") << endl;
	      cout << endl;
	      cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << endl;
	      exit(0);
	    }
	  else
	    filename = parameter;
	}
    }

  bool same_version = false;
  Profilelist::support_backward_compatibility();
  RecentlyPlayedGameList::support_backward_compatibility();
  Gamelist::support_backward_compatibility();
  FileCompat::support_backward_compatibility_for_common_files();
  if (identify_file == false && rewrite == "")
    {
      std::string tmpfile = File::get_tmp_file();
      File::copy(filename, tmpfile);
      bool upgraded = FileCompat::getInstance()->upgrade(tmpfile, 
                                                         same_version);

      if (same_version)
        {
          cerr << String::ucompose(_("%1 is already the latest version."), 
                                   filename) << endl;
          File::erase(tmpfile);
        }
      else if (!upgraded && !same_version)
        {
          cerr << String::ucompose(_("Error: %1 could not be upgraded."), 
                                   filename) << endl;
          File::erase(tmpfile);
        }
      else
        {
          File::copy(tmpfile, filename);
          File::erase(tmpfile);
        }
      return !upgraded;
    }
  else if (identify_file && rewrite == "")
    {
      std::string tag, version;
      FileCompat::Type type = FileCompat::getInstance()->getType(filename);
      FileCompat::getInstance()->get_tag_and_version_from_file(filename, type, tag, version);
      cout << String::ucompose("%1 (%2 %3)", FileCompat::typeToString(type), 
                               tag, version) << endl;
      return EXIT_SUCCESS;
    }
  else if (identify_file == false && rewrite != "")
    {
      FileCompat *fc = FileCompat::getInstance();
      std::string tag, version;
      FileCompat::Type type = fc->getType(filename);
      if (fc->get_tag_and_version_from_file(filename, type, tag, version))
        {
          if (fc->rewrite_with_updated_version(filename, type, tag, rewrite))
            return EXIT_SUCCESS;
          else
            return EXIT_FAILURE;
        }
      else
        return EXIT_FAILURE;
    }
  else if (identify_file && rewrite != "")
    {
      cerr << _("Error: The --identify and --rewrite options cannot be used at the same time.") << endl;
      return EXIT_FAILURE;
    }

}

