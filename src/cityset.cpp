// Copyright (C) 2008 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "cityset.h"

#include "File.h"
#include "xmlhelper.h"
#include "gui/image-helpers.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"

std::string Cityset::d_tag = "cityset";
std::string Cityset::file_extension = CITYSET_EXT;

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Cityset::Cityset(XML_Helper *helper, std::string directory)
	:Set()
{
  setDirectory(directory);
  helper->getData(d_id, "id"); 
  helper->getData(d_name, "name"); 
  helper->getData(d_info, "info");
  helper->getData(d_tileSize, "tilesize");
  helper->getData(d_cities_filename, "cities");
  helper->getData(d_razedcities_filename, "razed_cities");
  helper->getData(d_port_filename, "port");
  helper->getData(d_signpost_filename, "signpost");
  helper->getData(d_ruins_filename, "ruins");
  helper->getData(d_temples_filename, "temples");
  helper->getData(d_towers_filename, "towers");
  helper->getData(d_city_tile_width, "city_tile_width");
  helper->getData(d_temple_tile_width, "temple_tile_width");
  helper->getData(d_ruin_tile_width, "ruin_tile_width");
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    citypics[i] = NULL;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    razedcitypics[i] = NULL;
  for (unsigned int i = 0; i < RUIN_TYPES; i++)
    ruinpics[i] = NULL;
  for (unsigned int i = 0; i < TEMPLE_TYPES; i++)
    templepics[i] = NULL;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    towerpics[i] = NULL;
  port = NULL;
  signpost = NULL;
}

Cityset::~Cityset()
{
  uninstantiateImages();
}

class CitysetLoader
{
public:
    CitysetLoader(std::string filename)
      {
	cityset = NULL;
	dir = File::get_dirname(filename);
	if (File::nameEndsWith(filename, Cityset::file_extension) == false)
	  filename += Cityset::file_extension;
	XML_Helper helper(filename, ios::in, false);
	helper.registerTag(Cityset::d_tag, sigc::mem_fun((*this), &CitysetLoader::load));
	if (!helper.parse())
	  {
	    std::cerr << "Error, while loading an cityset. Cityset Name: ";
	    std::cerr <<File::get_basename(filename)<<std::endl <<std::flush;
	  }
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Cityset::d_tag)
	  {
	    cityset = new Cityset(helper, dir);
	    return true;
	  }
	return false;
      };
    std::string dir;
    Cityset *cityset;
};
Cityset *Cityset::create(std::string file)
{
  CitysetLoader d(file);
  return d.cityset;
}
void Cityset::getFilenames(std::list<std::string> &files)
{
  files.push_back(d_cities_filename);
  files.push_back(d_razedcities_filename);
  files.push_back(d_port_filename);
  files.push_back(d_signpost_filename);
  files.push_back(d_ruins_filename);
  files.push_back(d_temples_filename);
  files.push_back(d_towers_filename);
}

bool Cityset::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("id", d_id); 
  retval &= helper->saveData("name", d_name); 
  retval &= helper->saveData("info", d_info);
  retval &= helper->saveData("tilesize", d_tileSize);
  retval &= helper->saveData("cities", d_cities_filename);
  retval &= helper->saveData("razed_cities", d_razedcities_filename);
  retval &= helper->saveData("port", d_port_filename);
  retval &= helper->saveData("signpost", d_signpost_filename);
  retval &= helper->saveData("ruins", d_ruins_filename);
  retval &= helper->saveData("temples", d_temples_filename);
  retval &= helper->saveData("towers", d_towers_filename);
  retval &= helper->saveData("city_tile_width", d_city_tile_width);
  retval &= helper->saveData("temple_tile_width", d_temple_tile_width);
  retval &= helper->saveData("ruin_tile_width", d_ruin_tile_width);
  retval &= helper->closeTag();
  return retval;
}

void Cityset::uninstantiateImages()
{
  if (getPortImage() != NULL)
    {
      delete getPortImage();
      setPortImage(NULL);
    }
  if (getSignpostImage() != NULL)
    {
      delete getSignpostImage();
      setSignpostImage(NULL);
    }
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (getCityImage(i) != NULL)
	{
	  delete getCityImage(i);
	  setCityImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (getRazedCityImage(i) != NULL)
	{
	  delete getRazedCityImage(i);
	  setRazedCityImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (getTowerImage(i) != NULL)
	{
	  delete getTowerImage(i);
	  setTowerImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < RUIN_TYPES; i++)
    {
      if (getRuinImage(i) != NULL)
	{
	  delete getRuinImage(i);
	  setRuinImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < TEMPLE_TYPES; i++)
    {
      if (getTempleImage(i) != NULL)
	{
	  delete getTempleImage(i);
	  setTempleImage(i, NULL);
	}
    }
}

void Cityset::instantiateImages(std::string port_filename,
				std::string signpost_filename,
				std::string cities_filename,
				std::string razed_cities_filename,
				std::string towers_filename,
				std::string ruins_filename,
				std::string temples_filename)
{
  if (port_filename.empty() == false)
    setPortImage (PixMask::create(port_filename));
  if (signpost_filename.empty() == false)
    setSignpostImage (PixMask::create(signpost_filename));

      
  int citysize = d_tileSize * d_city_tile_width;
  if (cities_filename.empty() == false)
    {
      std::vector<PixMask* > citypics;
      citypics = disassemble_row(cities_filename, MAX_PLAYERS + 1);
      for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
	{
	  if (citypics[i]->get_width() != citysize)
	    PixMask::scale(citypics[i], citysize, citysize);
	  setCityImage(i, citypics[i]);
	}
    }

  if (razed_cities_filename.empty() == false)
    {
      std::vector<PixMask* > razedcitypics;
      razedcitypics = disassemble_row(razed_cities_filename, MAX_PLAYERS);
      for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	{
	  if (razedcitypics[i]->get_width() != citysize)
	    PixMask::scale(razedcitypics[i], citysize, citysize);
	  setRazedCityImage(i, razedcitypics[i]);
	}
    }

  if (towers_filename.empty() == false)
    {
      std::vector<PixMask* > towerpics = disassemble_row(towers_filename, 
							 MAX_PLAYERS);
      for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	{
	  if (towerpics[i]->get_width() != (int)d_tileSize)
	    PixMask::scale(towerpics[i], d_tileSize, d_tileSize);
	  setTowerImage(i, towerpics[i]);
	}
    }

  if (ruins_filename.empty() == false)
    {
      std::vector<PixMask* > ruinpics = disassemble_row(ruins_filename, RUIN_TYPES);
      int ruinsize = d_tileSize * d_ruin_tile_width;
      for (unsigned int i = 0; i < RUIN_TYPES ; i++)
	{
	  if (ruinpics[i]->get_width() != ruinsize)
	    PixMask::scale(ruinpics[i], ruinsize, ruinsize);
	  setRuinImage(i, ruinpics[i]);
	}
    }

  if (temples_filename.empty() == false)
    {
      std::vector<PixMask* > templepics;
      templepics = disassemble_row(temples_filename, TEMPLE_TYPES);
      int templesize = d_tileSize * d_temple_tile_width;
      for (unsigned int i = 0; i < TEMPLE_TYPES ; i++)
	{
	  if (templepics[i]->get_width() != templesize)
	    PixMask::scale(templepics[i], templesize, templesize);
	  setTempleImage(i, templepics[i]);
	}
    }
}

void Cityset::instantiateImages()
{
  debug("Loading images for cityset " << getName());
  uninstantiateImages();
  std::string port_filename = "";
  std::string signpost_filename = "";
  std::string cities_filename = "";
  std::string razed_cities_filename = "";
  std::string towers_filename = "";
  std::string ruins_filename = "";
  std::string temples_filename = "";

  if (getPortFilename().empty() == false)
    port_filename = getFile(getPortFilename());
  if (getSignpostFilename().empty() == false)
    signpost_filename = getFile(getSignpostFilename());
  if (getCitiesFilename().empty() == false)
    cities_filename = getFile(getCitiesFilename());
  if (getRazedCitiesFilename().empty() == false)
    razed_cities_filename = getFile(getRazedCitiesFilename());
  if (getTowersFilename().empty() == false)
    towers_filename = getFile(getTowersFilename());
  if (getRuinsFilename().empty() == false)
    ruins_filename = getFile(getRuinsFilename());
  if (getTemplesFilename().empty() == false)
    temples_filename = getFile(getTemplesFilename());
  instantiateImages(port_filename, signpost_filename, cities_filename,
		    razed_cities_filename, towers_filename, ruins_filename,
		    temples_filename);
}

std::string Cityset::getConfigurationFile()
{
  return getDirectory() + d_subdir + file_extension;
}

std::list<std::string> Cityset::scanUserCollection()
{
  return File::scanFiles(File::getUserCitysetDir(), file_extension);
}

std::list<std::string> Cityset::scanSystemCollection()
{
  std::list<std::string> retlist = File::scanFiles(File::getCitysetDir(), 
						   file_extension);
  if (retlist.empty())
    {
      std::cerr << "Couldn't find any citysets!" << std::endl;
      std::cerr << "Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc" << std::endl;
      std::cerr << "Exiting!" << std::endl;
      exit(-1);
    }

  return retlist;
}

// End of file
