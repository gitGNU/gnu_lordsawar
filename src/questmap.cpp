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

QuestMap::QuestMap(Quest *q)
{
    quest = q;
}

void QuestMap::after_draw()
{
  Playerlist *plist = Playerlist::getInstance();
  Player *p = plist->getActiveplayer();
  Stacklist *sl = p->getStacklist();
  GraphicsCache *gc = GraphicsCache::getInstance();

  OverviewMap::after_draw();

  // draw the hero picture
  Vector<int> start = sl->getPosition (quest->getHeroId ());

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

  switch (quest->getType ())
    {
      case Quest::KILLARMIES:
      case Quest::KILLARMYTYPE:
        //draw a plus sign for every enemy army
        break;
      case Quest::KILLHERO:
      case Quest::CITYSACK:
      case Quest::CITYOCCUPY:
      case Quest::CITYRAZE:
      case Quest::RUINSEARCH:
        //draw an orange line to the target and put a box around it.
        break;
    }
    map_changed.emit(surface);
}

