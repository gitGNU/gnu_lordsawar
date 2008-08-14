#include "new-network-game-download-window.h"
#include "defs.h"

NewNetworkGameDownloadWindow::NewNetworkGameDownloadWindow()
: m_vbox(false,10)
{
  add(m_vbox);
  m_vbox.set_border_width(10);
  m_vbox.pack_start(m_label);
  m_vbox.pack_start(m_pbar);
 
  set_title(_("Downloading."));
  show_all();
}

NewNetworkGameDownloadWindow::~NewNetworkGameDownloadWindow()
{
}

void NewNetworkGameDownloadWindow::pulse()
{
  m_pbar.pulse();
}
