// Copyright (C) 2009, 2014 Ben Asselstine
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

#include "set.h"
#include "tarhelper.h"

Set::Set(Glib::ustring ext, guint32 id, Glib::ustring name, guint32 ts)
  : origin(SYSTEM), dir(""), d_id(id), d_name(name), d_license(""), 
    d_basename(""), d_info(""), extension(ext), d_tileSize(ts), d_scale (1.0)
{
}

Set::Set(const Set &s)
  : origin(s.origin), dir(s.dir), d_id(s.d_id), d_name(s.d_name), 
    d_license(s.d_license), d_basename(s.d_basename), d_info(s.d_info),
    extension(s.extension), d_tileSize(s.d_tileSize), d_scale(s.d_scale)
{
}

Glib::ustring Set::getFile(Glib::ustring file) const
{
  return getDirectory() + file + ".png";
}

Set::Set(Glib::ustring ext, XML_Helper* helper)
 :d_scale(1.0)
{
  extension = ext;
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_copyright, "copyright");
  helper->getData(d_license, "license");
  helper->getData(d_info, "info");
}

bool Set::save(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("copyright", d_copyright);
  retval &= helper->saveData("license", d_license);
  retval &= helper->saveData("info", d_info);
  return retval;
}

Glib::ustring Set::getConfigurationFile() const
{
  return getDirectory() + getBaseName() + extension;
}

Glib::ustring Set::getFileFromConfigurationFile(Glib::ustring file)
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      Glib::ustring filename = t.getFile(file, broken);
      t.Close(false);
  
      if (broken == false)
        return filename;
    }
  return "";
}

bool Set::replaceFileInConfigurationFile(Glib::ustring file, Glib::ustring new_file)
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

bool Set::addFileInConfigurationFile(Glib::ustring new_file)
{
  return replaceFileInConfigurationFile("", new_file);
}

void Set::clean_tmp_dir() const
{
  return Tar_Helper::clean_tmp_dir(getConfigurationFile());
}

bool Set::saveTar(Glib::ustring tmpfile, Glib::ustring tmptar, Glib::ustring dest) const
{
  bool broken = false;
  Tar_Helper t(tmptar, std::ios::out, broken);
  if (broken == true)
    return false;
  t.saveFile(tmpfile, File::get_basename(dest, true));
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
      if (File::copy(tmptar, dest) == true)
        File::erase(tmptar);
    }
  return broken;
}
