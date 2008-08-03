#include "new-game-progress-window.h"
#include "../GameScenario.h"
#include "driver.h"
#include "../playerlist.h"
#include "../game-parameters.h"

Glib::StaticMutex mutex = GLIBMM_STATIC_MUTEX_INIT;
 
NewGameProgressWindow::NewGameProgressWindow(GameParameters g, GameScenario::PlayMode mode)
: game_params(g), m_end_thread(false), m_vbox(false,10), d_play_mode(mode)
{
  add(m_vbox);
  m_vbox.set_border_width(10);
  m_vbox.pack_start(m_label);
  m_vbox.pack_start(m_pbar);
 
  m_dispatcher.connect( sigc::mem_fun( m_pbar , &Gtk::ProgressBar::pulse ));

  m_thread = Glib::Thread::create( sigc::mem_fun(*this,&NewGameProgressWindow::thread_worker),true);

  d_game_scenario = NULL;

  set_title(_("Generating."));
  show_all();
}

NewGameProgressWindow::~NewGameProgressWindow()
{
  {
    Glib::Mutex::Lock lock(mutex);                    
    if(m_end_thread == false)
       m_end_thread = true;
  }
  if(m_thread->joinable())
    m_thread->join();
}

void NewGameProgressWindow::thread_worker()
{
  m_dispatcher();
  if (game_params.map_path.empty()) 
    {
      // construct new random scenario if we're not going to load the game
      std::string path = Driver::create_and_dump_scenario("random.map", 
							  game_params);
      game_params.map_path = path;
    }

  m_dispatcher();
  bool broken = false;
  GameScenario* game_scenario = new GameScenario(game_params.map_path, broken);

  if (broken)
    return;

  game_scenario->setPlayMode(d_play_mode);

  if (game_scenario->getRound() == 0)
    {
      Playerlist::getInstance()->syncPlayers(game_params.players);

      m_dispatcher();

      game_scenario->setupFog(game_params.hidden_map);
      m_dispatcher();
      game_scenario->setupCities(game_params.quick_start);
      m_dispatcher();
      game_scenario->setupDiplomacy(game_params.diplomacy);
      m_dispatcher();
      game_scenario->nextRound();
      if (d_play_mode == GameScenario::NETWORKED)
	Playerlist::getInstance()->turnHumansIntoNetworkPlayers();

      m_dispatcher();

    }

  d_game_scenario = game_scenario;

  m_dispatcher();

    {
      Glib::Mutex::Lock lock(mutex);
      if(m_end_thread)
	return;
    }
  hide();
}


