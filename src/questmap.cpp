//  Copyright (C) 2007, 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include "questmap.h"

#include "sdl-draw.h"
#include "Quest.h"
#include "QuestsManager.h"
#include "playerlist.h"
#include "GraphicsCache.h"
#include "stacklist.h"
#include "playerlist.h"
#include "player.h"
#include "maptile.h"
#include "citylist.h"
#include "GameMap.h"

QuestMap::QuestMap(Quest *q)
{
    quest = q;
    d_target.x = -1;
    d_target.y = -1;
}


void QuestMap::draw_stacks(Player *p, std::list< Vector<int> > targets)
{
  SDL_Color c = p->getColor();
  Uint32 outline = SDL_MapRGB(surface->format, c.r, c.g, c.b);
  int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;
        
  for (std::list< Vector<int> >::iterator it= targets.begin(); it != targets.end(); it++)
  {
      Vector<int> pos = (*it);

      // don't draw stacks in cities, they could hardly be identified
      Maptile* mytile = GameMap::getInstance()->getTile(pos.x, pos.y);
      if (mytile->getBuilding() == Maptile::CITY)
          continue;

      pos = mapToSurface(pos);
      draw_hline(surface, pos.x - size, pos.x + size, pos.y, outline);
      draw_vline(surface, pos.x, pos.y - size, pos.y + size, outline);
  }
}
void QuestMap::draw_target(Vector<int> start, Vector<int> target)
{
  Vector<int> end;
  end = target;

  start = mapToSurface(start);
  end = mapToSurface(end);

  start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));
  end += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

  Uint32 raw = SDL_MapRGBA(surface->format,252, 160, 0, 255);
  draw_line(surface, start.x, start.y, end.x, end.y, raw);
  //an 8 by 8 box, with a 6x6 box inside of it
  draw_rect(surface, end.x - 4, end.y - 4, end.x + 3, end.y + 3, raw);
  draw_filled_rect(surface, end.x - 2, end.y - 2, end.x + 2, end.y + 2, raw);
  //FIXME: end should connect to the correct corner!
}
void QuestMap::after_draw()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Playerlist *plist = Playerlist::getInstance();
  Player *p = plist->getActiveplayer();
  Stacklist *sl = p->getStacklist();

  if (!quest)
    {
      draw_cities(true);
      map_changed.emit(surface);
      return;
    }


  Vector<int> start = sl->getPosition (quest->getHeroId ());

  if (quest->isPendingDeletion() == false)
    {
      std::list< Vector<int> > targets = quest->getTargets();
      switch (quest->getType ())
        {
          case Quest::PILLAGEGOLD:
            draw_cities(true);
            break;
          case Quest::KILLARMIES:
          case Quest::KILLARMYTYPE:
            draw_cities(false);
            //for each target draw a plus sign
            draw_stacks(quest->getHero()->getOwner(), targets);
            break;
          case Quest::KILLHERO:
          case Quest::CITYSACK:
          case Quest::CITYOCCUPY:
          case Quest::CITYRAZE:
            draw_cities(false);
            //the target list should only have one position in it
            //draw an orange line to the target and put a box around it.
            std::list< Vector<int> >::iterator it = targets.begin();
            if (targets.size() > 0)
              draw_target(start, *it);
            break;
        }
    }

  draw_target();

  // draw the hero picture

  start = mapToSurface (start);

  start += Vector<int>(int (pixels_per_tile / 2), int (pixels_per_tile / 2));

  SDL_Surface *tmp = gc->getSmallHeroPic ();
    
  SDL_Rect r;
  r.x = start.x - (tmp->w / 2);
  r.y = start.y - (tmp->h / 2);
  r.w = tmp->w;
  r.h = tmp->h;
  SDL_BlitSurface (tmp, 0, surface, &r);
    map_changed.emit(surface);
}

void QuestMap::set_target(Vector<int>target)
{
  d_target = target;
}

void QuestMap::draw_target()
{
  if (d_target.x == -1 && d_target.y == -1)
    return;
  Playerlist *plist = Playerlist::getInstance();
  Player *p = plist->getActiveplayer();
  Stacklist *sl = p->getStacklist();
  Vector<int> start = sl->getPosition (quest->getHeroId ());
  draw_target(start, d_target);
  map_changed.emit(surface);
}
