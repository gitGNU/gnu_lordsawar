// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

#include <config.h>

#include <fstream>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <string>
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
#include "file-compat.h"
#include "ucompose.hpp"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

namespace
{
    std::list<Glib::ustring> get_files(Glib::ustring path, Glib::ustring ext)
    {
	std::list<Glib::ustring> retlist;
	Glib::Dir dir(path);
    
	for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
          {
	    Glib::ustring entry = *i;
	    Glib::ustring::size_type idx = entry.rfind(ext);
	    if (idx != Glib::ustring::npos && 
                idx == entry.length() - ext.length())
              retlist.push_back(Glib::filename_to_utf8(path + entry));
          }
	return retlist;
    }
}

bool File::nameEndsWith(Glib::ustring filename, Glib::ustring extension)
{
  Glib::ustring::size_type idx = filename.rfind(extension);
  if (idx == Glib::ustring::npos)
    return false;
  if (idx == filename.length() - extension.length())
    return true;
  return false;
}

Glib::ustring File::add_ext_if_necessary(Glib::ustring file, Glib::ustring ext)
{
  if (nameEndsWith(file, ext) == true)
    return file;
  else
    return file + ext;
}

Glib::ustring File::add_slash_if_necessary(Glib::ustring dir)
{
  if (dir.c_str()[strlen(dir.c_str())-1] == '/')
    return dir;
  else
    return dir + "/";
}

Glib::ustring File::getMiscFile(Glib::ustring filename)
{
  return Configuration::s_dataPath + "/" + filename;
}

Glib::ustring File::getXSLTFile(guint32 type, Glib::ustring old_version, Glib::ustring new_version)
{
  FileCompat::Type t = FileCompat::Type(type);
  Glib::ustring filename = String::ucompose("%1-%2-%3",
                                            FileCompat::typeToCode(t), 
                                            old_version, new_version);
  Glib::ustring file = getMiscFile("various/xslt/" + filename + ".xsl");
  if (File::exists(file))
    return file;
  else
    return "";
}

Glib::ustring File::getUserProfilesDescription()
{
  return Configuration::s_savePath + "/" + PROFILE_LIST;
}

Glib::ustring File::getUserRecentlyPlayedGamesDescription()
{
  return Configuration::s_savePath + "/" + RECENTLY_PLAYED_LIST;
}

Glib::ustring File::getUserRecentlyHostedGamesDescription()
{
  return Configuration::s_savePath + "/" + RECENTLY_HOSTED_LIST;
}

Glib::ustring File::getUserRecentlyAdvertisedGamesDescription()
{
  return Configuration::s_savePath + "/" + RECENTLY_ADVERTISED_LIST;
}

Glib::ustring File::getUserRecentlyEditedFilesDescription()
{
  return Configuration::s_savePath + "/" + RECENTLY_EDITED_LIST;
}

Glib::ustring File::getItemDescription()
{
  return Configuration::s_dataPath + "/various/items/items.xml";
}

Glib::ustring File::getEditorFile(Glib::ustring filename)
{
  return Configuration::s_dataPath + "/various/editor/" + filename + ".png";
}

Glib::ustring File::getMusicFile(Glib::ustring filename)
{
  return Glib::ustring(Configuration::s_dataPath + "/music/" + filename.c_str());
}

Glib::ustring File::getDataPath()
{
  return add_slash_if_necessary(Configuration::s_dataPath);
}

Glib::ustring File::getSavePath()
{
  return add_slash_if_necessary(Configuration::s_savePath);
}

Glib::ustring File::getUserMapDir()
{
  return add_slash_if_necessary(Configuration::s_savePath) + MAPDIR + "/";
}

Glib::ustring File::getMapDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + MAPDIR + "/";
}

Glib::ustring File::getUserMapFile(Glib::ustring file)
{
  return getUserMapDir() + file;
}

Glib::ustring File::getMapFile(Glib::ustring file)
{
  return getMapDir() + file;
}
std::list<Glib::ustring> File::scanUserMaps()
{
  Glib::ustring path = File::getUserMapDir();
    
    std::list<Glib::ustring> retlist;
    Glib::Dir dir(path);
    
    for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      Glib::ustring entry = *i;
      Glib::ustring::size_type idx = entry.find(".map");
      if (idx != Glib::ustring::npos)
	{
	  if (entry == "random.map")
	    continue;
	  retlist.push_back(Glib::filename_to_utf8(entry));
	}
    }
    
    return retlist;
}

std::list<Glib::ustring> File::scanMaps()
{
  Glib::ustring path = File::getMapDir();
    
    std::list<Glib::ustring> retlist;
    Glib::Dir dir(path);
    
    for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      Glib::ustring entry = *i;
      Glib::ustring::size_type idx = entry.find(".map");
      if (idx != Glib::ustring::npos)
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

Glib::ustring File::get_dirname(Glib::ustring path)
{
  return Glib::path_get_dirname(path);
}
Glib::ustring File::get_basename(Glib::ustring path, bool keep_ext)
{
  Glib::ustring file;
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
bool File::create_dir(Glib::ustring dir)
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
	
bool File::is_writable(Glib::ustring file)
{
#ifndef __WIN32__
  if (access (file.c_str(), W_OK) != 0)
    return false;
  else
    return true;
#endif
  return false;
}

bool File::exists(Glib::ustring f)
{
  FILE *fileptr = fopen (f.c_str(), "r");
  bool retval = fileptr != NULL;
  if (fileptr)
    fclose(fileptr);
  return retval;
}

//armysets 

Glib::ustring File::getArmysetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + ARMYSETDIR + "/";
}

Glib::ustring File::getUserArmysetDir()
{
  Glib::ustring dir =  getSavePath() + ARMYSETDIR + "/";
  return dir;
}

std::list<Glib::ustring> File::scanForFiles(Glib::ustring dir, Glib::ustring extension)
{
  std::list<Glib::ustring> files;
  try
    {
      files = get_files (dir, extension);
    }
  catch(const Glib::Exception &ex)
    {
      return files;
    }
    return files;
}


Glib::ustring File::getTilesetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + TILESETDIR + "/";
}

Glib::ustring File::getUserTilesetDir()
{
  Glib::ustring dir = getSavePath() + TILESETDIR + "/";
  return dir;
}

//shieldsets
Glib::ustring File::getShieldsetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + SHIELDSETDIR + "/";
}

Glib::ustring File::getUserShieldsetDir()
{
  Glib::ustring dir = getSavePath() + SHIELDSETDIR + "/";
  return dir;
}

Glib::ustring File::getCitysetDir()
{
  return add_slash_if_necessary(Configuration::s_dataPath) + CITYSETDIR + "/";
}

Glib::ustring File::getUserCitysetDir()
{
  Glib::ustring dir = getSavePath() + CITYSETDIR + "/";
  return dir;
}


void File::erase(Glib::ustring filename)
{
  remove(filename.c_str());
}

void File::erase_dir(Glib::ustring filename)
{
  rmdir(filename.c_str());
}

void File::clean_dir(Glib::ustring dirname)
{
  if (File::exists(dirname) == false)
    return;
  Glib::Dir dir(dirname);
  for (Glib::DirIterator it = dir.begin(); it != dir.end(); it++)
    File::erase(File::add_slash_if_necessary(dirname) + *it);
  dir.close();
  File::erase_dir(dirname);
}

Glib::ustring File::getSetConfigurationFilename(Glib::ustring dir, Glib::ustring subdir, Glib::ustring ext)
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
  
Glib::ustring File::get_tmp_file()
{
  std::string tmpfile = "lw.XXXX";
  int fd = Glib::file_open_tmp(tmpfile, "lw.XXXX");
  close(fd);
  return tmpfile;
}

Glib::ustring File::get_extension(Glib::ustring filename)
{
  if (filename.rfind('.') == Glib::ustring::npos)
    return "";
  return filename.substr(filename.rfind('.'));
}

//this method is from http://www.cplusplus.com/reference/list/list/sort/
bool case_insensitive (const Glib::ustring& first, const Glib::ustring& second)
{
  unsigned int i = 0;
  while (i < first.length () && i < second.length ())
    {
      if (tolower (first[i]) < tolower (second[i])) 
        return true;
      else if (tolower (first[i]) > tolower (second[i])) 
        return false;
      ++i;
    }
  return (first.length() < second.length());
}
