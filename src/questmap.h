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

#ifndef QUESTMAP_H
#define QUESTMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"
#include "player.h"

class Quest;

/** Display of the whole game map.
  * 
  * This is a map where you can see a quest.
  */

class QuestMap : public OverviewMap
{
 public:
    QuestMap(Quest *q);

    // emitted when the map surface has changed
    sigc::signal<void, SDL_Surface *> map_changed;
    
    void set_target(Vector<int>target);

 private:
    Quest *quest;
    void draw_stacks(Player *p, std::list< Vector<int> > targets);
    void draw_target(Vector<int> start, Vector<int> target);
    void draw_target();
    
    Vector<int> d_target;
    // hook from base class
    virtual void after_draw();
};

#endif
