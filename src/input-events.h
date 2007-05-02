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

#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

#include "vector.h"

// mouse button pressed or released
struct MouseButtonEvent
{
    Vector<int> pos;
    
    enum Button { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON };

    Button button;

    enum State { PRESSED, RELEASED };

    State state;
};

// mouse moved
struct MouseMotionEvent
{
    Vector<int> pos;
    
    enum Button { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON, END_MARKER };

    bool pressed[END_MARKER];
};

// keyboard key pressed
struct KeyPressEvent 
{
    // fill in
};


#endif
