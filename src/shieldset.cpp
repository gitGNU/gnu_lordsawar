//  Copyright (C) 2008, 2009 Ben Asselstine
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
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "shieldset.h"
#include "shieldstyle.h"
#include "File.h"
#include "rgb_shift.h"

using namespace std;

std::string Shieldset::d_tag = "shieldset";
std::string Shieldset::file_extension = SHIELDSET_EXT;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Shieldset::Shieldset(XML_Helper *helper, std::string directory)
	: d_subdir("")
{
  setDirectory(directory);
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_copyright, "copyright");
  helper->getData(d_license, "license");
  helper->getData(d_small_width, "small_width");
  helper->getData(d_small_height, "small_height");
  helper->getData(d_medium_width, "medium_width");
  helper->getData(d_medium_height, "medium_height");
  helper->getData(d_large_width, "large_width");
  helper->getData(d_large_height, "large_height");
  helper->registerTag(Shield::d_tag, 
		      sigc::mem_fun((*this), &Shieldset::loadShield));
  helper->registerTag(ShieldStyle::d_tag, sigc::mem_fun((*this), 
							&Shieldset::loadShield));
}

Shieldset::~Shieldset()
{
  for (iterator it = begin(); it != end(); it++)
    delete *it;
  uninstantiateImages();
}

ShieldStyle * Shieldset::lookupShieldByTypeAndColour(guint32 type, guint32 owner)
{
  for (iterator it = begin(); it != end(); it++)
    {
      for (Shield::iterator i = (*it)->begin(); i != (*it)->end(); i++)
	{
	  if ((*i)->getType() == type && (*it)->getOwner() == owner)
	    return *i;
	}
    }
  return NULL;
}

Gdk::Color Shieldset::getColor(guint32 owner)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getColor();
    }
  return Gdk::Color("black");
}

struct rgb_shift Shieldset::getMaskColorShifts(guint32 owner)
{
  struct rgb_shift empty;
  empty.r = 0;
  empty.g = 0;
  empty.b = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getMaskColorShifts();
    }
  return empty;
}

bool Shieldset::loadShield(std::string tag, XML_Helper* helper)
{
  if (tag == Shield::d_tag)
    {
      Shield* sh = new Shield(helper);
      push_back(sh);
      return true;
    }
  if (tag == ShieldStyle::d_tag)
    {
      ShieldStyle *sh = new ShieldStyle(helper);
      (*back()).push_back(sh);
      return true;
    }
  return false;
}


class ShieldsetLoader
{
public:
    ShieldsetLoader(std::string filename)
      {
	shieldset = NULL;
	dir = File::get_dirname(filename);
	if (File::nameEndsWith(filename, Shieldset::file_extension) == false)
	  filename += Shieldset::file_extension;
	XML_Helper helper(filename, ios::in, false);
	helper.registerTag(Shieldset::d_tag, sigc::mem_fun((*this), &ShieldsetLoader::load));
	if (!helper.parse())
	  {
	    std::cerr << "Error, while loading an shieldset. Shieldset Name: ";
	    std::cerr <<File::get_basename(File::get_dirname(filename))<<
	      std::endl <<std::flush;
	  }
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Shieldset::d_tag)
	  {
	    shieldset = new Shieldset(helper, dir);
	    return true;
	  }
	return false;
      };
    std::string dir;
    Shieldset *shieldset;
};
Shieldset *Shieldset::create(std::string filename)
{
  ShieldsetLoader d(filename);
  return d.shieldset;
}
void Shieldset::getFilenames(std::list<std::string> &files)
{
  for (iterator it = begin(); it != end(); it++)
    for (Shield::iterator i = (*it)->begin(); i != (*it)->end(); i++)
      {
	std::string file = (*i)->getImageName();
	if (std::find(files.begin(), files.end(), file) == files.end())
	  files.push_back(file);
      }
}
bool Shieldset::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("copyright", d_copyright);
  retval &= helper->saveData("license", d_license);
  retval &= helper->saveData("small_width", d_small_width);
  retval &= helper->saveData("small_height", d_small_height);
  retval &= helper->saveData("medium_width", d_medium_width);
  retval &= helper->saveData("medium_height", d_medium_height);
  retval &= helper->saveData("large_width", d_large_width);
  retval &= helper->saveData("large_height", d_large_height);
  for (iterator it = begin(); it != end(); it++)
    retval &= (*it)->save(helper);
  retval &= helper->closeTag();
  return retval;
}

void Shieldset::instantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages(this);
}

void Shieldset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

std::string Shieldset::getConfigurationFile()
{
  return getDirectory() + d_subdir + file_extension;
}

std::list<std::string> Shieldset::scanUserCollection()
{
  return File::scanFiles(File::getUserShieldsetDir(), file_extension);
}

std::list<std::string> Shieldset::scanSystemCollection()
{
  std::list<std::string> retlist = File::scanFiles(File::getShieldsetDir(), 
						   file_extension);
  if (retlist.empty())
    {
      std::cerr << "Couldn't find any shieldsets!" << std::endl;
      std::cerr << "Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc" << std::endl;
      std::cerr << "Exiting!" << std::endl;
      exit(-1);
    }

  return retlist;
}

//End of file
