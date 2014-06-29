//  Copyright (C) 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include <string.h>
#include "shieldset.h"
#include "shieldstyle.h"
#include "File.h"
#include "Configuration.h"
#include "tarhelper.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "xmlhelper.h"

Glib::ustring Shieldset::d_tag = "shieldset";
Glib::ustring Shieldset::file_extension = SHIELDSET_EXT;

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

Shieldset::Shieldset(guint32 id, Glib::ustring name)
 : d_id(id), d_name(name), d_copyright(""), d_license(""), d_info(""), 
    d_basename(""), d_small_height(0), d_small_width(0), d_medium_height(0), 
    d_medium_width(0), d_large_height(0), d_large_width(0)
{
}

Shieldset::Shieldset(const Shieldset& s)
  :Set(s), d_id(s.d_id), d_name(s.d_name), d_copyright(s.d_copyright), 
    d_license(s.d_license), d_info(s.d_info), d_basename(s.d_basename),
    d_small_height(s.d_small_height), d_small_width(s.d_small_width),
    d_medium_height(s.d_medium_height), d_medium_width(s.d_medium_width),
    d_large_height(s.d_large_height), d_large_width(s.d_large_width)
{
  for (const_iterator it = s.begin(); it != s.end(); it++)
    push_back(new Shield(*(*it)));
}

Shieldset::Shieldset(XML_Helper *helper, Glib::ustring directory)
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
  clean_tmp_dir();
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

Gdk::RGBA Shieldset::getColor(guint32 owner) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getColor();
    }
  return Gdk::RGBA("black");
}

bool Shieldset::loadShield(Glib::ustring tag, XML_Helper* helper)
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
    ShieldsetLoader(Glib::ustring filename, bool &broken, bool &unsupported)
      {
        unsupported_version = false;
	shieldset = NULL;
	dir = File::get_dirname(filename);
        file = File::get_basename(filename);
	if (File::nameEndsWith(filename, Shieldset::file_extension) == false)
	  filename += Shieldset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        Glib::ustring lwsfilename = 
          t.getFirstFile(Shieldset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwsfilename, std::ios::in);
	helper.registerTag(Shieldset::d_tag, sigc::mem_fun((*this), &ShieldsetLoader::load));
	if (!helper.parseXML())
	  {
            unsupported = unsupported_version;
            std::cerr << String::ucompose(_("Error!  can't load shieldet `%1'."), filename) << std::endl;
	    if (shieldset != NULL)
	      delete shieldset;
	    shieldset = NULL;
	  }
        File::erase(lwsfilename);
        helper.close();
        t.Close();
      };
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Shieldset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_SHIELDSET_VERSION)
              {
                shieldset = new Shieldset(helper, dir);
                shieldset->setBaseName(file);
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
    Shieldset *shieldset;
    bool unsupported_version;
};

Shieldset *Shieldset::create(Glib::ustring filename, bool &unsupported_version)
{
  bool broken = false;
  ShieldsetLoader d(filename, broken, unsupported_version);
  if (broken)
    return NULL;
  return d.shieldset;
}

void Shieldset::getFilenames(std::list<Glib::ustring> &files) const
{
  for (const_iterator it = begin(); it != end(); it++)
    for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); i++)
      {
	Glib::ustring file = (*i)->getImageName();
	if (std::find(files.begin(), files.end(), file) == files.end())
	  files.push_back(file);
      }
}

bool Shieldset::save(Glib::ustring filename, Glib::ustring extension) const
{
  bool broken = false;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, extension);
  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  helper.begin(LORDSAWAR_SHIELDSET_VERSION);
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
  for (std::list<Glib::ustring>::iterator it = delfiles.begin(); it != delfiles.end(); it++)
    File::erase(*it);
  File::erase(tmpfile);
  if (broken == false)
    {
      if (File::copy(tmptar, goodfilename) == true)
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

void Shieldset::instantiateImages(bool &broken)
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages(this, broken);
}

void Shieldset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

Glib::ustring Shieldset::getConfigurationFile() const
{
  return getDirectory() + d_basename + file_extension;
}

std::list<Glib::ustring> Shieldset::scanUserCollection()
{
  return File::scanForFiles(File::getUserShieldsetDir(), file_extension);
}

std::list<Glib::ustring> Shieldset::scanSystemCollection()
{
  std::list<Glib::ustring> retlist = File::scanForFiles(File::getShieldsetDir(), 
                                                      file_extension);
  if (retlist.empty())
    {
      //note to translators: %1 is a file extension, %2 is a directory.
      std::cerr << String::ucompose(_("Couldn't find any shieldsets (*%1) in `%2'."),file_extension, File::getShieldsetDir()) << std::endl;
      std::cerr << _("Please check the path settings in ~/.lordsawarrc") << std::endl;
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
  if (d_small_width == 0 || d_small_height == 0)
    return false;
  if (d_medium_width == 0 || d_medium_height == 0)
    return false;
  if (d_large_width == 0 || d_large_height == 0)
    return false;
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

void Shieldset::reload(bool &broken)
{
  broken = false;
  bool unsupported_version = false;
  ShieldsetLoader d(getConfigurationFile(), broken, unsupported_version);
  if (broken == false && d.shieldset && d.shieldset->validate())
    {
      //steal the values from d.shieldset and then don't delete it.
      uninstantiateImages();
      for (iterator it = begin(); it != end(); it++)
        delete *it;
      Glib::ustring basename = d_basename;
      *this = *d.shieldset;
      instantiateImages(broken);
      d_basename = basename;
    }
}

Glib::ustring Shieldset::getFileFromConfigurationFile(Glib::ustring file)
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

bool Shieldset::replaceFileInConfigurationFile(Glib::ustring file, Glib::ustring new_file)
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      broken = !t.replaceFile(file, new_file);
      t.Close();
    }
  return !broken;
}

bool Shieldset::copy(Glib::ustring src, Glib::ustring dest)
{
  return Tar_Helper::copy(src, dest);
}

guint32 Shieldset::countEmptyImageNames() const
{
  guint32 count = 0;
  for (Shieldset::const_iterator i = begin(); i != end(); i++)
    {
      for (std::list<ShieldStyle*>::const_iterator j = (*i)->begin(); j != (*i)->end(); j++)
        {
          if ((*j)->getImageName().empty() == true)
            count++;
        }
    }
  return count;
}

void Shieldset::clean_tmp_dir() const
{
  return Tar_Helper::clean_tmp_dir(getConfigurationFile());
}

bool Shieldset::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::SHIELDSET, d_tag);
}

void Shieldset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type(FileCompat::SHIELDSET, 
                                          file_extension, d_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::SHIELDSET, "0.2.0", LORDSAWAR_SHIELDSET_VERSION,
     sigc::ptr_fun(&Shieldset::upgrade));
}

Shieldset* Shieldset::copy(const Shieldset *shieldset)
{
  if (!shieldset)
    return NULL;
  return new Shieldset(*shieldset);
}

void Shieldset::setHeightsAndWidthsFromImages()
{
  d_small_width = 0;
  d_small_height = 0;
  d_medium_width = 0;
  d_medium_height = 0;
  d_large_width = 0;
  d_large_height = 0;
  for (iterator it = begin(); it != end(); it++)
    for (Shield::iterator i = (*it)->begin(); i != (*it)->end(); i++)
      {
        PixMask *image = (*i)->getImage();
        if (image == NULL)
          continue;
        guint32 height = 0;
        if (image->get_height() > 0)
          height = image->get_unscaled_height();
        guint32 width = 0;
        if (image->get_unscaled_width() > 0)
          width = image->get_width();
        if ((*i)->getType() == ShieldStyle::SMALL)
          {
            if (width > d_small_width)
              d_small_width = width;
            if (height > d_small_height)
              d_small_height = height;
          }
        else if ((*i)->getType() == ShieldStyle::MEDIUM)
          {
            if (width > d_medium_width)
              d_medium_width = width;
            if (height > d_medium_height)
              d_medium_height = height;
          }
        else if ((*i)->getType() == ShieldStyle::LARGE)
          {
            if (width > d_large_width)
              d_large_width = width;
            if (height > d_large_height)
              d_large_height = height;
          }
      }
}
//End of file
