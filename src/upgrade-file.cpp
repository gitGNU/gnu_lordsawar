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
#include "GameScenario.h"
#include "profilelist.h"
#include "recently-played-game-list.h"
#include "gamelist.h"
#include "ucompose.hpp"
#include "editor/recently-edited-file-list.h"
#include "Itemlist.h"

bool upgrade_file (std::string filename, std::string tag, std::string version, bool &same)
{
  bool upgraded = false;
  if (tag == Configuration::d_tag)
    {
      if (LORDSAWAR_CONFIG_VERSION == version)
        same = true;
      else
        upgraded = Configuration::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == Itemlist::d_tag)
    {
      if (LORDSAWAR_ITEMS_VERSION == version)
        same = true;
      else
        upgraded = Itemlist::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == Profilelist::d_tag)
    {
      if (LORDSAWAR_PROFILES_VERSION == version)
        same = true;
      else
        upgraded = Profilelist::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == RecentlyPlayedGameList::d_tag)
    {
      if (LORDSAWAR_RECENTLY_PLAYED_VERSION == version)
        same = true;
      else
        upgraded = RecentlyPlayedGameList::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == Gamelist::d_tag)
    {
      if (LORDSAWAR_RECENTLY_HOSTED_VERSION == version)
        same = true;
      else
        upgraded = Gamelist::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == RecentlyEditedFileList::d_tag)
    {
      if (LORDSAWAR_RECENTLY_EDITED_VERSION == version)
        same = true;
      else
        upgraded = RecentlyEditedFileList::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == Armyset::d_tag)
    {
      if (LORDSAWAR_ARMYSET_VERSION == version)
        same = true;
      else
        upgraded = Armyset::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == Tileset::d_tag)
    {
      if (LORDSAWAR_TILESET_VERSION == version)
        same = true;
      else
        upgraded = Tileset::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == Cityset::d_tag)
    {
      if (LORDSAWAR_CITYSET_VERSION == version)
        same = true;
      else
        upgraded = Cityset::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == Shieldset::d_tag)
    {
      if (LORDSAWAR_SHIELDSET_VERSION == version)
        same = true;
      else
        upgraded = Shieldset::upgradeOldVersionsOfFile(filename);
    }
  else if (tag == GameScenario::d_top_tag)
    {
      if (LORDSAWAR_SAVEGAME_VERSION == version)
        same = true;
      else
        upgraded = GameScenario::upgradeOldVersionsOfFile(filename);
    }
  else
    {
      std::cerr << 
        String::ucompose(_("Error: `%1' files are not supported."), tag) << 
        std::endl;
    }
  return upgraded;
}

bool get_tag_and_version_from_file(std::string filename, std::string &tag, std::string &version)
{
  bool broken = false;
  if (File::nameEndsWith(filename, ARMYSET_EXT) == true ||
      File::nameEndsWith(filename, TILESET_EXT) == true ||
      File::nameEndsWith(filename, CITYSET_EXT) == true ||
      File::nameEndsWith(filename, SHIELDSET_EXT) == true ||
      File::nameEndsWith(filename, MAP_EXT) == true ||
      File::nameEndsWith(filename, SAVE_EXT) == true)
    {
      std::string ext = "";
      if (filename.rfind('.') == std::string::npos)
        return false;
      ext = filename.substr(filename.rfind('.'));
      Tar_Helper t(filename, std::ios::in, broken);
      if (!broken)
        {
          std::string tmpfile = t.getFirstFile(ext, broken);
          XML_Helper helper(tmpfile, std::ios::in, Configuration::s_zipfiles);
          tag = XML_Helper::get_top_tag(tmpfile, Configuration::s_zipfiles);
          VersionLoader l(tmpfile, tag, version, broken, 
                          Configuration::s_zipfiles);
          t.Close();
          File::erase(tmpfile);
        }
    }
  else
    {
      tag = XML_Helper::get_top_tag(filename, false);
      VersionLoader l(filename, tag, version, broken);
    }
  return !broken;
}

bool upgrade(std::string filename, bool &same)
{
  std::string tag;
  std::string version;
  if (get_tag_and_version_from_file (filename, tag, version))
    return upgrade_file (filename, tag, version, same);
  else
    return false;
}

int max_vector_width;
int main(int argc, char* argv[])
{
  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);

  #if ENABLE_NLS
  //cout << "Configuration::s_lang.c_str(): " << Configuration::s_lang.c_str() << endl;
  setlocale(LC_ALL, Configuration::s_lang.c_str());
  //setlocale(LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
  #endif

  if (argc != 2)
    {
      std::cerr << "Usage: " << argv[0] << "FILE" << std::endl;
      return EXIT_FAILURE;
    }

  bool same_version = false;
  bool upgraded = upgrade(argv[1], same_version);

  if (same_version)
    std::cerr << String::ucompose(_("%1 is already the latest version."), 
                                  argv[1]) << std::endl;
  if (!upgraded && !same_version)
    std::cerr << String::ucompose(_("Error: %1 could not be upgraded."), 
                                  argv[1]) << std::endl;

  return !upgraded;
}
