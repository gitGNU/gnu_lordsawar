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

std::string Armyset::d_tag = "armyset";
using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

#define DEFAULT_ARMY_TILE_SIZE 40
Armyset::Armyset(guint32 id, std::string name)
	: d_id(id), d_name(name), d_dir(""), d_tilesize(DEFAULT_ARMY_TILE_SIZE),
	d_ship(0), d_shipmask(0), d_standard(0), d_standard_mask(0),
	private_collection(true)
{
}

Armyset::Armyset(XML_Helper *helper, bool from_private_collection)
    : d_id(0), d_name(""), d_dir(""), d_tilesize(DEFAULT_ARMY_TILE_SIZE),
	d_ship(0), d_shipmask(0), d_standard(0), d_standard_mask(0)
{
  private_collection = from_private_collection;
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_tilesize, "tilesize");
  helper->getData(d_stackship_name, "stackship");
  helper->getData(d_standard_name, "plantedstandard");
  helper->registerTag(ArmyProto::d_tag, 
		      sigc::mem_fun((*this), &Armyset::loadArmyProto));
}

Armyset::~Armyset()
{
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
    retval &= helper->saveData("tilesize", d_tilesize);
    retval &= helper->saveData("stackship", d_stackship_name);
    retval &= helper->saveData("plantedstandard", d_standard_name);

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
      if ((*it)->getProductionCost() > 0 )
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
    ArmysetLoader(std::string name, bool p) 
      {
	armyset = NULL;
	private_collection = p;
	std::string filename = "";
	if (private_collection == false)
	  filename = File::getArmyset(name);
	else
	  filename = File::getUserArmyset(name);
	XML_Helper helper(filename, ios::in, false);
	helper.registerTag(Armyset::d_tag, sigc::mem_fun((*this), &ArmysetLoader::load));
	if (!helper.parse())
	  {
	    std::cerr << "Error, while loading an armyset. Armyset Name: ";
	    std::cerr <<name <<std::endl <<std::flush;
	  }
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Armyset::d_tag)
	  {
	    armyset = new Armyset(helper, private_collection);
	    return true;
	  }
	return false;
      };
    bool private_collection;
    Armyset *armyset;
};
Armyset *Armyset::create(std::string filename, bool private_collection)
{
  ArmysetLoader d(filename, private_collection);
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
