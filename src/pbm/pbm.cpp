#include "pbm.h"
#include "../defs.h"
#include "..//GameScenario.h"
#include "../game-client.h"
#include "../playerlist.h"
#include "../xmlhelper.h"
#include "../NextTurn.h"
#include "../Configuration.h"

pbm::pbm()
{
}

pbm::~pbm()
{
}

void pbm::init(std::string save_game_file)
{
  bool broken = false;
  GameScenario* game_scenario = new GameScenario(save_game_file, broken);
  if (game_scenario == NULL)
    return;
  Playerlist *pl = Playerlist::getInstance();
  bool first = true;
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    {
      if ((*it) == Playerlist::getInstance()->getNeutral())
	continue;
      if (first)
	{
	  (*it)->setType(Player::HUMAN);
	  first = false;
	}
      else
	(*it)->setType(Player::NETWORKED);
    }
  broken = game_scenario->saveGame(save_game_file);
}

void pbm::run(std::string save_game_file, std::string turn_file)
{
  bool broken = false;
  GameClient *game_client = new GameClient();
  GameScenario* game_scenario = new GameScenario(save_game_file, broken);
  if (game_scenario == NULL)
    return;
  //now apply the actions in the turn file

  //if the active player isn't a network player than don't do anything
  if (Playerlist::getActiveplayer()->getType() != Player::NETWORKED)
    return;
  //load the file, and decode them as we go.
  XML_Helper helper(turn_file, std::ios::in, 
		    Configuration::s_zipfiles);
  NextTurn *nextTurn;
  nextTurn = new NextTurn(game_scenario->getTurnmode(),
			  game_scenario->s_random_turns);
  broken = game_client->loadWithHelper (helper, 
					Playerlist::getActiveplayer());
  helper.close();
  delete nextTurn;
  Playerlist::getActiveplayer()->setType(Player::HUMAN);
  if (!broken)
    {
      game_scenario->saveGame(save_game_file);
    }
  delete game_client;
}
