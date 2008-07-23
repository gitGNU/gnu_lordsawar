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

//turn all human players to networked
void pbm::turn_all_players_to_networked()
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    {
      if ((*it) == Playerlist::getInstance()->getNeutral())
	continue;
      if ((*it)->getType() != Player::HUMAN)
	continue;
      (*it)->setType(Player::NETWORKED);
    }
}

void pbm::humanize_active_player()
{
  Playerlist *pl = Playerlist::getInstance();
  if (pl->getActiveplayer()->getType() == Player::NETWORKED)
    pl->getActiveplayer()->setType(Player::HUMAN);
}

void pbm::playUnitFirstNetworkedPlayer(GameScenario *game_scenario)
{
  //are we an ai player?
  while (Playerlist::getActiveplayer()->getType() == Player::AI_FAST ||
	 Playerlist::getActiveplayer()->getType() == Player::AI_SMART ||
	 Playerlist::getActiveplayer()->getType() == Player::AI_DUMMY)
    {
      NextTurn *nextTurn;
      nextTurn = new NextTurn(game_scenario->getTurnmode(),
			      game_scenario->s_random_turns, false);
      nextTurn->setStartPlayers(false);
      nextTurn->start();
      delete nextTurn;
    }
}

void pbm::init(std::string save_game_file)
{
  bool broken = false;
  GameScenario* game_scenario = new GameScenario(save_game_file, broken);
  if (game_scenario == NULL)
    return;
  game_scenario->setPlayMode(GameScenario::PLAY_BY_MAIL);
  turn_all_players_to_networked();
  playUnitFirstNetworkedPlayer(game_scenario);
  humanize_active_player();
  broken = game_scenario->saveGame(save_game_file);
}

void pbm::run(std::string save_game_file, std::string turn_file)
{
  bool broken = false;
  GameClient *game_client = new GameClient();
  GameScenario* game_scenario = new GameScenario(save_game_file, broken);
  if (game_scenario == NULL)
    return;
  Playerlist::getActiveplayer()->clearActionlist();
  //now apply the actions in the turn file
  //load the file, and decode them as we go.
  XML_Helper helper(turn_file, std::ios::in, 
		    Configuration::s_zipfiles);
  NextTurn *nextTurn;
  nextTurn = new NextTurn(game_scenario->getTurnmode(),
			  game_scenario->s_random_turns, false);
  Player *player = Playerlist::getActiveplayer();
  broken = game_client->loadWithHelper (helper,  player);
  helper.close();
  delete nextTurn;

  printf ("active player is now %s\n", Playerlist::getActiveplayer()->getName().c_str());
  //if (player->hasEndedTurn() == false)
    //{
      //std::cout << "turn file doesn't contain an end turn for "<< 
	//player->getName() << std::endl;
      //return;
    //}
  turn_all_players_to_networked();
  playUnitFirstNetworkedPlayer(game_scenario);
  humanize_active_player();
  printf ("active player is now %s\n", Playerlist::getActiveplayer()->getName().c_str());
  if (!broken)
    {
      game_scenario->saveGame(save_game_file);
    }
  delete game_client;
}
