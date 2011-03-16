// Copyright (C) 2011 Ben Asselstine
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

#include <limits.h>
#include <fstream>
#include <iostream>
#include "xmlhelper.h"
#include "Configuration.h"
#include <sigc++/functors/mem_fun.h>
#include "defs.h"
#include "File.h"
#include "profile.h"
#include "profilelist.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

std::string Profilelist::d_tag = "profilelist";

Profilelist* Profilelist::s_instance = 0;

Profilelist* Profilelist::getInstance()
{
  if (s_instance == 0)
    {
      s_instance = new Profilelist();
      s_instance->load();
    }

  return s_instance;
}

bool Profilelist::save() const
{
  return saveToFile(File::getSavePath() + "/" + PROFILE_LIST);
}

bool Profilelist::saveToFile(std::string filename) const
{
  bool retval = true;
  XML_Helper helper(filename, std::ios::out, false);
  retval &= save(&helper);
  helper.close();
  return retval;
}

bool Profilelist::load()
{
  return loadFromFile(File::getSavePath() + "/" + PROFILE_LIST);
}

bool Profilelist::loadFromFile(std::string filename)
{
  std::ifstream in(filename.c_str());
  if (in)
    {
      XML_Helper helper(filename.c_str(), std::ios::in, false);
      helper.registerTag(Profile::d_tag, 
                         sigc::mem_fun(this, &Profilelist::load_tag));
      bool retval = helper.parse();
      if (retval == false)
	unlink(filename.c_str());
      return retval;
    }
  return true;
}

Profilelist* Profilelist::getInstance(XML_Helper* helper)
{
  if (s_instance)
    deleteInstance();

  s_instance = new Profilelist(helper);
  return s_instance;
}

void Profilelist::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

Profilelist::Profilelist()
{
}

Profilelist::Profilelist(XML_Helper* helper)
{
  helper->registerTag(Profile::d_tag, 
                      sigc::mem_fun(this, &Profilelist::load_tag));
}

Profilelist::~Profilelist()
{
  for (Profilelist::iterator it = begin(); it != end(); it++)
    delete *it;
}

bool Profilelist::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->begin(LORDSAWAR_PROFILES_VERSION);
  retval &= helper->openTag(Profilelist::d_tag);

  for (const_iterator it = begin(); it != end(); it++)
    (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool Profilelist::load_tag(std::string tag, XML_Helper* helper)
{
  if (helper->getVersion() != LORDSAWAR_PROFILES_VERSION)
    {
      return false;
    }
  if (tag == Profile::d_tag)
    {
      Profile *p = Profile::handle_load(helper);
      push_back(p);
      return true;
    }
  return false;
}

Profile *Profilelist::findLastPlayedProfileForUser(Glib::ustring user) const
{
  Profile *p = NULL;
  Glib::TimeVal latest = Glib::TimeVal(0,0);
  for (Profilelist::const_iterator i = begin(); i != end(); i++)
    {
      if ((*i)->getUserName() == user)
        {
          if ((*i)->getLastPlayedOn() > latest)
            {
              p = (*i);
              latest = (*i)->getLastPlayedOn();
            }
        }
    }
  return p;
}
        
Profile *Profilelist::findProfileById(std::string id) const
{
  for (Profilelist::const_iterator i = begin(); i != end(); i++)
    {
      if ((*i)->getId() == id)
        return *i;
    }
  return NULL;
}

bool Profilelist::removeOldVersionsOfFile()
{
  bool removed = false;
  bool broken = false;
  std::string version = "";
  std::string filename = File::getSavePath() + "/" + PROFILE_LIST;
  VersionLoader l(filename, d_tag, version, broken);
  if (broken == false && version != "" && version != LORDSAWAR_PROFILES_VERSION)
    {
      File::erase(filename);
      removed = true;
    }
  return removed;
}
// End of file
