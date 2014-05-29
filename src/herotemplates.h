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

#ifndef HERO_TEMPLATES_H
#define HERO_TEMPLATES_H

#include <vector>
#include "hero.h"
#include "defs.h"

class ArmyProto;
class HeroProto;
class XML_Helper;

//! A list of Item objects.
/** 
 * The HeroTemplates holds all hero templates together.
 * 
 * It is implemented as a singleton. Upon creation, it reads the hero
 * description file and initialises an internal list.
 */
class HeroTemplates
{
    public:
        //! Returns the singleton instance.
	static HeroTemplates* getInstance();

        //! Explicitely deletes the singleton instance.
        static void deleteInstance();

        HeroProto *getRandomHero(int player_id);

	HeroProto *getRandomHero(Hero::Gender gender, int player_id);
        
    protected:
	//! Default constructor. The function reads in the heronames file and produces a set of hero templates to be randomly selected from.
	HeroTemplates();
	//! Destructor.
        ~HeroTemplates();

	bool load(Glib::ustring tag, XML_Helper *helper);

    private:
        /* the contents of the heronames data file */
        std::vector<HeroProto*> d_herotemplates[MAX_PLAYERS];

	//a list of male hero prototypes contained in the the army set.
	std::vector<const ArmyProto*> d_male_heroes;
	//a list of female hero prototypes contained in the the army set.
	std::vector<const ArmyProto*> d_female_heroes;

        static HeroTemplates* d_instance;

        int loadHeroTemplates();
};

#endif
