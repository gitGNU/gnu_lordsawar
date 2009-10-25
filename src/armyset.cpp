//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <iostream>
#include <expat.h>
#include <gtkmm.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>
#include <string>

#include "armyset.h"
#include "File.h"
#include "GraphicsCache.h"
#include "shield.h"
#include "gui/image-helpers.h"

std::string Armyset::d_tag = "armyset";
std::string Armyset::file_extension = ARMYSET_EXT;
using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

#define DEFAULT_ARMY_TILE_SIZE 40
Armyset::Armyset(guint32 id, std::string name)
	: d_id(id), d_name(name), d_copyright(""), d_license(""), d_subdir(""), 
	d_tilesize(DEFAULT_ARMY_TILE_SIZE), d_ship(0), d_shipmask(0), 
	d_standard(0), d_standard_mask(0), d_bag(0)
{
}

Armyset::Armyset(XML_Helper *helper, std::string directory)
    : d_id(0), d_name(""), d_copyright(""), d_license(""), d_subdir(""), 
    d_tilesize(DEFAULT_ARMY_TILE_SIZE), d_ship(0), d_shipmask(0), 
    d_standard(0), d_standard_mask(0), d_bag(0)
{
  setDirectory(directory);
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_copyright, "copyright");
  helper->getData(d_license, "license");
  helper->getData(d_tilesize, "tilesize");
  helper->getData(d_stackship_name, "stackship");
  helper->getData(d_standard_name, "plantedstandard");
  helper->getData(d_bag_name, "bag");
  helper->registerTag(ArmyProto::d_tag, 
		      sigc::mem_fun((*this), &Armyset::loadArmyProto));
}

Armyset::~Armyset()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); it++)
    delete *it;
}

bool Armyset::loadArmyProto(string tag, XML_Helper* helper)
{
    if (tag == ArmyProto::d_tag)
      {
	std::string s;
	ArmyProto* a = new ArmyProto(helper);
	a->setTypeId(size());
	a->setArmyset(d_id);
	push_back(a);
      }
    return true;
}

bool Armyset::save(XML_Helper* helper)
{
    bool retval = true;

    retval &= helper->openTag(d_tag);

    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("copyright", d_copyright);
    retval &= helper->saveData("license", d_license);
    retval &= helper->saveData("tilesize", d_tilesize);
    retval &= helper->saveData("stackship", d_stackship_name);
    retval &= helper->saveData("plantedstandard", d_standard_name);
    retval &= helper->saveData("bag", d_bag);

    for (const_iterator it = begin(); it != end(); it++)
      (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

ArmyProto * Armyset::lookupArmyByType(guint32 army_type_id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getTypeId() == army_type_id)
	return *it;
    }
  return NULL;
}
	
bool Armyset::validateSize()
{
  if (size() == 0)
    return false;
  return true;
}

bool Armyset::validateHero()
{
  bool found = false;
  //do we have a hero?
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->isHero() == true)
	{
	  found = true;
	  break;
	}
    }
  if (!found)
    return false;
  return true;
}
bool Armyset::validatePurchasables()
{
  bool found = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getNewProductionCost() > 0 )
	{
	  found = true;
	  break;
	}
    }
  if (!found)
    return false;
  return true;
}

bool Armyset::validateRuinDefenders()
{
  bool found = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getDefendsRuins() == true)
	{
	  found = true;
	  break;
	}
    }
  if (!found)
    return false;
  return true;
}

bool Armyset::validateAwardables()
{
  bool found = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getAwardable() == true)
	{
	  found = true;
	  break;
	}
    }
  if (!found)
    return false;
  return true;
}
bool Armyset::validateShip()
{
  if (getShipImageName() == "")
    return false;
  return true;
}

bool Armyset::validateBag()
{
  if (getBagImageName() == "")
    return false;
  return true;
}

bool Armyset::validateStandard()
{
  if (getStandardImageName() == "")
    return false;
  return true;
}

bool Armyset::validateArmyUnitImage(ArmyProto *army, Shield::Colour &c)
{
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    if (army->getImageName(Shield::Colour(i)) == "")
      {
	c = Shield::Colour(i);
	return false;
      }
  return true;
}
bool Armyset::validateArmyUnitImages()
{
  Shield::Colour c;
  for (iterator it = begin(); it != end(); it++)
    {
      if (validateArmyUnitImage(*it, c) == false)
	return false;
    }
  return true;
}

bool Armyset::validateArmyUnitName(ArmyProto *army)
{
  if (army->getName() == "")
    return false;
  return true;
}
bool Armyset::validateArmyUnitNames()
{
  for (iterator it = begin(); it != end(); it++)
    {
      if (validateArmyUnitName(*it) == false)
	return false;
    }
  return true;
}
bool Armyset::validate()
{
  bool valid = true;
  valid = validateSize();
  if (!valid)
    return false;
  valid = validateHero();
  if (!valid)
    return false;
  valid = validatePurchasables();
  if (!valid)
    return false;
  //do we have any units that defend ruins?
  valid = validateRuinDefenders();
  if (!valid)
    return false;
  //do we have any units that can be awarded?
  valid = validateAwardables();
  if (!valid)
    return false;
  //is the stackship set?
  valid = validateShip();
  if (!valid)
    return false;
  //is the standard set?
  valid = validateStandard();
  if (!valid)
    return false;
  //is the bag set?
  valid = validateBag();
  if (!valid)
    return false;
  //is there an image set for each army unit?
  valid = validateArmyUnitImages();
  if (!valid)
    return false;
  //is there a name set for each army unit?
  valid = validateArmyUnitNames();
  if (!valid)
    return false;

  return valid;
}
class ArmysetLoader
{
public:
    ArmysetLoader(std::string filename)
      {
	armyset = NULL;
	dir = File::get_dirname(filename);
	if (File::nameEndsWith(filename, Armyset::file_extension) == false)
	  filename += Armyset::file_extension;
	XML_Helper helper(filename, ios::in, false);
	helper.registerTag(Armyset::d_tag, sigc::mem_fun((*this), &ArmysetLoader::load));
	if (!helper.parse())
	  {
	    std::cerr << "Error, while loading an armyset. Armyset Name: ";
	    std::cerr <<File::get_basename(File::get_dirname(filename)) <<std::endl <<std::flush;
	  }
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Armyset::d_tag)
	  {
	    armyset = new Armyset(helper, dir);
	    return true;
	  }
	return false;
      };
    std::string dir;
    Armyset *armyset;
};
Armyset *Armyset::create(std::string filename)
{
  ArmysetLoader d(filename);
  return d.armyset;
}
void Armyset::getFilenames(std::list<std::string> &files)
{
  for (iterator it = begin(); it != end(); it++)
    {
      for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
	{
	  std::string file = (*it)->getImageName(Shield::Colour(i));
	  if (std::find(files.begin(), files.end(), file) == files.end())
	    files.push_back(file);
	}
    }
}
	
void Armyset::instantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages(this);
  if (getShipImageName().empty() == false)
    loadShipPic(getFile(getShipImageName()));
  if (getStandardImageName().empty() == false)
    loadStandardPic(getFile(getStandardImageName()));
  if (getBagImageName().empty() == false)
    loadBagPic(getFile(getBagImageName()));
}

void Armyset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

void Armyset::loadShipPic(std::string image_filename)
{
  if (image_filename.empty() == true)
    return;
  std::vector<PixMask*> half;
  half = disassemble_row(image_filename, 2);
  int size = getTileSize();
  PixMask::scale(half[0], size, size);
  PixMask::scale(half[1], size, size);
  setShipImage(half[0]);
  setShipMask(half[1]);
}

void Armyset::loadBagPic(std::string image_filename)
{
  if (image_filename.empty() == true)
    return;
  setBagPic(PixMask::create(image_filename));
}
void Armyset::loadStandardPic(std::string image_filename)
{
  if (image_filename.empty() == true)
    return;
  std::vector<PixMask*> half = disassemble_row(image_filename, 2);
  int size = getTileSize();
  PixMask::scale(half[0], size, size);
  PixMask::scale(half[1], size, size);
  setStandardPic(half[0]);
  setStandardMask(half[1]);
}

std::string Armyset::getConfigurationFile()
{
  return getDirectory() + d_subdir + file_extension;
}

std::list<std::string> Armyset::scanUserCollection()
{
  return File::scanFiles(File::getUserArmysetDir(), file_extension);
}

std::list<std::string> Armyset::scanSystemCollection()
{
  std::list<std::string> retlist = File::scanFiles(File::getArmysetDir(), 
						   file_extension);
  if (retlist.empty())
    {
      std::cerr << "Couldn't find any armysets!" << std::endl;
      std::cerr << "Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc" << std::endl;
      std::cerr << "Exiting!" << std::endl;
      exit(-1);
    }

  return retlist;
}

