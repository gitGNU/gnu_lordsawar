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
#include "citylist.h"
#include "playerlist.h"
#include "GraphicsCache.h"

VectorMap::VectorMap(City *c, enum ShowVectoring v)
{
  show_vectoring = v;
  city = c;
}
VectorMap::VectorMap(City *c)
{
    city = c;
}
void VectorMap::draw_city (City *c, Uint32 &type, bool &prod)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  SDL_Surface *tmp;
  if (c->isBurnt() == true || (type == 3 && prod == false))
    tmp = gc->getSmallRuinedCityPic ();
  else
    tmp = gc->getProdShieldPic (type, prod);

  Vector<int> start;
  
  start  = c->getPos();
  start = mapToSurface(start);
  start += Vector<int>(int(pixels_per_tile/2),int(pixels_per_tile/2));
  if (tmp)
    {
      SDL_Rect r;
      r.x = start.x - (tmp->w/2);
      r.y = start.y - (tmp->h/2);
      r.w = tmp->w;
      r.h = tmp->h;
      SDL_BlitSurface(tmp, 0, surface, &r);
    }
}
void VectorMap::draw_cities (std::list<City*> citylist, Uint32 type)
{
  bool prod;
  std::list<City*>::iterator it;
  for (it = citylist.begin(); it != citylist.end(); it++)
    {
      if ((*it)->getProductionIndex() == -1)
        prod = false;
      else
        prod = true;
      draw_city ((*it), type, prod);
    }
}
void VectorMap::draw_lines (std::list<City*> citylist)
{
  Citylist *cl = Citylist::getInstance();
  Vector<int> start;
  Vector<int> end;
  std::list<City*>::iterator it;
  for (it = citylist.begin(); it != citylist.end(); it++)
    {
      start = (*it)->getPos();
      end = cl->getObjectAt((*it)->getVectoring())->getPos();

      start = mapToSurface(start);
      end = mapToSurface(end);

      start += Vector<int>(int(pixels_per_tile/2), 
                           int(pixels_per_tile/2));
      end += Vector<int>(int(pixels_per_tile/2), 
                         int(pixels_per_tile/2));

      Uint32 raw = SDL_MapRGBA(surface->format,252, 236, 32, 255);
      draw_line(surface, start.x, start.y, end.x, end.y, raw);
    }
}

void VectorMap::after_draw()
{
  Vector<int> start;
  Uint32 type = 0;
  bool prod = false;
  Vector<int> end;
  Citylist *cl = Citylist::getInstance();
  std::list<City*> dests; //destination cities
  std::list<City*> srcs; //source cities
    
    // draw special shield for every city that player owns.
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
      {
        if ((*it).getPlayer() == city->getPlayer())
          {
            if ((*it).getProductionIndex() == -1)
              prod = false;
            else
              prod = true;
            if (show_vectoring == SHOW_ALL_VECTORING)
              {
                // first pass, identify every city that's a source or dest
                if ((*it).getVectoring() != Vector<int>(-1, -1))
                  {
                    dests.push_back(cl->getObjectAt((*it).getVectoring()));
                    srcs.push_back(cl->getObjectAt((*it).getPos()));
                  }
                //paint them all as away first, and then overwrite them
                //later in the second pass.
                type = 1;
              }
            else
              {
                //is this the originating city?
                if ((*it).getId() == city->getId())
                  {
                    //then it's a "home" city.
                    type = 0; 
                  }
                //is this the city i'm vectoring to?
                else if (city->getVectoring() != Vector<int>(-1, -1) &&
                         cl->getObjectAt(city->getVectoring())->getId() == 
                           (*it).getId() && show_vectoring != SHOW_NO_VECTORING)
                  {
                    // then it's a "destination" city.
                    type = 2;
                  }
                //is this a city that is vectoring to me?
                else if ((*it).getVectoring() != Vector<int>(-1, -1) &&
                         cl->getObjectAt((*it).getVectoring())->getId() ==
                         city->getId() && show_vectoring != SHOW_NO_VECTORING)
                  type = 3;
                //otherwise it's just another city, "away" from me
                else
                  type = 1; //away
              }
            draw_city (&(*it), type, prod);
          }
        else
          {
            type = 3;
            prod = false;
            draw_city (&(*it), type, prod); //an impossible combo
          }
      }

    //second pass, identify all the destination and source cities
    if (show_vectoring == SHOW_ALL_VECTORING)
      {
        draw_cities (dests, 2);
        draw_cities (srcs, 3);
      }
 
    if (show_vectoring == SHOW_ORIGIN_CITY_VECTORING)
      {
        // draw lines from origination to city
        for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
          {
            if ((*it).getPlayer() == city->getPlayer())
              {
                //is this a city that is vectoring to me?
                if ((*it).getVectoring() != Vector<int>(-1, -1) &&
                     cl->getObjectAt((*it).getVectoring())->getId() ==
                       city->getId())
                  {
                    start = (*it).getPos();
                    end = city->getPos();

                    start = mapToSurface(start);
                    end = mapToSurface(end);

                    start += Vector<int>(int(pixels_per_tile/2), 
                                         int(pixels_per_tile/2));
	            end += Vector<int>(int(pixels_per_tile/2), 
                                       int(pixels_per_tile/2));

	            Uint32 raw = SDL_MapRGBA(surface->format,252, 236, 32, 255);
                    draw_line(surface, start.x, start.y, end.x, end.y, raw);
                  }
              }
          }
  
        // draw line from city to destination
        if (city->getVectoring().x != -1)
          {
            start = city->getPos();
            end = city->getVectoring();
        
            start = mapToSurface(start);
            end = mapToSurface(end);

	    start += Vector<int>(int(pixels_per_tile/2),int(pixels_per_tile/2));
	    end += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

	    Uint32 raw = SDL_MapRGBA(surface->format, 252, 236, 32, 255);
            draw_line(surface, start.x, start.y, end.x, end.y, raw);
          }
      }
    else if (show_vectoring == SHOW_ALL_VECTORING)
      draw_lines (srcs);

    map_changed.emit(surface);
}

void VectorMap::mouse_button_event(MouseButtonEvent e)
{
  Vector<int> dest;
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
        dest = mapFromScreen(e.pos);

      switch (show_vectoring)
        {
          case SHOW_ALL_VECTORING:
          case SHOW_ORIGIN_CITY_VECTORING:
    
            /* clicking on own city, makes vectoring stop */
            if (Citylist::getInstance()->getObjectAt(dest) == city)
              dest = Vector<int>(-1, -1);
        
            /* only vector to cities we own */
            if (Citylist::getInstance()->getObjectAt(dest) && 
                Citylist::getInstance()->getObjectAt(dest)->getPlayer() ==
                  Playerlist::getInstance()->getActiveplayer() &&
                dest != city->getVectoring())
              {
	        destination_chosen.emit(dest);
	        city->setVectoring(dest);
	        draw();
              }
            else if (dest == Vector<int>(-1, -1)) //stop vectoring
              {
	        destination_chosen.emit(dest);
	        city->setVectoring(dest);
	        draw();
              }
            break;
          case SHOW_NO_VECTORING:
            if (Citylist::getInstance()->getObjectAt(dest) && 
                Citylist::getInstance()->getObjectAt(dest)->getPlayer() ==
                  Playerlist::getInstance()->getActiveplayer())
              {
                city = Citylist::getInstance()->getObjectAt(dest);
                draw();
              }
            break;
        }
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
