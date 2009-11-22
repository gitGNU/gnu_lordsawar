//  Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef CREATE_SCENARIO_RANDOMIZE_H
#define CREATE_SCENARIO_RANDOMIZE_H

#include <fstream>
#include <string>
#include <vector>
#include <gtkmm.h>
#include "game-parameters.h"
#include "namelist.h"
#include "shield.h"

class Signpost;
class Army;
class Player;
class Reward;

class CreateScenarioRandomize
{
    public:
        CreateScenarioRandomize();
        virtual ~CreateScenarioRandomize();

	/** take a random city name
	 */
	std::string popRandomCityName();

	/* give a random city name
	 */
	void pushRandomCityName(std::string name);
	std::string popRandomRuinName();
	void pushRandomRuinName(std::string name);
	std::string popRandomTempleName();
	void pushRandomTempleName(std::string name);
	std::string popRandomSignpost();
	void pushRandomSignpost(std::string name);
	guint32 getRandomCityIncome(bool capital = false);
	bool randomSignpostsEmpty() {return d_signposts->empty();}
	std::string getDynamicSignpost(Signpost *signpost);
	int getNumSignposts() {return d_signposts->size();}
	Army * getRandomRuinKeeper(Player *p);
	Reward *getNewRandomReward(bool hidden_ruins);


	//! Based on the difficulty, get how much gold each player should
	//start with.
	void getBaseGold (int difficulty, int *base_gold);

	int adjustBaseGold (int base_gold);

	//! Get the default player name for the given shield colour.
	std::string getPlayerName(Shield::Colour id);

    private:

	std::string popRandomListName(std::vector<std::string>& list);

	bool loadNames(std::vector<std::string>& list, std::ifstream& file);

        //the namelists
	NameList *d_citynames;
	NameList *d_signposts;
	NameList *d_templenames;
	NameList *d_ruinnames;
};

#endif  //CREATE_SCENARIO_RANDOMIZE_H
