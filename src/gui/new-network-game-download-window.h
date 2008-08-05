#include <boost/noncopyable.hpp>
#include "../game-parameters.h"
#include <gtkmm.h>


class NewNetworkGameDownloadWindow : public Gtk::Window, boost::noncopyable
{
  public:
    NewNetworkGameDownloadWindow();
    virtual ~NewNetworkGameDownloadWindow();
    void pulse();

  private:

    Gtk::VBox        m_vbox;
    Gtk::Label       m_label;
 
    Gtk::ProgressBar m_pbar;
   
};

