//  Copyright (C) 2008, 2009, 2010 Ben Asselstine
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

#include <string.h>
#include "shieldset.h"
#include "shieldstyle.h"
#include "File.h"
#include "Configuration.h"
#include "tarhelper.h"

using namespace std;

std::string Shieldset::d_tag = "shieldset";
std::string Shieldset::file_extension = SHIELDSET_EXT;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Shieldset::Shieldset(guint32 id, std::string name)
	: d_id(id), d_name(name), d_copyright(""), d_license(""), d_info(""), 
	d_basename("")
{
}

Shieldset::Shieldset(std::string filename, bool &broken)
{
  *this = *Shieldset::create(filename);
}

Shieldset::Shieldset(XML_Helper *helper, std::string directory)
	: d_basename("")
{
  setDirectory(directory);
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_copyright, "copyright");
  helper->getData(d_license, "license");
  helper->getData(d_info, "info");
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
  clear();
}

Shieldset::~Shieldset()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); it++)
    delete *it;
}

ShieldStyle * Shieldset::lookupShieldByTypeAndColour(guint32 type, guint32 colour) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); i++)
	{
	  if ((*i)->getType() == type && (*it)->getOwner() == colour)
	    return *i;
	}
    }
  return NULL;
}

Gdk::Color Shieldset::getColor(guint32 owner) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getColor();
    }
  return Gdk::Color("black");
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
    ShieldsetLoader(std::string filename, bool &broken)
      {
	shieldset = NULL;
	dir = File::get_dirname(filename);
        file = File::get_basename(filename);
	if (File::nameEndsWith(filename, Shieldset::file_extension) == false)
	  filename += Shieldset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        std::string lwsfilename = 
          t.getFirstFile(Shieldset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwsfilename, ios::in, false);
	helper.registerTag(Shieldset::d_tag, sigc::mem_fun((*this), &ShieldsetLoader::load));
	if (!helper.parse())
	  {
	    std::cerr << "Error, while loading an shieldset. Shieldset File: ";
	    std::cerr << filename << std::endl <<std::flush;
	    if (shieldset != NULL)
	      delete shieldset;
	    shieldset = NULL;
	  }
        File::erase(lwsfilename);
        helper.close();
        t.Close();
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Shieldset::d_tag)
	  {
	    shieldset = new Shieldset(helper, dir);
            shieldset->setBaseName(file);
	    return true;
	  }
	return false;
      };
    std::string dir;
    std::string file;
    Shieldset *shieldset;
};

Shieldset *Shieldset::create(std::string filename)
{
  bool broken = false;
  ShieldsetLoader d(filename, broken);
  if (broken)
    return NULL;
  return d.shieldset;
}

void Shieldset::getFilenames(std::list<std::string> &files) const
{
  for (const_iterator it = begin(); it != end(); it++)
    for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); i++)
      {
	std::string file = (*i)->getImageName();
	if (std::find(files.begin(), files.end(), file) == files.end())
	  files.push_back(file);
      }
}

bool Shieldset::save(std::string filename, std::string extension) const
{
  bool broken = false;
  std::string goodfilename = filename;
  if (File::nameEndsWith(filename, extension) == false)
    goodfilename += "." + extension;
  std::string tmpfile = "lw.XXXX";
  int fd = Glib::file_open_tmp(tmpfile, "lw.XXXX");
  close (fd);
  XML_Helper helper(tmpfile, std::ios::out, Configuration::s_zipfiles);
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

bool Shieldset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("copyright", d_copyright);
  retval &= helper->saveData("license", d_license);
  retval &= helper->saveData("info", d_info);
  retval &= helper->saveData("small_width", d_small_width);
  retval &= helper->saveData("small_height", d_small_height);
  retval &= helper->saveData("medium_width", d_medium_width);
  retval &= helper->saveData("medium_height", d_medium_height);
  retval &= helper->saveData("large_width", d_large_width);
  retval &= helper->saveData("large_height", d_large_height);
  for (const_iterator it = begin(); it != end(); it++)
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

std::string Shieldset::getConfigurationFile() const
{
  return getDirectory() + d_basename + file_extension;
}

std::list<std::string> Shieldset::scanUserCollection()
{
  return File::scanForFiles(File::getUserShieldsetDir(), file_extension);
}

std::list<std::string> Shieldset::scanSystemCollection()
{
  std::list<std::string> retlist = File::scanForFiles(File::getShieldsetDir(), 
                                                      file_extension);
  if (retlist.empty())
    {
      std::cerr << "Couldn't find any shieldsets (*" << file_extension << 
        ") in : " << File::getShieldsetDir() << std::endl;
      std::cerr << "Please check the path settings in ~/.lordsawarrc" << std::endl;
      std::cerr << "Exiting!" << std::endl;
      exit(-1);
    }
  return retlist;
}

	
bool Shieldset::validate() const
{
  bool valid = true;
  if (validateNumberOfShields() == false)
    return false;
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      if (validateShieldImages(Shield::Colour(i)) == false)
	return false;
    }
  return valid;
}

bool Shieldset::validateNumberOfShields() const
{
  int players[MAX_PLAYERS + 1][3];
  memset(players, 0, sizeof(players));
  //need at least 3 complete player shields, one of which must be neutral.
  for (const_iterator it = begin(); it != end(); it++)
    {
      for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); i++)
	{
	  int idx = 0;
	  switch ((*i)->getType())
	    {
	    case ShieldStyle::SMALL: idx = 0; break;
	    case ShieldStyle::MEDIUM: idx = 1; break;
	    case ShieldStyle::LARGE: idx = 2; break;
	    }
	  players[(*it)->getOwner()][idx]++;
	}
    }
  int count = 0;
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (players[i][0] > 0 && players[i][1] > 0 && players[i][2] > 0)
	count++;
    }
  if (count <= 2)
    return false;
  if (players[MAX_PLAYERS][0] == 0 || players[MAX_PLAYERS][1] == 0 || players[MAX_PLAYERS][2] == 0)
    return false;
  return true;
}

bool Shieldset::validateShieldImages(Shield::Colour c) const
{
  //if we have a shield, it should have all 3 sizes.
  int player[3];
  memset(player, 0, sizeof(player));
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getOwner() != guint32(c))
	continue;
      for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); i++)
	{
	  int idx = 0;
	  switch ((*i)->getType())
	    {
	    case ShieldStyle::SMALL: idx = 0; break;
	    case ShieldStyle::MEDIUM: idx = 1; break;
	    case ShieldStyle::LARGE: idx = 2; break;
	    }
	  if ((*i)->getImageName().empty() == false)
	    player[idx]++;
	}
    }
  int count = player[0] + player[1] + player[2];
  if (count <= 2)
    return false;
  return true;
}

void Shieldset::reload()
{
  bool broken = false;
  ShieldsetLoader d(getConfigurationFile(), broken);
  if (broken == false && d.shieldset && d.shieldset->validate())
    {
      //steal the values from d.shieldset and then don't delete it.
      uninstantiateImages();
      for (iterator it = begin(); it != end(); it++)
        delete *it;
      std::string basename = d_basename;
      *this = *d.shieldset;
      instantiateImages();
      d_basename = basename;
    }
}

std::string Shieldset::getFileFromConfigurationFile(std::string file)
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

bool Shieldset::replaceFileInConfigurationFile(std::string file, std::string new_file)
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

bool Shieldset::copy(std::string src, std::string dest)
{
  return Tar_Helper::copy(src, dest);
}
//End of file
