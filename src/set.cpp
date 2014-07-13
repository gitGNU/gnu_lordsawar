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

Set::Set(Glib::ustring ext, guint32 id, Glib::ustring name)
  : origin(SYSTEM), dir(""), d_id(id), d_name(name), d_license(""), 
    d_basename(""), d_info(""), extension(ext)
{
}

Set::Set(const Set &s)
  : origin(s.origin), dir(s.dir), d_id(s.d_id), d_name(s.d_name), 
    d_license(s.d_license), d_basename(s.d_basename), d_info(s.d_info),
    extension(s.extension)
{
}

Glib::ustring Set::getFile(Glib::ustring file) const
{
  return getDirectory() + file + ".png";
}

Set::Set(Glib::ustring ext, XML_Helper* helper)
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
