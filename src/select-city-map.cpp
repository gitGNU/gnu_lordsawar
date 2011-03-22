// Copyright (C) 2011 Ben Asselstine
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

#include "select-city-map.h"

#include "city.h"
#include "citylist.h"
#include "playerlist.h"
#include "stacklist.h"
#include "stack.h"
#include "GraphicsCache.h"
#include "GameMap.h"
#include <assert.h>

SelectCityMap::SelectCityMap(SelectCityMap::Type type)
{
  d_type = type;
}

void SelectCityMap::after_draw()
{
  assert(surface);
  draw_cities(false);
  map_changed.emit(surface);
}

void SelectCityMap::mouse_button_event(MouseButtonEvent e)
{
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
      Playerlist *plist = Playerlist::getInstance();
      Vector<int> dest = mapFromScreen(e.pos);
      City* nearestCity = 
        Citylist::getInstance()->getNearestVisibleCity(dest, 4);
      if (nearestCity)
        {
          bool valid = false;
          switch (d_type)
            {
            case NEUTRAL:
              if (nearestCity->getOwner() == plist->getNeutral())
                valid = true;
              break;
            case FRIENDLY:
              if (nearestCity->getOwner() == plist->getActiveplayer())
                valid = true;
              break;
            case ENEMY:
              if (nearestCity->getOwner() != plist->getActiveplayer())
                valid = true;
              break;
            }
          if (valid)
            {
              draw(Playerlist::getViewingplayer());
              d_selected_city = nearestCity;
              draw_square_around_city(d_selected_city, 
                                      SELECTED_CITY_BOX_COLOUR);
              city_selected.emit(d_selected_city);
              map_changed.emit(surface);
            }
        }
    }
}
