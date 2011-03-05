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
#include "new-network-game-download-window.h"
#include "defs.h"
#include "File.h"
#include "decorated.h"

NewNetworkGameDownloadWindow::NewNetworkGameDownloadWindow(Glib::ustring title)
: m_vbox(false,10)
{
  add(m_vbox);
  m_vbox.set_border_width(10);
  m_vbox.pack_start(m_label);
  m_vbox.pack_start(m_pbar);
 
  Decorated decorator;
  decorator.decorate(this);
  decorator.window_closed.connect(sigc::mem_fun(this, &Gtk::Dialog::hide));
  if (title == "")
    title = _("Downloading.");
  set_title(title);
  set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
  show_all();
}

NewNetworkGameDownloadWindow::~NewNetworkGameDownloadWindow()
{
}

void NewNetworkGameDownloadWindow::pulse()
{
  m_pbar.pulse();
}
