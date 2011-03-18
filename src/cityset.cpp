// Copyright (C) 2008, 2010, 2011 Ben Asselstine
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
#include "tarhelper.h"
#include "Configuration.h"
#include "file-compat.h"

std::string Cityset::d_tag = "cityset";
std::string Cityset::file_extension = CITYSET_EXT;

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

#define DEFAULT_CITY_TILE_SIZE 40
Cityset::Cityset(guint32 id, std::string name)
	: d_id(id), d_name(name), d_copyright(""), d_license(""), 
	d_tileSize(DEFAULT_CITY_TILE_SIZE), d_basename("")
{
	d_cities_filename = "";
	d_razedcities_filename = "";
	d_port_filename = "";
	d_signpost_filename = "";
	d_ruins_filename = "";
	d_temples_filename = "";
	d_towers_filename = "";
	for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
	  citypics[i] = NULL;
	for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	  razedcitypics[i] = NULL;
	port = NULL;
	signpost = NULL;
	for (unsigned int i = 0; i < RUIN_TYPES; i++)
	  ruinpics[i] = NULL;
	for (unsigned int i = 0; i < TEMPLE_TYPES; i++)
	  templepics[i] = NULL;
	for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	  towerpics[i] = NULL;

	d_city_tile_width = 2;
	d_temple_tile_width = 1;
	d_ruin_tile_width = 1;
}

Cityset::Cityset(XML_Helper *helper, std::string directory)
	:Set()
{
  setDirectory(directory);
  helper->getData(d_id, "id"); 
  helper->getData(d_name, "name"); 
  helper->getData(d_copyright, "copyright"); 
  helper->getData(d_license, "license"); 
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
  clean_tmp_dir();
}

class CitysetLoader
{
public:
    CitysetLoader(std::string filename, bool &broken, bool &unsupported)
      {
        unsupported_version = false;
	cityset = NULL;
	dir = File::get_dirname(filename);
        file = File::get_basename(filename);
	if (File::nameEndsWith(filename, Cityset::file_extension) == false)
	  filename += Cityset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        std::string lwcfilename = 
          t.getFirstFile(Cityset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwcfilename, ios::in, false);
	helper.registerTag(Cityset::d_tag, sigc::mem_fun((*this), &CitysetLoader::load));
	if (!helper.parse())
	  {
            unsupported = unsupported_version;
	    std::cerr << "Error, while loading a cityset. Cityset File: ";
	    std::cerr << filename << std::endl <<std::flush;
	    if (cityset != NULL)
	      delete cityset;
	    cityset = NULL;
	  }
        File::erase(lwcfilename);
        helper.close();
        t.Close();
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Cityset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_CITYSET_VERSION)
              {
                cityset = new Cityset(helper, dir);
                cityset->setBaseName(file);
                return true;
              }
            else
              {
                unsupported_version = true;
                return false;
              }
	  }
	return false;
      };
    std::string dir;
    std::string file;
    Cityset *cityset;
    bool unsupported_version;
};

Cityset *Cityset::create(std::string file, bool &unsupported_version)
{
  bool broken = false;
  CitysetLoader d(file, broken, unsupported_version);
  if (broken)
    return NULL;
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

bool Cityset::save(std::string filename, std::string extension) const
{
  bool broken = false;
  std::string goodfilename = File::add_ext_if_necessary(filename, extension);
  std::string tmpfile = "lw.XXXX";
  int fd = Glib::file_open_tmp(tmpfile, "lw.XXXX");
  close (fd);
  XML_Helper helper(tmpfile, std::ios::out, Configuration::s_zipfiles);
  helper.begin(LORDSAWAR_CITYSET_VERSION);
  broken = !save(&helper);
  helper.close();
  if (broken == true)
    return false;
  std::string tmptar = tmpfile + ".tar";
  Tar_Helper t(tmptar, std::ios::out, broken);
  if (broken == true)
    return false;
  t.saveFile(tmpfile, File::get_basename(goodfilename, true));
  //now the images, go get 'em from the tarball we were made from.
  std::list<std::string> delfiles;
  Tar_Helper orig(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      std::list<std::string> files = orig.getFilenamesWithExtension(".png");
      for (std::list<std::string>::iterator it = files.begin(); 
           it != files.end(); it++)
        {
          std::string pngfile = orig.getFile(*it, broken);
          if (broken == false)
            {
              t.saveFile(pngfile);
              delfiles.push_back(pngfile);
            }
          else
            break;
        }
      orig.Close();
    }
  else
    {
      FILE *fileptr = fopen (getConfigurationFile().c_str(), "r");
      if (fileptr)
        fclose (fileptr);
      else
        broken = false;
    }
  t.Close();
  for (std::list<std::string>::iterator it = delfiles.begin(); it != delfiles.end(); it++)
    File::erase(*it);
  File::erase(tmpfile);
  if (broken == false)
    {
      if (File::copy(tmptar, goodfilename) == 0)
        File::erase(tmptar);
    }

  return !broken;
}

bool Cityset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("id", d_id); 
  retval &= helper->saveData("name", d_name); 
  retval &= helper->saveData("copyright", d_copyright); 
  retval &= helper->saveData("license", d_license); 
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
				std::string temples_filename,
                                bool &broken)
{
  if (port_filename.empty() == false && !broken)
    setPortImage (PixMask::create(port_filename, broken));
  if (signpost_filename.empty() == false && !broken)
    setSignpostImage (PixMask::create(signpost_filename, broken));

      
  int citysize = d_tileSize * d_city_tile_width;
  if (cities_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > citypics;
      citypics = disassemble_row(cities_filename, MAX_PLAYERS + 1, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
            {
              if (citypics[i]->get_width() != citysize)
                PixMask::scale(citypics[i], citysize, citysize);
              setCityImage(i, citypics[i]);
            }
        }
    }

  if (razed_cities_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > razedcitypics;
      razedcitypics = disassemble_row(razed_cities_filename, MAX_PLAYERS, 
                                      broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < MAX_PLAYERS; i++)
            {
              if (razedcitypics[i]->get_width() != citysize)
                PixMask::scale(razedcitypics[i], citysize, citysize);
              setRazedCityImage(i, razedcitypics[i]);
            }
        }
    }

  if (towers_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > towerpics = disassemble_row(towers_filename, 
							 MAX_PLAYERS, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < MAX_PLAYERS; i++)
            {
              if (towerpics[i]->get_width() != (int)d_tileSize)
                PixMask::scale(towerpics[i], d_tileSize, d_tileSize);
              setTowerImage(i, towerpics[i]);
            }
        }
    }

  if (ruins_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > ruinpics = disassemble_row(ruins_filename, 
                                                        RUIN_TYPES, broken);
      if (!broken)
        {
          int ruinsize = d_tileSize * d_ruin_tile_width;
          for (unsigned int i = 0; i < RUIN_TYPES ; i++)
            {
              if (ruinpics[i]->get_width() != ruinsize)
                PixMask::scale(ruinpics[i], ruinsize, ruinsize);
              setRuinImage(i, ruinpics[i]);
            }
        }
    }

  if (temples_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > templepics;
      templepics = disassemble_row(temples_filename, TEMPLE_TYPES, broken);
      if (!broken)
        {
          int templesize = d_tileSize * d_temple_tile_width;
          for (unsigned int i = 0; i < TEMPLE_TYPES ; i++)
            {
              if (templepics[i]->get_width() != templesize)
                PixMask::scale(templepics[i], templesize, templesize);
              setTempleImage(i, templepics[i]);
            }
        }
    }
}

void Cityset::instantiateImages(bool &broken)
{
  debug("Loading images for cityset " << getName());
  uninstantiateImages();
  broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;
  std::string port_filename = "";
  std::string signpost_filename = "";
  std::string cities_filename = "";
  std::string razed_cities_filename = "";
  std::string towers_filename = "";
  std::string ruins_filename = "";
  std::string temples_filename = "";

  if (getPortFilename().empty() == false && !broken)
    port_filename = t.getFile(getPortFilename() + ".png", broken);
  if (getSignpostFilename().empty() == false && !broken)
    signpost_filename = t.getFile(getSignpostFilename() + ".png", broken);
  if (getCitiesFilename().empty() == false && !broken)
    cities_filename = t.getFile(getCitiesFilename() + ".png", broken);
  if (getRazedCitiesFilename().empty() == false && !broken)
    razed_cities_filename = t.getFile(getRazedCitiesFilename() + ".png", broken);
  if (getTowersFilename().empty() == false && !broken)
    towers_filename = t.getFile(getTowersFilename() + ".png", broken);
  if (getRuinsFilename().empty() == false && !broken)
    ruins_filename = t.getFile(getRuinsFilename() + ".png", broken);
  if (getTemplesFilename().empty() == false && !broken)
    temples_filename = t.getFile(getTemplesFilename() + ".png", broken);
  if (!broken)
    instantiateImages(port_filename, signpost_filename, cities_filename,
                      razed_cities_filename, towers_filename, ruins_filename,
                      temples_filename, broken);
  if (port_filename != "")
    File::erase(port_filename);
  if (signpost_filename != "")
    File::erase(signpost_filename);
  if (cities_filename != "")
    File::erase(cities_filename);
  if (razed_cities_filename != "")
    File::erase(razed_cities_filename);
  if (towers_filename != "")
    File::erase(towers_filename);
  if (ruins_filename != "")
    File::erase(ruins_filename);
  if (temples_filename != "")
    File::erase(temples_filename);
  t.Close();
}

std::string Cityset::getConfigurationFile() const
{
  return getDirectory() + d_basename + file_extension;
}

std::list<std::string> Cityset::scanUserCollection()
{
  return File::scanForFiles(File::getUserCitysetDir(), file_extension);
}

std::list<std::string> Cityset::scanSystemCollection()
{
  std::list<std::string> retlist = File::scanForFiles(File::getCitysetDir(), 
						      file_extension);
  if (retlist.empty())
    {
      std::cerr << "Couldn't find any citysets (*" << file_extension << 
        ") in : " << File::getCitysetDir() << std::endl;
      std::cerr << "Please check the path settings in ~/.lordsawarrc" << std::endl;
      exit(-1);
    }

  return retlist;
}

	
bool Cityset::validate()
{
  bool valid = true;
  if (validateCitiesFilename() == false)
    return false;
  if (validateRazedCitiesFilename() == false)
    return false;
  if (validatePortFilename() == false)
    return false;
  if (validateSignpostFilename() == false)
    return false;
  if (validateRuinsFilename() == false)
    return false;
  if (validateTemplesFilename() == false)
    return false;
  if (validateTowersFilename() == false)
    return false;
  if (validateCityTileWidth() == false)
    return false;
  if (validateRuinTileWidth() == false)
    return false;
  if (validateTempleTileWidth() == false)
    return false;
  return valid;
}
bool Cityset::validateCitiesFilename()
{
  if (getCitiesFilename().empty() == true)
    return false;
  return true;
}
bool Cityset::validateRazedCitiesFilename()
{
  if (getRazedCitiesFilename().empty() == true)
    return false;
  return true;
}
bool Cityset::validateSignpostFilename()
{
  if (getSignpostFilename().empty() == true)
    return false;
  return true;
}
bool Cityset::validatePortFilename()
{
  if (getPortFilename().empty() == true)
    return false;
  return true;
}
bool Cityset::validateRuinsFilename()
{
  if (getRuinsFilename().empty() == true)
    return false;
  return true;
}
bool Cityset::validateTemplesFilename()
{
  if (getTemplesFilename().empty() == true)
    return false;
  return true;
}
bool Cityset::validateTowersFilename()
{
  if (getTowersFilename().empty() == true)
    return false;
  return true;
}
bool Cityset::validateCityTileWidth()
{
  if (getCityTileWidth() <= 0)
    return false;
  return true; 
}
bool Cityset::validateRuinTileWidth()
{
  if (getRuinTileWidth() <= 0)
    return false;
  return true; 
}
bool Cityset::validateTempleTileWidth()
{
  if (getTempleTileWidth() <= 0)
    return false;
  return true; 
}
bool Cityset::tileWidthsEqual(Cityset *cityset)
{
  if (getCityTileWidth() == cityset->getCityTileWidth() &&
      getRuinTileWidth() == cityset->getRuinTileWidth() &&
      getTempleTileWidth() == cityset->getTempleTileWidth())
    return true;
  return false;
}

void Cityset::reload(bool &broken)
{
  broken = false;
  bool unsupported_version = false;
  CitysetLoader d(getConfigurationFile(), broken, unsupported_version);
  if (!broken && d.cityset && d.cityset->validate())
    {
      //steal the values from d.cityset and then don't delete it.
      uninstantiateImages();
      std::string basename = d_basename;
      *this = *d.cityset;
      instantiateImages(broken);
      d_basename = basename;
    }
}

std::string Cityset::getFileFromConfigurationFile(std::string file)
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      std::string filename = t.getFile(file, broken);
      t.Close();
  
      if (broken == false)
        return filename;
    }
  return "";
}

bool Cityset::replaceFileInConfigurationFile(std::string file, std::string new_file)
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      broken = t.replaceFile(file, new_file);
      t.Close();
    }
  return broken;
}

guint32 Cityset::calculate_preferred_tile_size() const
{
  guint32 tilesize = 0;
  std::map<guint32, guint32> sizecounts;

  if (citypics[0])
    sizecounts[citypics[0]->get_unscaled_width() / d_city_tile_width]++;
  if (razedcitypics[0])
    sizecounts[razedcitypics[0]->get_unscaled_width() / d_city_tile_width]++;
  if (port)
    sizecounts[port->get_unscaled_width()]++;
  if (signpost)
    sizecounts[signpost->get_unscaled_width()]++;
  if (ruinpics[0])
    sizecounts[ruinpics[0]->get_unscaled_width() / d_ruin_tile_width]++;
  if (templepics[0])
    sizecounts[templepics[0]->get_unscaled_width() / d_temple_tile_width]++;
  if (towerpics[0])
    sizecounts[towerpics[0]->get_unscaled_width()]++;

  guint32 maxcount = 0;
  for (std::map<guint32, guint32>::iterator it = sizecounts.begin(); 
       it != sizecounts.end(); it++)
    {
      if ((*it).second > maxcount)
        {
          maxcount = (*it).second;
          tilesize = (*it).first;
        }
    }
  if (tilesize == 0)
    tilesize = DEFAULT_CITY_TILE_SIZE;
  return tilesize;
}

bool Cityset::copy(std::string src, std::string dest)
{
  return Tar_Helper::copy(src, dest);
}

guint32 Cityset::countEmptyImageNames() const
{
  guint32 count = 0;
  if (d_cities_filename.empty() == true)
    count++;
  if (d_razedcities_filename.empty() == true)
    count++;
  if (d_port_filename.empty() == true)
    count++;
  if (d_signpost_filename.empty() == true)
    count++;
  if (d_ruins_filename.empty() == true)
    count++;
  if (d_temples_filename.empty() == true)
    count++;
  if (d_towers_filename.empty() == true)
    count++;
  return count;
}

void Cityset::clean_tmp_dir() const
{
  return Tar_Helper::clean_tmp_dir(getConfigurationFile());
}

bool Cityset::upgrade(std::string filename, std::string old_version, std::string new_version)
{
  return FileCompat::getInstance()->rewrite_with_updated_version
    (filename, FileCompat::CITYSET, d_tag, new_version);
  return true;
}

void Cityset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type (FileCompat::CITYSET, file_extension, 
                                           d_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::CITYSET, "0.2.0", LORDSAWAR_CITYSET_VERSION,
     sigc::ptr_fun(&Cityset::upgrade));
}
// End of file
