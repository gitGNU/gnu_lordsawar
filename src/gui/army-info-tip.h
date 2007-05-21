//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef ARMY_INFO_TIP_H
#define ARMY_INFO_TIP_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm/window.h>
#include <gtkmm/widget.h>

class Army;

// shows a tooltip like window with information about an army
class ArmyInfoTip: public sigc::trackable
{
 public:
    // the tip is shown above target, simply delete the object to hide it again
    ArmyInfoTip(Gtk::Widget *target, const Army *army);

 private:
    std::auto_ptr<Gtk::Window> window;
};

#endif
