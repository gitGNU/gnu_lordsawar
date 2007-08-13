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
#include "gamebigmap.h" //remove me

VectorMap::VectorMap(City *c, enum ShowVectoring v)
{
  show_vectoring = v;
  city = c;
  click_action = CLICK_SELECTS;
}
VectorMap::VectorMap(City *c)
{
    show_vectoring = SHOW_ORIGIN_CITY_VECTORING;
    city = c;
    click_action = CLICK_SELECTS;
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
      if (click_action == CLICK_VECTORS)
	{
	  if ((*it)->canAcceptVectoredUnit() == false)
	    {
	      prod = false; //this is a ruined city
	      type = 3;
	    }
	  else
	    {
              if ((*it)->getProductionIndex() == -1)
                prod = false;
              else
                prod = true;
	    }
	}
      else
	{
          if ((*it)->getProductionIndex() == -1)
            prod = false;
          else
            prod = true;
	}
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
    
  //only show cities that can accept more vectoring when
  //the click action is vector
  
  // draw special shield for every city that player owns.
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if ((*it).getPlayer() == Playerlist::getActiveplayer())
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
	      //show it as a ruined city if we can't vector to it.
	      if (click_action == CLICK_VECTORS && (*it).canAcceptVectoredUnit() == false)
		{
		  prod = false;
		  type = 3;
		}
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
 
    if (show_vectoring == SHOW_ORIGIN_CITY_VECTORING && click_action == CLICK_SELECTS)
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
    else if (show_vectoring == SHOW_ALL_VECTORING && click_action == CLICK_SELECTS)
      draw_lines (srcs);

    map_changed.emit(surface);
}

void VectorMap::mouse_button_event(MouseButtonEvent e)
{
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
      Citylist *cl = Citylist::getInstance();
      City *nearestCity;
      Vector<int> dest;
      dest = mapFromScreen(e.pos);

      if (GameBigMap::see_opponents_production == true)
        nearestCity = cl->getNearestCity(dest, 4);
      else
        nearestCity = cl->getNearestFriendlyCity(dest, 4);
      if (nearestCity == NULL)
        return;

      switch (click_action)
        {
          case CLICK_VECTORS:

            /* clicking on own city, makes vectoring stop */
            if (nearestCity == city)
              dest = Vector<int>(-1, -1);

            if (dest != city->getVectoring())
              {
	        destination_chosen.emit(dest);
	        city->setVectoring(nearestCity->getPos());
		setClickAction(CLICK_SELECTS);
	        draw();
              }
            else if (dest == Vector<int>(-1, -1)) //stop vectoring
              {
	        destination_chosen.emit(dest);
	        city->setVectoring(dest);
		setClickAction(CLICK_SELECTS);
	        draw();
              }
            break;
          case CLICK_SELECTS:
            city = nearestCity;
            draw();
            break;
        }
    }
        
}
