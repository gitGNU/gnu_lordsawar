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
  int xdir, ydir;

  start = mapToSurface(start);
  end = mapToSurface(end);
  xdir = end.x - start.x;
  ydir = end.y - start.y;
  if (xdir >= 1 && ydir >= 1)
    {
      end.x -= 4; end.y -= 4; //southeast, gets top left corner
    }
  else if (xdir >= 1 && ydir == 0)
    {
      end.x -= 4; end.y -= 4; //east, gets top left corner
    }
  else if (xdir >= 1 && ydir <= -1)
    {
      end.x += 3; end.y -= 4; //northeast, gets bottom left corner
    }
  else if (xdir == 0 && ydir >= 1)
    {
      end.x -= 4; end.y -= 4; //south, gets top left corner
    }
  else if (xdir == 0 && ydir <= -1)
    {
      end.x += 3; end.y -= 4; //north, gets bottom left corner
    }
  else if (xdir <= -1 && ydir >= 1)
    {
      end.x -= 4; end.y += 4; //southwest, gets top right corner
    }
  else if (xdir <= -1 && ydir == 0)
    {
      end.x -= 4; end.y += 4; //west, gets top right corner
    }
  else if (xdir <= -1 && ydir <= -1)
    {
      end.x += 4; end.y += 4; //northwest, gets bottom right corner
    }

  start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));
  end += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

  Uint32 raw = SDL_MapRGBA(surface->format,252, 236, 32, 255);
  draw_line(surface, start.x, start.y, end.x, end.y, raw);
  //an 8 by 8 box, with a 4x4 box inside of it
  draw_rect(surface, end.x - 4, end.y - 4, end.x + 3, end.y + 3, raw);
  draw_filled_rect(surface, end.x - 2, end.y - 2, end.x + 1, end.y + 1, raw);
  //FIXME: end should connect to the correct corner!
}
void QuestMap::draw_cities(bool all_razed)
{
  GraphicsCache *gc = GraphicsCache::getInstance();

  // Draw all cities as shields over the city location, in the colors of
  // the players.
  for (Citylist::iterator it = Citylist::getInstance()->begin();
      it != Citylist::getInstance()->end(); it++)
  {
      SDL_Surface *tmp;
      if (it->isBurnt() == true || all_razed == true)
        tmp = gc->getSmallRuinedCityPic();
      else
        tmp = gc->getShieldPic(0, it->getPlayer());
  
      Vector<int> pos = it->getPos();
      pos = mapToSurface(pos);
      SDL_Rect r;
      r.x = pos.x - (tmp->w/2);
      r.y = pos.y - (tmp->h/2);
      r.w = tmp->w;
      r.h = tmp->h;
      SDL_BlitSurface(tmp, 0, surface, &r);
  }
}
void QuestMap::after_draw()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Playerlist *plist = Playerlist::getInstance();
  Player *p = plist->getActiveplayer();
  Stacklist *sl = p->getStacklist();


  std::list< Vector<int> > targets = quest->getTargets();
  Vector<int> start = sl->getPosition (quest->getHeroId ());

  switch (quest->getType ())
    {
      case Quest::PILLAGEGOLD:
        draw_cities(true);
        break;
      case Quest::KILLARMIES:
      case Quest::KILLARMYTYPE:
        draw_cities(false);
        //for each target draw a plus sign
        draw_stacks(quest->getHero()->getPlayer(), targets);
        break;
      case Quest::KILLHERO:
      case Quest::CITYSACK:
      case Quest::CITYOCCUPY:
      case Quest::CITYRAZE:
      case Quest::RUINSEARCH:
        draw_cities(false);
        //the target list should only have one position in it
        //draw an orange line to the target and put a box around it.
        std::list< Vector<int> >::iterator it = targets.begin();
        if (targets.size() > 0)
          draw_target(start, *it);
        break;
    }

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
  SDL_FreeSurface (tmp);
    map_changed.emit(surface);
}

