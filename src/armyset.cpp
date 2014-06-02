//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

#include "armyset.h"
#include "File.h"
#include "ImageCache.h"
#include "shield.h"
#include "gui/image-helpers.h"
#include "armysetlist.h"
#include "armyprodbase.h"
#include "tarhelper.h"
#include "Configuration.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "xmlhelper.h"

Glib::ustring Armyset::d_tag = "armyset";
Glib::ustring Armyset::file_extension = ARMYSET_EXT;

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

#define DEFAULT_ARMY_TILE_SIZE 40
Armyset::Armyset(guint32 id, Glib::ustring name)
	: d_id(id), d_name(name), d_copyright(""), d_license(""), d_basename(""), 
	d_tilesize(DEFAULT_ARMY_TILE_SIZE), d_ship(0), d_shipmask(0), 
	d_standard(0), d_standard_mask(0), d_bag(0)
{
  d_info = "";
  d_bag_name = "";
  d_stackship_name = "";
  d_standard_name = "";
}

Armyset::Armyset(XML_Helper *helper, Glib::ustring directory)
    : d_id(0), d_name(""), d_copyright(""), d_license(""), d_basename(""), 
    d_tilesize(DEFAULT_ARMY_TILE_SIZE), d_ship(0), d_shipmask(0), 
    d_standard(0), d_standard_mask(0), d_bag(0)
{
  d_bag_name = "";
  d_stackship_name = "";
  d_standard_name = "";
  setDirectory(directory);
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_copyright, "copyright");
  helper->getData(d_license, "license");
  helper->getData(d_info, "info");
  helper->getData(d_tilesize, "tilesize");
  helper->getData(d_stackship_name, "stackship");
  helper->getData(d_standard_name, "plantedstandard");
  helper->getData(d_bag_name, "bag");
  helper->registerTag(ArmyProto::d_tag, 
		      sigc::mem_fun((*this), &Armyset::loadArmyProto));
}

Armyset::Armyset(const Armyset& a)
  :Set(a), d_id(a.d_id), d_name(a.d_name), d_copyright(a.d_copyright),
    d_license(a.d_license), d_basename(a.d_basename), d_info(a.d_info),
    d_tilesize(a.d_tilesize)
{

  if (a.d_ship)
    d_ship = a.d_ship->copy();

  if (a.d_shipmask)
    d_shipmask = a.d_shipmask->copy();

  if (a.d_standard)
    d_standard = a.d_standard->copy();

  if (a.d_standard_mask)
    d_standard_mask = a.d_standard_mask->copy();

  if (a.d_bag)
    d_bag = a.d_bag->copy();

  d_standard_name = a.d_standard_name;
  d_stackship_name = a.d_stackship_name;
  d_bag_name = a.d_bag_name;

  for (const_iterator i = a.begin(); i != a.end(); i++)
    push_back(new ArmyProto(*(*i)));
}

Armyset::~Armyset()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); it++)
    delete *it;
  clean_tmp_dir();
}

bool Armyset::loadArmyProto(Glib::ustring tag, XML_Helper* helper)
{
    if (tag == ArmyProto::d_tag)
      {
        ArmyProto *a = new ArmyProto(helper);
        a->setArmyset(getId());
        push_back(a);
      }
    return true;
}

bool Armyset::save(Glib::ustring filename, Glib::ustring extension) const
{
  bool broken = false;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, extension);

  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  helper.begin(LORDSAWAR_ARMYSET_VERSION);
  broken = !save(&helper);
  helper.close();
  if (broken == true)
    return false;
  Glib::ustring tmptar = tmpfile + ".tar";
  Tar_Helper t(tmptar, std::ios::out, broken);
  if (broken == true)
    return false;
  t.saveFile(tmpfile, File::get_basename(goodfilename, true));
  //now the images, go get 'em from the tarball we were made from.
  std::list<Glib::ustring> delfiles;
  Tar_Helper orig(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      std::list<Glib::ustring> files = orig.getFilenamesWithExtension(".png");
      for (std::list<Glib::ustring>::iterator it = files.begin(); 
           it != files.end(); it++)
        {
          Glib::ustring pngfile = orig.getFile(*it, broken);
          if (broken == false)
            {
              bool success = t.saveFile(pngfile);
              if (!success)
                  broken = true;
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
  for (std::list<Glib::ustring>::iterator it = delfiles.begin(); it != delfiles.end(); it++)
    File::erase(*it);
  File::erase(tmpfile);
  t.Close();
  if (broken == false)
    {
      if (File::copy(tmptar, goodfilename) == 0)
        File::erase(tmptar);
    }

  return !broken;
}

bool Armyset::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(d_tag);

    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("copyright", d_copyright);
    retval &= helper->saveData("license", d_license);
    retval &= helper->saveData("info", d_info);
    retval &= helper->saveData("tilesize", d_tilesize);
    retval &= helper->saveData("stackship", d_stackship_name);
    retval &= helper->saveData("plantedstandard", d_standard_name);
    retval &= helper->saveData("bag", d_bag_name);

    for (const_iterator it = begin(); it != end(); it++)
      (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

ArmyProto * Armyset::lookupSimilarArmy(ArmyProto *army) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == army->getGender() &&
	  (*it)->getStrength() == army->getStrength() &&
	  (*it)->getProduction() == army->getProduction() &&
	  (*it)->getArmyBonus() == army->getArmyBonus() &&
	  (*it)->getMoveBonus() == army->getMoveBonus() &&
	  (*it)->getMaxMoves() == army->getMaxMoves() &&
	  (*it)->getAwardable() == army->getAwardable() &&
	  (*it)->getDefendsRuins() == army->getDefendsRuins())
	return *it;
    }
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == army->getGender() &&
	  (*it)->getStrength() == army->getStrength() &&
	  (*it)->getProduction() == army->getProduction() &&
	  (*it)->getArmyBonus() == army->getArmyBonus() &&
	  (*it)->getMoveBonus() == army->getMoveBonus() &&
	  (*it)->getMaxMoves() == army->getMaxMoves())
	return *it;
    }
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == army->getGender() &&
	  (*it)->getStrength() == army->getStrength() &&
	  (*it)->getProduction() == army->getProduction() &&
	  (*it)->getMaxMoves() == army->getMaxMoves())
	return *it;
    }
  return NULL;
}

ArmyProto * Armyset::lookupArmyByGender(Hero::Gender gender) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == gender)
	return *it;
    }
  return  NULL;
}
ArmyProto * Armyset::lookupArmyByStrengthAndTurns(guint32 str, guint32 turns) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if (str && turns)
	{
	  if ((*it)->getStrength() == str && (*it)->getProduction() == turns)
	    return *it;
	}
      else if (str && !turns)
	{
	  if ((*it)->getStrength() == str)
	    return *it;
	}
      else if (turns && !str)
	{
	  if ((*it)->getProduction() == turns)
	    return *it;
	}
    }
  return NULL;
}

ArmyProto * Armyset::lookupArmyByName(Glib::ustring name) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getName() == name)
	return *it;
    }
  return NULL;
}
	
ArmyProto * Armyset::lookupArmyByType(guint32 army_type_id) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == army_type_id)
	return *it;
    }
  return NULL;
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
bool Armyset::validateArmyTypeIds()
{
  std::list<guint32> ids = std::list<guint32>();
  for (iterator it = begin(); it != end(); it++)
    {
      if (std::find(ids.begin(), ids.end(), (*it)->getId()) == ids.end())
        ids.push_back((*it)->getId());
      else
        return false;
    }
  return true;
}
bool Armyset::validate()
{
  bool valid = true;
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
  //unique Ids per army unit?
  valid = validateArmyTypeIds();
  if (!valid)
    return false;

  return valid;
}

class ArmysetLoader
{
public:
    ArmysetLoader(Glib::ustring filename, bool &broken, bool &unsupported)
      {
        unsupported_version = false;
	armyset = NULL;
	dir = File::get_dirname(filename);
        file = File::get_basename(filename);
	if (File::nameEndsWith(filename, Armyset::file_extension) == false)
	  filename += Armyset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        Glib::ustring lwafilename = 
          t.getFirstFile(Armyset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwafilename, std::ios::in);
	helper.registerTag(Armyset::d_tag, sigc::mem_fun((*this), &ArmysetLoader::load));
	if (!helper.parse())
	  {
            unsupported = unsupported_version;
            std::cerr << String::ucompose(_("Error!  can't load armyset `%1'."), filename) << std::endl;
	    if (armyset != NULL)
	      delete armyset;
	    armyset = NULL;
	  }
        File::erase(lwafilename);
        helper.close();
        t.Close();
      };
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Armyset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_ARMYSET_VERSION)
              {
                armyset = new Armyset(helper, dir);
                armyset->setBaseName(file);
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
    Glib::ustring dir;
    Glib::ustring file;
    Armyset *armyset;
    bool unsupported_version;
};

Armyset *Armyset::create(Glib::ustring filename, bool &unsupported_version)
{
  bool broken = false;
  ArmysetLoader d(filename, broken, unsupported_version);
  if (broken)
    return NULL;
  return d.armyset;
}

void Armyset::getFilenames(std::list<Glib::ustring> &files)
{
  for (iterator it = begin(); it != end(); it++)
    {
      for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
	{
	  Glib::ustring file = (*it)->getImageName(Shield::Colour(i));
	  if (std::find(files.begin(), files.end(), file) == files.end())
	    files.push_back(file);
	}
    }
}
	
void Armyset::instantiateImages(bool &broken)
{
  uninstantiateImages();
  broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;

  for (iterator it = begin(); it != end(); ++it)
    (*it)->instantiateImages(getTileSize(), &t, broken);

  Glib::ustring ship_filename = "";
  Glib::ustring flag_filename = "";
  Glib::ustring bag_filename = "";
  if (getShipImageName().empty() == false && !broken)
    ship_filename = t.getFile(getShipImageName() + ".png", broken);
  if (getStandardImageName().empty() == false && !broken)
    flag_filename = t.getFile(getStandardImageName() + ".png", broken);
  if (getBagImageName().empty() == false && !broken)
    bag_filename = t.getFile(getBagImageName() + ".png", broken);

  if (!broken)
    {
      if (ship_filename.empty() == false)
        loadShipPic(ship_filename, broken);
      if (flag_filename.empty() == false)
        loadStandardPic(flag_filename, broken);
      if (bag_filename.empty() == false)
        loadBagPic(bag_filename, broken);
    }

  if (ship_filename.empty() == false)
    File::erase(ship_filename);
  if (flag_filename.empty() == false)
    File::erase(flag_filename);
  if (bag_filename.empty() == false)
    File::erase(bag_filename);
  t.Close();
}

void Armyset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

void Armyset::loadShipPic(Glib::ustring image_filename, bool &broken)
{
  if (image_filename.empty() == true)
    {
      broken = true;
      return;
    }
  std::vector<PixMask*> half;
  half = disassemble_row(image_filename, 2, broken);
  if (!broken)
    {
      int size = getTileSize();
      PixMask::scale(half[0], size, size);
      PixMask::scale(half[1], size, size);
      setShipImage(half[0]);
      setShipMask(half[1]);
    }
}

void Armyset::loadBagPic(Glib::ustring image_filename, bool &broken)
{
  if (image_filename.empty() == true)
    {
      broken = true;
      return;
    }
  if (!broken)
    setBagPic(PixMask::create(image_filename, broken));
}

void Armyset::loadStandardPic(Glib::ustring image_filename, bool &broken)
{
  if (image_filename.empty() == true)
    {
      broken = true;
      return;
    }
  std::vector<PixMask*> half = disassemble_row(image_filename, 2, broken);
  if (!broken)
    {
      int size = getTileSize();
      PixMask::scale(half[0], size, size);
      PixMask::scale(half[1], size, size);
      setStandardPic(half[0]);
      setStandardMask(half[1]);
    }
}

Glib::ustring Armyset::getConfigurationFile() const
{
  return getDirectory() + d_basename + file_extension;
}

std::list<Glib::ustring> Armyset::scanUserCollection()
{
  return File::scanForFiles(File::getUserArmysetDir(), file_extension);
}

std::list<Glib::ustring> Armyset::scanSystemCollection()
{
  std::list<Glib::ustring> retlist = File::scanForFiles(File::getArmysetDir(), 
                                                      file_extension);
  if (retlist.empty())
    {
      //note to translators: %1 is a file extension, %2 is a directory.
      std::cerr << String::ucompose(_("Couldn't find any armysets (*%1) in `%2'."),file_extension, File::getArmysetDir()) << std::endl;
      std::cerr << _("Please check the path settings in ~/.lordsawarrc") << std::endl;
      exit(-1);
    }

  return retlist;
}

void Armyset::switchArmysetForRuinKeeper(Army *army, const Armyset *armyset)
{
  //do our best to change the armyset for the given ruin keeper.
 
  //go find an equivalent type in the new armyset.
  Armyset *old_armyset
    = Armysetlist::getInstance()->getArmyset(army->getOwner()->getArmyset());
  ArmyProto *old_armyproto = old_armyset->lookupArmyByType(army->getTypeId());
  if (old_armyproto == NULL)
    return;
  const ArmyProto *new_armyproto = armyset->lookupArmyByType(army->getTypeId());

  //try looking at the same id first
  if (new_armyproto != NULL && 
      old_armyproto->getName() == new_armyproto->getName() &&
      old_armyproto->getDefendsRuins() == new_armyproto->getDefendsRuins())
    {
      army->morph(new_armyproto);
      return;
    }

  //try finding an army by the same name
  new_armyproto = armyset->lookupArmyByName(old_armyproto->getName());
  if (new_armyproto != NULL &&
      old_armyproto->getDefendsRuins() == new_armyproto->getDefendsRuins())
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any ruin keeper will do.
  new_armyproto = armyset->getRandomRuinKeeper();
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

}

void Armyset::switchArmyset(ArmyProdBase *army, const Armyset *armyset)
{
  //do our best to change the armyset for the given armyprodbase.

  //go find an equivalent type in the new armyset.
  Armyset *old_armyset
    = Armysetlist::getInstance()->getArmyset(army->getArmyset());
  ArmyProto *old_armyproto = old_armyset->lookupArmyByType(army->getTypeId());
  if (old_armyproto == NULL)
    return;
  ArmyProto *new_armyproto = armyset->lookupArmyByType(army->getTypeId());

  //try looking at the same id first
  if (new_armyproto != NULL && 
      old_armyproto->getName() == new_armyproto->getName())
    {
      army->morph(new_armyproto);
      return;
    }

  //try finding an army by the same name
  new_armyproto = armyset->lookupArmyByName(old_armyproto->getName());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with similar characteristics will do.
  new_armyproto = armyset->lookupSimilarArmy(old_armyproto);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength and turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(),
					  old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(), 0);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(0, old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army will do.
  new_armyproto = armyset->lookupArmyByGender(old_armyproto->getGender());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

}

void Armyset::switchArmyset(Army *army, const Armyset *armyset)
{
  //do our best to change the armyset for the given army.

  //go find an equivalent type in the new armyset.
  Armyset *old_armyset
    = Armysetlist::getInstance()->getArmyset(army->getOwner()->getArmyset());
  ArmyProto *old_armyproto = old_armyset->lookupArmyByType(army->getTypeId());
  if (!old_armyproto)
    return;
  ArmyProto *new_armyproto = armyset->lookupArmyByType(army->getTypeId());

  //try looking at the same id first
  if (new_armyproto != NULL && 
      old_armyproto->getId() == new_armyproto->getId())
    {
      army->morph(new_armyproto);
      return;
    }

  //try finding an army by the same name
  new_armyproto = armyset->lookupArmyByName(old_armyproto->getName());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, an army with the same gender (heroes).
  if (army->isHero() == true)
    {
      new_armyproto = armyset->lookupArmyByGender(old_armyproto->getGender());
      if (new_armyproto != NULL)
	{
	  army->morph(new_armyproto);
	  return;
	}
    }

  //failing that, any army with similar characteristics will do.
  new_armyproto = armyset->lookupSimilarArmy(old_armyproto);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength and turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(),
					  old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(), 0);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(0, old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army will do.
  new_armyproto = armyset->lookupArmyByGender(old_armyproto->getGender());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

}

const ArmyProto * Armyset::getRandomRuinKeeper() const
{
  // list all the army types that can be a sentinel.
  std::vector<const ArmyProto*> occupants;
  for (const_iterator i = begin(); i != end(); i++)
    {
      const ArmyProto *a = *i;
      if (a->getDefendsRuins())
	occupants.push_back(a);
    }
            
  if (!occupants.empty())
    return occupants[rand() % occupants.size()];

  return NULL;
}

const ArmyProto *Armyset::getRandomAwardableAlly() const
{
  // list all the army types that can be given out as a reward.
  std::vector<const ArmyProto*> allies;
  for (const_iterator i = begin(); i != end(); i++)
    {
      const ArmyProto *a = *i;
      if (a->getAwardable() == true)
	allies.push_back(a);
    }
            
  if (!allies.empty())
    return allies[rand() % allies.size()];

  return NULL;
}

void Armyset::reload(bool &broken)
{
  broken = false;
  bool unsupported = false;
  ArmysetLoader d(getConfigurationFile(), broken, unsupported);
  if (!broken && d.armyset && d.armyset->validate())
    {
      uninstantiateImages();
      for (iterator it = begin(); it != end(); it++)
        delete *it;
      clear();
      for (iterator it = d.armyset->begin(); it != d.armyset->end(); it++)
        push_back(new ArmyProto(*(*it)));
      instantiateImages(broken);
    }
}

Glib::ustring Armyset::getFileFromConfigurationFile(Glib::ustring file)
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      Glib::ustring filename = t.getFile(file, broken);
      t.Close();
  
      if (broken == false)
        return filename;
    }
  return "";
}

bool Armyset::replaceFileInConfigurationFile(Glib::ustring file, Glib::ustring new_file)
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

guint32 Armyset::calculate_preferred_tile_size() const
{
  guint32 tilesize = 0;
  std::map<guint32, guint32> sizecounts;

  if (d_ship)
    sizecounts[d_ship->get_unscaled_width()]++;
  if (d_standard)
    sizecounts[d_standard->get_unscaled_width()]++;
  if (d_bag)
    sizecounts[d_bag->get_unscaled_width()]++;
  for (const_iterator it = begin(); it != end(); it++)
    {
      ArmyProto *a = (*it);
      sizecounts[a->getImage(Shield::NEUTRAL)->get_unscaled_width()]++;
    }

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
    tilesize = DEFAULT_ARMY_TILE_SIZE;
  return tilesize;
}

bool Armyset::copy(Glib::ustring src, Glib::ustring dest)
{
  return Tar_Helper::copy(src, dest);
}

void Armyset::clean_tmp_dir() const
{
  return Tar_Helper::clean_tmp_dir(getConfigurationFile());
}

bool Armyset::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::ARMYSET, d_tag);
}

void Armyset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type(FileCompat::ARMYSET, file_extension, 
                                          d_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::ARMYSET, "0.2.1", "0.3.0",
     sigc::ptr_fun(&Armyset::upgrade));
}

Armyset * Armyset::copy(const Armyset *armyset)
{
  if (!armyset)
    return NULL;
  return new Armyset(*armyset);
}

guint32 Armyset::getMaxId() const
{
  guint32 max = 0;
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i)->getId() > max)
      max = (*i)->getId();
  return max;
}

bool weakest_quickest (const ArmyProto* first, const ArmyProto* second)
{
  int f = (first->getStrength() * 100) + (first->getProduction() * 101);
  int s = (second->getStrength() * 100) + (second->getProduction() * 101);
  if (f < s)
    return true;
  return false;
}

ArmyProto *Armyset::lookupWeakestQuickestArmy() const
{
  Armyset *a = new Armyset(*this);
  a->sort(weakest_quickest);
  guint32 type_id = (*(a->begin()))->getId();
  ArmyProto *p = Armysetlist::getInstance()->getArmy(getId(), type_id);
  delete a;
  return p;
}
