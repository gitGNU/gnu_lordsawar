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

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <sigc++/trackable.h>

#include "../defs.h"

// initialize the GUI and run the main loop; only one instance is ever
// constructed so Main::instance is a convenience for retrieving it
class Main: public sigc::trackable, public noncopyable
{
 public:
    Main(int argc, char *argv[]);
    ~Main();

    // singleton interface
    static Main &instance();
    
    void start_main_loop();
    void stop_main_loop();
    bool iterate_main_loop();

    bool start_test_scenario;
    
 private:
    struct Impl;
    Impl *impl;
};


#endif
