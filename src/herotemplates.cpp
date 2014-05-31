//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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

#include <sstream>

#include "herotemplates.h"

#include "File.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "hero.h"
#include "heroproto.h"
#include "xmlhelper.h"
#include "ucompose.hpp"

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

HeroProto *HeroTemplates::getRandomHero(Hero::Gender gender, int player_id)
{
  std::vector<HeroProto*> heroes;
  for (unsigned int i = 0; i < d_herotemplates[player_id].size(); i++)
    {
      if (Hero::Gender(d_herotemplates[player_id][i]->getGender()) == gender)
	heroes.push_back (d_herotemplates[player_id][i]);
    }
  if (heroes.size() == 0)
    return getRandomHero(player_id);

  int num = rand() % heroes.size();
  return heroes[num];
}

HeroProto *HeroTemplates::getRandomHero(int player_id)
{
  int num = rand() % d_herotemplates[player_id].size();
  return d_herotemplates[player_id][num];
}

int HeroTemplates::loadHeroTemplates()
{
  const Armysetlist* al = Armysetlist::getInstance();

  d_male_heroes.clear();
  d_female_heroes.clear();

  // list all the army types that are heroes.
  Player *p = Playerlist::getInstance()->getNeutral();
  Armyset *as = al->getArmyset(p->getArmyset());
  for (Armyset::iterator j = as->begin(); j != as->end(); ++j)
    {
      const ArmyProto *a = al->getArmy (p->getArmyset(), (*j)->getId());
      if (a->isHero())
	{
	  if (a->getGender() == Hero::FEMALE)
	    d_female_heroes.push_back(a);
	  else
	    d_male_heroes.push_back(a);
	}
    }
  if (d_female_heroes.size() == 0 && d_male_heroes.size() > 0)
    {
      //add a female hero if there isn't one in the armyset.
      ArmyProto *female_hero = new ArmyProto(*(*d_male_heroes.begin()));
      female_hero->setGender(Hero::FEMALE);
      d_female_heroes.push_back(female_hero);
    }

  XML_Helper helper(File::getMiscFile("heronames.xml"), std::ios::in);

  helper.registerTag("herotemplate", sigc::mem_fun((*this), &HeroTemplates::load));

  if (!helper.parse())
    {
      std::cerr << String::ucompose(_("Error!  can't load heronames file `%1'.  Exiting."), File::getMiscFile("heronames.xml")) << std::endl;
      exit(-1);
    }

  helper.close();
  return 0;
}

      
bool HeroTemplates::load(Glib::ustring tag, XML_Helper *helper)
{
  if (tag == "herotemplate")
    {
      Glib::ustring name;
      helper->getData(name, "name");
      guint32 owner;
      helper->getData(owner, "owner");
      Glib::ustring gender_str;
      if (owner >= (int) MAX_PLAYERS)
	return false;
      helper->getData(gender_str, "gender");
      Hero::Gender gender;
      gender = Hero::genderFromString(gender_str);

      const ArmyProto *herotype = NULL;
      if (gender == Hero::MALE)
	{
	  if (d_male_heroes.size() > 0)
	    herotype = d_male_heroes[rand() % d_male_heroes.size()];
	}
      else if (gender == Hero::FEMALE)
	{
	  if (d_female_heroes.size() > 0)
	    herotype = d_female_heroes[rand() % d_female_heroes.size()];
	}
      if (herotype == NULL)
	{
	  if (d_male_heroes.size() > 0)
	    herotype = d_male_heroes[rand() % d_male_heroes.size()];
	  else if (d_female_heroes.size() > 0)
	    herotype = d_female_heroes[rand() % d_female_heroes.size()];
	}
      if (herotype == NULL)
	return false;
      HeroProto *newhero = new HeroProto (*herotype);
      newhero->setOwnerId(owner);

      newhero->setName (_(name.c_str()));
      d_herotemplates[owner].push_back (newhero);
    }
  return true;
}

