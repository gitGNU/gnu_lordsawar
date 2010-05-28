// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include "config.h"

#include <iostream>
#include <string.h>
#include <algorithm>
#include <glibmm/fileutils.h>
#include <glibmm/ustring.h>
#include <glibmm/convert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#ifndef __WIN32__
#include <unistd.h>
#endif

#include "File.h"
#include "Configuration.h"
#include "defs.h"
#include "armyset.h"
#include "tileset.h"
#include "shieldset.h"
#include "cityset.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

namespace
{
    std::list<std::string> get_files(std::string path, std::string ext)
    {
	std::list<std::string> retlist;
	Glib::Dir dir(path);
    
	for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
          {
	    std::string entry = *i;
	    std::string::size_type idx = entry.rfind(ext);
	    if (idx != std::string::npos && 
                idx == entry.length() - ext.length())
              retlist.push_back(Glib::filename_to_utf8(path + entry));
          }
	return retlist;
    }
}

std::string File::add_ext_if_necessary(std::string file, std::string ext)
{
  if (nameEndsWith(file, ext) == true)
    return file;
  else
    return file + ext;
}

std::string File::add_slash_if_necessary(std::string dir)
{
  if (dir.c_str()[strlen(dir.c_str())-1] == '/')
    return dir;
  else
    return dir + "/";
}

std::string File::getMiscFile(std::string filename)
{
  return Configuration::s_dataPath + "/" + filename;
}

std::string File::getItemDescription()
{
  return Configuration::s_dataPath + "/various/items/items.xml";
}

std::string File::getEditorFile(std::string filename)
{
  return Configuration::s_dataPath + "/various/editor/" + filename + ".png";
}

std::string File::getMusicFile(std::string filename)
{
  return std::string(Configuration::s_dataPath + "/music/" + filename.c_str());
}

std::string File::getDataPath()
{
  return add_slash_if_necessary(Configuration::s_dataPath);
}

std::string File::getSavePath()
{
  return add_slash_if_necessary(Configuration::s_savePath);
}

std::string File::getUserMapDir()
{
  return add_slash_if_necessary(Configuration::s_savePath) + MAPDIR + "/";
}

std::string File::getMapDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + MAPDIR + "/";
}

std::string File::getUserMapFile(std::string file)
{
  return getUserMapDir() + file;
}

std::string File::getMapFile(std::string file)
{
  return getMapDir() + file;
}
std::list<std::string> File::scanUserMaps()
{
  std::string path = File::getUserMapDir();
    
    std::list<std::string> retlist;
    Glib::Dir dir(path);
    
    for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      std::string entry = *i;
      std::string::size_type idx = entry.find(".map");
      if (idx != std::string::npos)
	{
	  if (entry == "random.map")
	    continue;
	  retlist.push_back(Glib::filename_to_utf8(entry));
	}
    }
    
    return retlist;
}

std::list<std::string> File::scanMaps()
{
  std::string path = File::getMapDir();
    
    std::list<std::string> retlist;
    Glib::Dir dir(path);
    
    for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      std::string entry = *i;
      std::string::size_type idx = entry.find(".map");
      if (idx != std::string::npos)
	{
	    retlist.push_back(Glib::filename_to_utf8(entry));
	}
    }
    
    if (retlist.empty())
    {
      std::cerr << "Couldn't find a single map!" << std::endl;
      std::cerr << "Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc" << std::endl;
    }

    return retlist;
}

std::string File::get_dirname(std::string path)
{
  return Glib::path_get_dirname(path);
}
std::string File::get_basename(std::string path, bool keep_ext)
{
  std::string file;
  file = Glib::path_get_basename(path);
  if (keep_ext)
    return file;
  //now strip everything past the last dot.
  const char *tmp = strrchr (file.c_str(), '.');
  if (!tmp)
    return file;
  int npos = tmp - file.c_str() + 1;
  file = file.substr(0, npos - 1);
  return file;
}
//copy_file taken from ardour-2.0rc2, gplv2+.
int File::copy (Glib::ustring from, Glib::ustring to)
{
  std::ifstream in (from.c_str());
  std::ofstream out (to.c_str());

  if (!in)
    return -1;

  if (!out)
    return -1;

  out << in.rdbuf();

  if (!in || !out) 
    {
      unlink (to.c_str());
      return -1;
    }

  return 0;
}
bool File::create_dir(std::string dir)
{
    struct stat testdir;
#ifndef __WIN32__
    if (stat(dir.c_str(), &testdir) || !S_ISDIR(testdir.st_mode))
    {
        guint32 mask = 0755; //make directory only readable for user and group
        if (mkdir(dir.c_str(), mask))
	  return false;
    }
#else
    return false;
#endif
    return true;
}
	
bool File::is_writable(std::string file)
{
#ifndef __WIN32__
  if (access (file.c_str(), W_OK) != 0)
    return false;
  else
    return true;
#endif
  return false;
}

bool File::exists(std::string f)
{
  FILE *fileptr = fopen (f.c_str(), "r");
  bool retval = fileptr != NULL;
  if (fileptr)
    fclose(fileptr);
  return retval;
}

//armysets 

std::string File::getArmysetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + ARMYSETDIR + "/";
}

std::string File::getUserArmysetDir()
{
  std::string dir =  getSavePath() + ARMYSETDIR + "/";
  return dir;
}

std::list<std::string> File::scanForFiles(std::string dir, std::string extension)
{
  return get_files (dir, extension);
}


std::string File::getTilesetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + TILESETDIR + "/";
}

std::string File::getUserTilesetDir()
{
  std::string dir = getSavePath() + TILESETDIR + "/";
  return dir;
}

//shieldsets
std::string File::getShieldsetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + SHIELDSETDIR + "/";
}

std::string File::getUserShieldsetDir()
{
  std::string dir = getSavePath() + SHIELDSETDIR + "/";
  return dir;
}

std::string File::getCitysetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + CITYSETDIR + "/";
}

std::string File::getUserCitysetDir()
{
  std::string dir = getSavePath() + CITYSETDIR + "/";
  return dir;
}


bool File::nameEndsWith(std::string filename, std::string extension)
{
  std::string::size_type idx = filename.rfind(extension);
  if (idx == std::string::npos)
    return false;
  if (idx == filename.length() - extension.length())
    return true;
  return false;
}

void File::erase(std::string filename)
{
  remove(filename.c_str());
}

void File::erase_dir(std::string filename)
{
  rmdir(filename.c_str());
}
std::string File::getSetConfigurationFilename(std::string dir, std::string subdir, std::string ext)
{
  return add_slash_if_necessary(dir) + subdir + "/" + subdir + ext;
}
char *File::sanify(const char *string)
{
  char *result = NULL;
  size_t resultlen = 1;
  size_t len = strlen(string);
  result = (char*) malloc (resultlen);
  result[0] = '\0';
  for (unsigned int i = 0; i < len; i++)
    {
      int letter = tolower(string[i]);
      if (strchr("abcdefghijklmnopqrstuvwxyz0123456789-", letter) == NULL)
	continue;

      resultlen++;
      result = (char *) realloc (result, resultlen);
      if (result)
	{
	  result[resultlen-2] = char(letter);
	  result[resultlen-1] = '\0';
	}
    }
  return result;
}
