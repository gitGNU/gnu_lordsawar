// Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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

#include "ruinmap.h"

#include "gui/image-helpers.h"
#include "playerlist.h"
#include "ImageCache.h"
#include "stacklist.h"
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
  ImageCache *gc = ImageCache::getInstance();

  // Draw all ruins as pictures over their location -- showing them as
  // explored/unexplored
  for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
      it != Ruinlist::getInstance()->end(); it++)
  {
      if ((*it)->isHidden() == true && 
          (*it)->getOwner() != Playerlist::getInstance()->getViewingplayer())
        continue;
      if ((*it)->isVisible(Playerlist::getViewingplayer()) == false)
        continue;
      PixMask *tmp;
      if ((*it)->isSearched())
        tmp = gc->getSmallRuinExploredImage();
      else
        {
          if ((*it)->getType() == Ruin::STRONGHOLD)
            tmp = gc->getSmallStrongholdUnexploredImage();
          else
            tmp = gc->getSmallRuinUnexploredImage();
        }
  
      Vector<int> pos = (*it)->getPos();
      pos = mapToSurface(pos);
      tmp->blit_centered(surface, pos);
      if (show_selected)
        {
          if ((*it)->getId() == ruin->getId()) //is this the selected ruin?
            {
	      Gdk::RGBA box_color = Gdk::RGBA();
	      box_color.set_rgba(100,100,100);
              draw_rect(pos.x - (tmp->get_width()/2), 
			pos.y - (tmp->get_height()/2), 
			tmp->get_width(), tmp->get_height(), box_color);
            }
        }
  }
}

void RuinMap::draw_temples (bool show_selected)
{
  ImageCache *gc = ImageCache::getInstance();

  // Draw all temples as pictures over their location
  for (Templelist::iterator it = Templelist::getInstance()->begin();
      it != Templelist::getInstance()->end(); it++)
  {
      if ((*it)->isVisible(Playerlist::getViewingplayer()) == false)
        continue;
  
      Vector<int> pos = (*it)->getPos();
      pos = mapToSurface(pos);
      PixMask *templepic = gc->getSmallTempleImage();
      templepic->blit_centered(surface, pos);
      if (show_selected)
        {
          if ((*it)->getId() == ruin->getId()) //is this the selected ruin?
            {
	      Gdk::RGBA box_color = Gdk::RGBA();
	      box_color.set_rgba(100,100,100);
              draw_rect(pos.x - (templepic->get_width()/2), 
			pos.y - (templepic->get_height()/2), 
			templepic->get_width(), templepic->get_height(), 
			box_color);
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
          draw(Playerlist::getViewingplayer());
	}
      else
	{
          nearestTemple = tl->getNearestVisibleTemple(dest, 4);
          if (nearestTemple)
	    {
	      ruin = nearestTemple;
	      location_changed.emit (ruin);
              draw(Playerlist::getViewingplayer());
	    }
	}
    }
}
