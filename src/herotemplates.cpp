//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
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

#include "herotemplates.h"

#include "File.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "hero.h"
#include "heroproto.h"
#include "xmlhelper.h"
#include <sstream>

HeroTemplates* HeroTemplates::d_instance = 0;

HeroTemplates* HeroTemplates::getInstance()
{
    if (!d_instance)
        d_instance = new HeroTemplates();

    return d_instance;
}

void HeroTemplates::deleteInstance()
{
    if (d_instance != 0)
        delete d_instance;

    d_instance = 0;
}


HeroTemplates::HeroTemplates()
{
  loadHeroTemplates();
}

HeroTemplates::~HeroTemplates()
{
  for (unsigned int i = 0; i < MAX_PLAYERS; ++i)
    for (std::vector<HeroProto *>::iterator j = d_herotemplates[i].begin();
           j != d_herotemplates[i].end(); ++j)
      delete *j;
}

HeroProto *HeroTemplates::getRandomHero(int player_id)
{
  int num = rand() % d_herotemplates[player_id].size();
  return d_herotemplates[player_id][num];
}

int HeroTemplates::loadHeroTemplates()
{
  const Armysetlist* al = Armysetlist::getInstance();

  d_heroes.clear();

  // list all the army types that are heroes.
  Player *p = Playerlist::getInstance()->getNeutral();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const ArmyProto *a = al->getArmy (p->getArmyset(), j);
      if (a->isHero())
	d_heroes.push_back(a);
    }
  XML_Helper helper(File::getMiscFile("heronames.xml"), std::ios::in, false);

  helper.registerTag("herotemplate", sigc::mem_fun((*this), &HeroTemplates::load));

  if (!helper.parse())
    {
      std::cerr << "Error, while loading a template from heronames.xml" <<std::endl <<std::flush;
      exit(-1);
    }

  helper.close();
  return 0;
}

bool HeroTemplates::load(std::string tag, XML_Helper *helper)
{
  if (tag == "herotemplate")
    {
      std::string name;
      helper->getData(name, "name");
      guint32 owner;
      helper->getData(owner, "owner");
      std::string gender_str;
      if (owner >= (int) MAX_PLAYERS)
	return false;
      helper->getData(gender_str, "gender");
      Hero::Gender gender;
      gender = Hero::genderFromString(gender_str);

      const ArmyProto *herotype = d_heroes[rand() % d_heroes.size()];
      HeroProto *newhero = new HeroProto (*herotype);
      newhero->setGender(gender);
      newhero->setGender(Hero::FEMALE);

      newhero->setName (_(name.c_str()));
      d_herotemplates[owner].push_back (newhero);
    }
  return true;
}

