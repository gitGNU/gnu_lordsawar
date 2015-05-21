//  Copyright (C) 2015 Ben Asselstine
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

#include "builder-cache.h"
#include "File.h"

BuilderCache* BuilderCache::s_instance = 0;

BuilderCache* BuilderCache::getInstance()
{
    if (s_instance == 0)
        s_instance = new BuilderCache();

    return s_instance;
}

void BuilderCache::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

void BuilderCache::preloadAllBuilders()
{
  std::list<Glib::ustring> f = 
    File::scanForFiles(File::getMiscFile("glade/"), ".ui");
  for (std::list<Glib::ustring>::iterator i = f.begin(); i != f.end(); i++)
    {
      if (File::get_basename ((*i)) == "about-dialog")
        continue;
      Glib::RefPtr<Gtk::Builder> xml = Gtk::Builder::create_from_file(*i);
      (*this)[File::get_basename(*i, true)] = xml;
      printf("loaded %s\n", (*i).c_str());
    }
}

BuilderCache::BuilderCache()
{
  preloadAllBuilders();
}

BuilderCache::~BuilderCache()
{
}

Glib::RefPtr<Gtk::Builder> BuilderCache::get(Glib::ustring f)
{
  BuilderCache *b = getInstance();
  std::map<Glib::ustring,Glib::RefPtr<Gtk::Builder> >::iterator it = b->find(f);
  if (it == b->end())
    printf("couldn't find builder for '%s'\n", f.c_str());
  else
    printf("getting builder for '%s'\n", f.c_str());
  return (*getInstance())[f];
}
// End of file
