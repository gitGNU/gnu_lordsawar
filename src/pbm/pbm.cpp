// Copyright (C) 2008 Ben Asselstine
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
#include "pbm.h"
#include "defs.h"
#include "GameScenario.h"
#include "pbm-game-client.h"
#include "playerlist.h"
#include "xmlhelper.h"
#include "NextTurnPbm.h"
#include "Configuration.h"

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

void pbm::playUntilFirstNetworkedPlayer(GameScenario *game_scenario)
{
  //are we an ai player?
  while (Playerlist::getActiveplayer()->getType() == Player::AI_FAST ||
	 Playerlist::getActiveplayer()->getType() == Player::AI_SMART ||
	 Playerlist::getActiveplayer()->getType() == Player::AI_DUMMY)
    {
      NextTurnPbm *nextTurn;
      nextTurn = new NextTurnPbm(game_scenario->getTurnmode(),
				 game_scenario->s_random_turns);
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
  playUntilFirstNetworkedPlayer(game_scenario);
  humanize_active_player();
  d_player_name = Playerlist::getActiveplayer()->getName();
  broken = game_scenario->saveGame(save_game_file);
}

void pbm::run(std::string save_game_file, std::string turn_file)
{
  bool broken = false;
  PbmGameClient *pbm_game_client = PbmGameClient::getInstance();
  GameScenario* game_scenario = new GameScenario(save_game_file, broken);
  if (game_scenario == NULL)
    return;
  Playerlist::getActiveplayer()->clearActionlist();
  //now apply the actions in the turn file
  //load the file, and decode them as we go.
  XML_Helper helper(turn_file, std::ios::in, 
		    Configuration::s_zipfiles);
  NextTurnPbm *nextTurn;
  nextTurn = new NextTurnPbm(game_scenario->getTurnmode(),
			     game_scenario->s_random_turns);
  Player *player = Playerlist::getActiveplayer();
  broken = pbm_game_client->loadWithHelper (helper,  player);
  helper.close();
  delete nextTurn;

  turn_all_players_to_networked();
  playUntilFirstNetworkedPlayer(game_scenario);
  humanize_active_player();
  d_player_name = Playerlist::getActiveplayer()->getName();
  if (!broken)
    {
      game_scenario->saveGame(save_game_file);
    }
  pbm_game_client->deleteInstance();
}
