// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include "config.h"

#include <iostream>
#include <algorithm>
#include <glibmm/fileutils.h>
#include <glibmm/ustring.h>
#include <glibmm/convert.h>

#include "File.h"
#include <SDL_image.h>
#include "Configuration.h"
#include "Campaign.h"
#include "defs.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

namespace
{
    // returns a list of full paths of the immediate subdirs of path
    std::list<std::string> get_immediate_subdirs(std::string path)
    {
	Glib::Dir dir(path);
	std::list<std::string> dirlist;

	for (Glib::Dir::iterator i = dir.begin(), end = dir.end();
	     i != end; ++i)
	{
	    std::string entry = path + *i;
	    if (Glib::file_test(entry, Glib::FILE_TEST_IS_DIR))
		dirlist.push_back(entry);
	}

	return dirlist;
    }

    // returns a list of the XML file names in path with the ".xml" extension
    // stripped
    std::list<std::string> get_xml_files(std::string path)
    {
	std::list<std::string> retlist;
	Glib::Dir dir(path);
    
	for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
	{
	    std::string entry = *i;
	    std::string::size_type idx = entry.find(".xml");
	    if (idx != std::string::npos)
	    {
		entry.replace(idx, 4, "");  //substitute the ".xml" with ""
		retlist.push_back(Glib::filename_to_utf8(entry));
	    }
	}
	return retlist;
    }
    
    // returns a list of the XML file names in the immediate subdirs of path
    // with the ".xml" extension stripped
    std::list<std::string> get_xml_files_in_immediate_subdirs(std::string path)
    {
	std::list<std::string> retlist, dirlist = get_immediate_subdirs(path);
	for (std::list<std::string>::iterator i = dirlist.begin(),
		 end = dirlist.end(); i != end; ++i)
	{
	    std::list<std::string> files = get_xml_files(*i);
	
	    retlist.insert(retlist.end(), files.begin(), files.end());
	}
	return retlist;
    }
}

std::string getArmysetDir()
{
  return Configuration::s_dataPath + "/" + ARMYSETDIR + "/";
}

std::string getTilesetDir()
{
  return Configuration::s_dataPath + "/" + TILESETDIR + "/";
}

std::string getCitysetDir()
{
  return Configuration::s_dataPath + "/" + CITYSETDIR + "/";
}

std::string getShieldsetDir()
{
  return Configuration::s_dataPath + "/" + SHIELDSETDIR + "/";
}

std::string getCampaignDir()
{
  return Configuration::s_dataPath + "/" + CAMPAIGNDIR + "/";
}

std::list<std::string> File::scanArmysets()
{
    std::list<std::string> retlist = 
      get_xml_files_in_immediate_subdirs(getArmysetDir());

    if (retlist.empty())
    {
      std::cerr << _("Couldn't find any armysets!") << std::endl;
      std::cerr << _("Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc") << std::endl;
      std::cerr << _("Exiting!") << std::endl;
      exit(-1);
    }

    return retlist;
}

std::string File::getArmyset(std::string armysetsubdir)
{
  return getArmysetDir() + armysetsubdir + "/" + armysetsubdir + ".xml";
}

std::string File::getTileset(std::string tilesetsubdir)
{
  return getTilesetDir() + tilesetsubdir + "/" + tilesetsubdir + ".xml";
}

std::string File::getTilesetFile(std::string tilesetsubdir, std::string picname)
{
  return getTilesetDir() + tilesetsubdir + "/" + picname;
}

std::string File::getShieldsetFile(std::string shieldsetsubdir, std::string picname)
{
  return getShieldsetDir() + shieldsetsubdir + "/" + picname;
}

std::string File::getMiscFile(std::string filename)
{
  return Configuration::s_dataPath + "/" + filename;
}

std::string File::getCityset(std::string citysetdir)
{
  return getCitysetDir() + citysetdir + "/" + citysetdir + ".xml";
}

std::string File::getCitysetFile(std::string citysetsubdir, std::string picname)
{
  return getCitysetDir() + citysetsubdir + "/" + picname;
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

std::string File::getCampaignFile(std::string filename)
{
  return getCampaignDir() + "/" + filename;
}

std::string File::getSavePath()
{
    return Configuration::s_savePath + "/";
}

std::list<std::string> File::scanTilesets()
{
    std::list<std::string> retlist = 
      get_xml_files_in_immediate_subdirs(getTilesetDir());
    
    if (retlist.empty())
    {
      std::cerr << _("Couldn't find any tilesets!") << std::endl;
      std::cerr << _("Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc") << std::endl;
      std::cerr << _("Exiting!") << std::endl;
      exit(-1);
    }
    
    return retlist;
}

std::list<std::string> File::scanCitysets()
{
    std::list<std::string> retlist = 
      get_xml_files_in_immediate_subdirs(getCitysetDir());
    
    if (retlist.empty())
    {
      std::cerr << _("Couldn't find any citysets!") << std::endl;
      std::cerr << _("Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc") << std::endl;
      std::cerr << _("Exiting!") << std::endl;
        exit(-1);
    }
    
    return retlist;
}

std::list<std::string> File::scanCampaigns()
{
    std::string campaign;
    std::string path = getCampaignDir();
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
      std::cerr << _("Couldn't find a single campaign!") << std::endl;
      std::cerr << _("Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc") << std::endl;
    }

    //now we find the ones that are pointed to, and remove them
    for (std::list<std::string>::iterator it = retlist.begin(); 
	 it != retlist.end();)
      {
	campaign = Campaign::get_campaign_from_scenario_file(path + "/" + *it);
	if (find (retlist.begin(), retlist.end(), campaign) != retlist.end())
	  {
	    it = retlist.erase (it);
	    continue;
	  }
	it++;
      }

    return retlist;
}

std::list<std::string> File::scanMaps()
{
  std::string path = Configuration::s_dataPath + "/" + MAPDIR + "/";
    
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
      std::cerr << _("Couldn't find a single map!") << std::endl;
      std::cerr << _("Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc") << std::endl;
    }

    return retlist;
}

std::list<std::string> File::scanShieldsets()
{
    std::list<std::string> retlist = 
      get_xml_files_in_immediate_subdirs(getShieldsetDir());

    if (retlist.empty())
    {
      std::cerr << _("Couldn't find any shieldsets!") << std::endl;
      std::cerr << _("Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc") << std::endl;
      std::cerr << _("Exiting!") << std::endl;
      exit(-1);
    }

    return retlist;
}

std::string File::getShieldset(std::string shieldsetsubdir)
{
  return getShieldsetDir() + shieldsetsubdir + "/" + shieldsetsubdir + ".xml";
}

// End of file
