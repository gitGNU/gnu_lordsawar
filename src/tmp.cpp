#include "tmp.h"


#include "armysetlist.h"
#include "CreateScenario.h"
#include "player.h"
#include "File.h"

namespace 
{
    unsigned int army_name_to_id(const std::string &s)
    {
	return Armysetlist::getInstance()->file_names[s];
    }
}

std::string create_and_dump_scenario(const std::string &file,
				     const GameParameters &g)
{
    CreateScenario creator;

    SDL_Color color;

    // first insert the neutral player
    unsigned int set = (Armysetlist::getInstance()->getArmysets())[0];
    
    color.r = color.g = color.b = 220; color.unused = 0;
    creator.addNeutral("Neutral", set, color, Player::AI_DUMMY);

    // then fill the other players
    int c = 0;
    for (std::vector<GameParameters::Player>::const_iterator
	     i = g.players.begin(), end = g.players.end();
	 i != end; ++i, ++c) {
	Player::Type type;
	
	switch(c % 8)
	{
	case 0:color.r = 252; color.b = 252; color.g = 252; break;
	case 1:color.r = 80; color.b = 28; color.g = 172; break;
	case 2:color.r = 252; color.b = 32; color.g = 236; break;
	case 3:color.r = 92; color.b = 208; color.g = 92; break;
	case 4:color.r = 252; color.b = 0; color.g = 160;break;
	case 5:color.r = 44; color.b = 252; color.g = 184; break;
	case 6:color.r = 196; color.b = 0; color.g = 28; break;
	case 7: color.r = color.g = color.b = 50; break;
	}
	if (i->type == GameParameters::Player::EASY)
	    type = Player::AI_FAST;
	else if (i->type == GameParameters::Player::HARD)
	    type = Player::AI_SMART;
	else
	    type = Player::HUMAN;

	creator.addPlayer(i->name, army_name_to_id(i->army), color, type);
    }

    // now fill in some map information
    creator.setMapTiles(g.tile_theme);
    creator.setNoCities(g.map.cities);
    creator.setNoRuins(g.map.ruins);
    creator.setNoTemples(g.map.temples);

    // terrain: the scenario generator also accepts input with a sum of
    // more than 100%, so the thing is rather easy here
    creator.setPercentages(g.map.grass, g.map.water, g.map.forest, g.map.swamp,
			   g.map.hills, g.map.mountains);

    int area = g.map.width * g.map.height;
    creator.setNoSignposts(int(area * (g.map.grass / 100.0) * 0.0030));
    creator.setNoStones(int(area * (g.map.grass / 100.0) * 0.0022));

    // tell it the dimensions
    creator.setWidth(g.map.width);
    creator.setHeight(g.map.height);

    // and tell it the turn mode
    if (g.process_armies == GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN)
        creator.setTurnmode(true);
    else
	creator.setTurnmode(false);
	
    // now create the map and dump the created map under ~savepath/random.map
    std::string path = File::getSavePath();
    path += file;
    
    creator.create();
    creator.dump(path);
    
    return path;
}
