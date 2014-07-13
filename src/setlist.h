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

#ifndef SETLIST_H
#define SETLIST_H
#include <iostream>
#include "File.h"
#include "ucompose.hpp"
#include "defs.h"

template<class T>
class SetList: public std::list<T*>
{
public:
    SetList(Glib::ustring ext){extension=ext;};
    ~SetList(){};
    static Glib::ustring getConfigurationFilename(Glib::ustring dir, Glib::ustring subdir, Glib::ustring ext) {return File::add_slash_if_necessary(dir) + subdir + "/" + subdir + ext;};
    static std::list<Glib::ustring> scan(Glib::ustring extension, bool system = true)
      {
        if (system == false)
          return File::scanForFiles(File::getSetDir(extension, false), extension);
        else
          {
            std::list<Glib::ustring> retlist = 
              File::scanForFiles(File::getSetDir(extension), extension);
            if (retlist.empty())
              {
                //note to translators: %1 is a file extension, %2 is a directory.
                std::cerr << String::ucompose(_("Couldn't find any *%1 files in `%2'."),extension, File::getSetDir(extension)) << std::endl;
                std::cerr << _("Please check the path settings in ~/.lordsawarrc") << std::endl;
                exit(-1);
              }
            return retlist;
          }

      }
    bool contains(Glib::ustring name) const
      {
        for (class SetList<T>::const_iterator it = this->begin(); 
             it != this->end(); it++)
          if ((*it)->getName() == name)
            return true;

        return false;
      }
protected: 
    Glib::ustring extension;
};

#endif
