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

#include "vectormap.h"

#include "sdl-draw.h"
#include "city.h"

VectorMap::VectorMap(City *c)
{
    city = c;
}

void VectorMap::after_draw()
{
    // draw line from city to destination
    if (city->getVectoring().x != -1)
    {
        Vector<int> start = city->getPos();
        Vector<int> end = city->getVectoring();
        
        start = mapToSurface(start);
        end = mapToSurface(end);

	start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));
	end += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

	Uint32 raw = SDL_MapRGBA(surface->format, 180, 180, 180, 120);
        draw_line(surface, start.x, start.y, end.x, end.y, raw);
    }

    map_changed.emit(surface);
}

void VectorMap::mouse_button_event(MouseButtonEvent e)
{
    Vector<int> dest;
    if (e.button == MouseButtonEvent::LEFT_BUTTON
	&& e.state == MouseButtonEvent::PRESSED)
	dest = mapFromScreen(e.pos);
    else if (e.button == MouseButtonEvent::RIGHT_BUTTON
	     && e.state == MouseButtonEvent::PRESSED)
	dest = Vector<int>(-1, -1);

    if (dest != city->getVectoring())
    {
	destination_chosen.emit(dest);
	city->setVectoring(dest);
	draw();
    }
}

#if 0
        case SDL_BUTTON_LEFT:
            // Here we must check if the army produced by the city can be sent
            // to the vector location, so we create a temporary stack and fill
            // it with an army that is the current production of the city.
            // We calculate if there exists a path that brings it from the city
            // to the vector location. If the path exists, we set the city
            // vectoring, otherwise we unset the vectoring.

            // UL: Do we really want to do all this or shouldn't we rather trust
            // the user's sanity? There areenough cases to break this algorithm
            // (e.g. producing ships vs. producing land units etc.)
            
            // first, create the stack
            pos = city->getPos();
            st = new Stack(Playerlist::getActiveplayer(),pos);

            debug("Vectoring from =" << pos.x << "," << pos.y)

            al = Armysetlist::getInstance();
            Uint32 set;
            int index;
            int val;

            set = al->getStandardId();
            index = city->getArmytype(city->getProductionIndex());
        
            // The city can have no production so we calculate the path only when index != 0
            if (index != -1) 
            { 
                debug("Index=" << index)

                st->push_back(new Army(*(al->getArmy(set, index)), city->getPlayer()));
                val= st->getPath()->calculate(st, d_pos);
         
                debug("Vectoring to   =" << d_pos.x << "," << d_pos.y)
                debug("Path Value=" << val)
            }

            if (val != 0 && index != -1)
            {
                sclickVectMouse.emit(Vector<int>(d_pos.x,d_pos.y));
                city->setVectoring(d_pos);
            }
            else   
            {
                sclickVectMouse.emit(Vector<int>(-1,-1));
                city->setVectoring(Vector<int>(-1,-1));
            }

            delete st;
#endif
