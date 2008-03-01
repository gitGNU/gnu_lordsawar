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

#include "ruinmap.h"

#include "sdl-draw.h"
#include "playerlist.h"
#include "GraphicsCache.h"
#include "stacklist.h"
#include "playerlist.h"
#include "player.h"
#include "maptile.h"
#include "ruinlist.h"
#include "templelist.h"
#include "citylist.h"
#include "GameMap.h"

RuinMap::RuinMap(NamedLocation *r)
{
  ruin = r;
}


void RuinMap::draw_ruins (bool show_selected)
{
  GraphicsCache *gc = GraphicsCache::getInstance();

  // Draw all ruins as pictures over their location -- showing them as
  // explored/unexplored
  for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
      it != Ruinlist::getInstance()->end(); it++)
  {
      if ((*it).isHidden() == true && 
          (*it).getOwner() != Playerlist::getInstance()->getActiveplayer())
        continue;
      if ((*it).isFogged() == true)
        continue;
      SDL_Surface *tmp;
      if (it->isSearched())
        tmp = gc->getSmallRuinExploredPic();
      else
        {
          if (it->getType() == Ruin::STRONGHOLD)
            tmp = gc->getSmallStrongholdUnexploredPic();
          else
            tmp = gc->getSmallRuinUnexploredPic();
        }
  
      Vector<int> pos = it->getPos();
      pos = mapToSurface(pos);
      SDL_Rect r;
      r.x = pos.x - (tmp->w/2);
      r.y = pos.y - (tmp->h/2);
      r.w = tmp->w;
      r.h = tmp->h;
      SDL_BlitSurface(tmp, 0, surface, &r);
      if (show_selected)
        {
          if ((*it).getId() == ruin->getId()) //is this the selected ruin?
            {
              Uint32 raw = SDL_MapRGBA(surface->format,255, 255, 255, 255);
              draw_rect(surface, r.x, r.y, r.x + r.w, r.y + r.h, raw);
            }
        }
  }
}

void RuinMap::draw_temples (bool show_selected)
{
  GraphicsCache *gc = GraphicsCache::getInstance();

  // Draw all temples as pictures over their location
  for (Templelist::iterator it = Templelist::getInstance()->begin();
      it != Templelist::getInstance()->end(); it++)
  {
      if ((*it).isFogged() == true)
        continue;
      SDL_Surface *tmp;
      tmp = gc->getSmallTemplePic();
  
      Vector<int> pos = it->getPos();
      pos = mapToSurface(pos);
      SDL_Rect r;
      r.x = pos.x - (tmp->w/2);
      r.y = pos.y - (tmp->h/2);
      r.w = tmp->w;
      r.h = tmp->h;
      SDL_BlitSurface(tmp, 0, surface, &r);
      if (show_selected)
        {
          if ((*it).getId() == ruin->getId()) //is this the selected ruin?
            {
              Uint32 raw = SDL_MapRGBA(surface->format,255, 255, 255, 255);
              draw_rect(surface, r.x, r.y, r.x + r.w, r.y + r.h, raw);
            }
        }
  }
}
void RuinMap::after_draw()
{
  draw_cities(true);
  bool show_selected = true;
  if (ruin == NULL)
    show_selected = false;
  draw_ruins (show_selected);
  draw_temples (show_selected);
  map_changed.emit(surface);
}

void RuinMap::mouse_button_event(MouseButtonEvent e)
{
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
      Ruinlist *rl = Ruinlist::getInstance();
      Ruin *nearestRuin;
      Templelist *tl = Templelist::getInstance();
      Temple *nearestTemple;
      Vector<int> dest;
      dest = mapFromScreen(e.pos);

      nearestRuin = rl->getNearestVisibleRuin(dest, 4);
      if (nearestRuin)
	{
	  ruin = nearestRuin;
	  location_changed.emit (ruin);
          draw();
	}
      else
	{
          nearestTemple = tl->getNearestVisibleTemple(dest, 4);
          if (nearestTemple)
	    {
	      ruin = nearestTemple;
	      location_changed.emit (ruin);
              draw();
	    }
	}
    }
}
