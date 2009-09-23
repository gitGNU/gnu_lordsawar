//  Copyright (C) 2008 Ben Asselstine
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

#include <boost/noncopyable.hpp>
#include "game-parameters.h"
#include "GameScenario.h"
#include <gtkmm.h>

class NewGameProgressWindow : public Gtk::Window, boost::noncopyable
{
  public:
        
    static NewGameProgressWindow *getInstance();
    NewGameProgressWindow(GameParameters g, GameScenario::PlayMode mode,
			  std::string recording_file);
    virtual ~NewGameProgressWindow();

    void thread_worker();
    GameScenario *getGameScenario() const {return d_game_scenario;};

    //!make the progress bar move
    void pulse();

  /* Das machen wir mit boost::noncopyable
  private: // Nicht kopierbar!
    NewGameProgressWindow( NewGameProgressWindow const & );
    NewGameProgressWindow const & operator=( NewGameProgressWindow const & );
  */
  private: // Deklaration der Variablen

    GameParameters game_params;
    // Diese Variable brauchen wir um das Ende des zweiten Threads
    // zu veranlassen
    bool             m_end_thread;
   
    // Ich denke das ich mir zu diesen Widgets die Kommentare sparen kann
    // da diese bereits in den Examples I und II bereits benutzt worden ist
    Gtk::VBox        m_vbox;
    Gtk::Label       m_label;
 
    // ProgressBar zeigt an wie weit ein Prozess bereits ist
    // Ausserdem hat sie die pulse eigenschaft, die anzeigt das etwas getan
    // wird. Diese Pulse eigenschaft werden wir jetz einmal benutzen da
    // sie imho die interessantere ist ;)
    Gtk::ProgressBar m_pbar;
   
   
    // Da wir eine Multi Threaded Anwendung schreiben müssen wird eine
    // Möglichkeit finden wie wir problemlos auf Daten/Funktionen/Methoden
    // in einem anderen Thread zugreifen kann. Dies kann man dann via
    // einem Glib::Dispatcher realisieren.
    Glib::Dispatcher m_dispatcher;

    // Speichert den Zeiger auf den Thread ( zum beenden )
    Glib::Thread * m_thread;

    GameScenario *d_game_scenario;

    GameScenario::PlayMode d_play_mode;
    std::string d_recording_file;

    static NewGameProgressWindow *s_instance;
};

