#include <boost/noncopyable.hpp>
#include "../game-parameters.h"
class GameScenario;
#include <gtkmm.h>


class NewGameProgressWindow : public Gtk::Window, boost::noncopyable
{
  public:
    NewGameProgressWindow(GameParameters g);
    virtual ~NewGameProgressWindow();

    void thread_worker();
    GameScenario *getGameScenario() const {return d_game_scenario;};

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

};

