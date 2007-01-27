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

#include <pgmessagebox.h>

#include "Bigmap.h"
#include "TempleDialog.h"
#include "SignpostDialog.h"
#include "CityDialog.h"
#include "RuinDialog.h"
#include "StackDialog.h"
#include "../GameMap.h"
#include "../maptile.h"
#include "../armysetlist.h"
#include "../templelist.h"
#include "../roadlist.h"
#include "../stonelist.h"
#include "../ruinlist.h"
#include "../signpostlist.h"
#include "../citylist.h"
#include "../playerlist.h"
#include "../stacklist.h"
#include "../File.h"
#include "../GraphicsCache.h"

E_Bigmap::E_Bigmap(PG_Widget* parent, PG_Rect rect)
    :PG_Widget(parent, rect, true), d_pressed(false)
{
    d_tilesize=GameMap::getInstance()->getTileSet()->getTileSize();
    d_rect.w = rect.w/d_tilesize;
    d_rect.h = rect.h/d_tilesize;
    d_rect.x = d_rect.y = 0;

    d_renderer = new MapRenderer(my_srfObject);

    // Load the pictures
    std::string tileset = GameMap::getInstance()->getTileSet()->getName();
    
    d_stackpic = File::getEditorPic("button_stack");
    d_ruinpic = File::getMapsetPicture(tileset, "misc/ruin.png");
    d_signpostpic = File::getMapsetPicture(tileset, "misc/signpost.png");
    d_templepic = File::getMapsetPicture(tileset, "misc/temples.png");
    d_stonepic = File::getMapsetPicture(tileset, "misc/stones.png");
    d_itempic = File::getMiscPicture("items.png");

    eventMouseLeave();
    centerView(PG_Point(0,0));
}

E_Bigmap::~E_Bigmap()
{
    if (d_renderer)
        delete d_renderer;
}

void E_Bigmap::centerView(const PG_Point pos)
{
    // the coordinates give us the center of the view!
    // if the view is already centered, do nothing
    if ((pos.x - d_rect.w/2 == d_rect.x) && (pos.y -d_rect.h/2 == d_rect.y))
        return;

    d_rect.x = pos.x - d_rect.w/2;
    d_rect.y = pos.y - d_rect.h/2;

    // if we would look outside of the map, center the rect accordingly
    if (d_rect.x < 0)
        d_rect.x = 0;
    if (d_rect.y < 0)
        d_rect.y = 0;
    if (d_rect.x + d_rect.w > GameMap::getWidth())
        d_rect.x = GameMap::getWidth() - d_rect.w;
    if (d_rect.y + d_rect.h > GameMap::getHeight())
        d_rect.y = GameMap::getHeight() - d_rect.h;

    // send the signal
    Redraw();
    schangingView.emit(PG_Point(d_rect.x+d_rect.w/2, d_rect.y+d_rect.h/2));
}

void E_Bigmap::changeStatus(STATUS status, Uint32 data)
{
    d_status = status;
    d_data = data;
}

void E_Bigmap::eventDraw(SDL_Surface* surface, const PG_Rect& rect)
{
    d_renderer->render(0, 0, d_rect.x, d_rect.y, d_rect.w, d_rect.h);

    // Draw temples
    Templelist* tl = Templelist::getInstance();
    for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        if (!d_rect.IsInside(pos))
            continue;

        PG_Rect r((pos.x-d_rect.x)*d_tilesize, (pos.y-d_rect.y)*d_tilesize,d_tilesize,d_tilesize);
        SDL_BlitSurface(GraphicsCache::getInstance()->getTemplePic(it->getType()), 0, surface, &r);
    }
    
    // Draw roads
    Roadlist* rol = Roadlist::getInstance();
    for (Roadlist::iterator it = rol->begin(); it != rol->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        if (!d_rect.IsInside(pos))
            continue;

        PG_Rect r((pos.x-d_rect.x)*d_tilesize, (pos.y-d_rect.y)*d_tilesize,d_tilesize,d_tilesize);
        SDL_BlitSurface(GraphicsCache::getInstance()->getRoadPic(it->getType()), 0, surface, &r);
    }
    
    // Draw stones
    Stonelist* sl = Stonelist::getInstance();
    for (Stonelist::iterator it = sl->begin(); it != sl->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        if (!d_rect.IsInside(pos))
            continue;

        PG_Rect r((pos.x-d_rect.x)*d_tilesize, (pos.y-d_rect.y)*d_tilesize,d_tilesize,d_tilesize);
        SDL_BlitSurface(GraphicsCache::getInstance()->getStonePic(it->getType()), 0, surface, &r);
    }
    
    // Draw Ruins
    Ruinlist* rl = Ruinlist::getInstance();
    for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        if (!d_rect.IsInside(pos))
            continue;
        
        PG_Rect r((pos.x-d_rect.x)*d_tilesize, (pos.y-d_rect.y)*d_tilesize,d_tilesize,d_tilesize);
        SDL_BlitSurface(d_ruinpic, 0, surface, &r);
    }

    // Draw Signposts
    Signpostlist* sp = Signpostlist::getInstance();
    for (Signpostlist::iterator it = sp->begin(); it != sp->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        if (!d_rect.IsInside(pos))
            continue;
        
        PG_Rect r((pos.x-d_rect.x)*d_tilesize, (pos.y-d_rect.y)*d_tilesize,d_tilesize,d_tilesize);
        SDL_BlitSurface(d_signpostpic, 0, surface, &r);
    }

    // Draw cities
    // Since we also want to draw cities partly ouside of the view, we need a larger
    // rectangle
    Citylist* cl = Citylist::getInstance();
    PG_Rect r;
    r.x = d_rect.x - 1;     r.y = d_rect.y - 1;
    r.w = d_rect.w + 2;     r.h = d_rect.h + 2;
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        if (!r.IsInside(pos))
            continue;
        
        PG_Rect r((pos.x-d_rect.x)*d_tilesize, (pos.y-d_rect.y)*d_tilesize,d_tilesize*2,d_tilesize*2);
        SDL_BlitSurface(GraphicsCache::getInstance()->getCityPic(&(*it)), 0, surface, &r);
    }

    // Draw stacks
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
    {
        Stacklist* sl = (*pit)->getStacklist();
        for (Stacklist::iterator it = sl->begin(); it != sl->end(); it++)
        {
            PG_Point pos = (*it)->getPos();
            if (!d_rect.IsInside(pos))
                continue;
            
            PG_Rect r((pos.x-d_rect.x)*d_tilesize+6, (pos.y-d_rect.y)*d_tilesize+6, 40, 40);
            SDL_BlitSurface((*it)->getStrongestArmy()->getPixmap(), 0, surface, &r);

            r.SetRect((pos.x-d_rect.x)*d_tilesize, (pos.y - d_rect.y)*d_tilesize,d_tilesize,d_tilesize);
            SDL_BlitSurface(GraphicsCache::getInstance()->getFlagPic(*it), 0, surface, &r);
        }
    }

    // Draw items if there are any items lying around
    for (int x = d_rect.x; x < d_rect.x+d_rect.w; x++)
        for (int y = d_rect.y; y < d_rect.y+d_rect.h; y++)
            if (!GameMap::getInstance()->getTile(x,y)->getItems().empty())
            {
                PG_Rect r((x-d_rect.x)*d_tilesize+40, (y-d_rect.y)*d_tilesize+40, 10, 10);
                SDL_BlitSurface(d_itempic, 0, surface, &r);
            }

    // finally, draw the border and the status
    drawStatus();
    DrawBorder(rect, 1);
}

int E_Bigmap::getRoadType()
{
     int type = 7;
     int x = d_pos.x/d_tilesize + d_rect.x;
     int y = d_pos.y/d_tilesize + d_rect.y;
     int u = 0, l = 0, r = 0, b = 0;
     Maptile *tile;
     Roadlist *rl = Roadlist::getInstance();
     /*  
      * We need to find out if x connects to upper (u), below (b), left (l), 
      * and right (right).
      *  u
      * lxr
      *  b
      */
     //do we connect to upper?
     //(i >= GameMap::getWidth()) || (j >= GameMap::getHeight()))

     if (y-1 >= 0)
     {
          tile = GameMap::getInstance()->getTile(x,y-1);
          if (tile && (tile->getBuilding() == Maptile::ROAD))
          {
	          switch (rl->getObjectAt(x,y-1)->getType())
	          {
		          case 1: case 2: case 5: case 6: 
		          case 7: case 8: case 10: case 11:
			          u = 1;
			          break;
	          }
          }
     }

     //do we connect to below?
     if (y+1 <= GameMap::getHeight()-1)
     {
          tile = GameMap::getInstance()->getTile(x,y+1);
          if (tile && (tile->getBuilding() == Maptile::ROAD))
          {
	          switch (rl->getObjectAt(x,y+1)->getType())
	          {
		          case 1: case 2: case 3: case 4: 
		          case 7: case 8: case 9: case 11:
			          b = 1;
			          break;
	          }
          }
     }

     //do we connect to left?
     if (x-1 >= 0)
     {
          tile = GameMap::getInstance()->getTile(x-1,y);
          if (tile && (tile->getBuilding() == Maptile::ROAD))
          {
	          switch (rl->getObjectAt(x-1,y)->getType())
	          {
		          case 0: case 2: case 4: case 5: 
		          case 7: case 8: case 9: case 10:
			          l = 1;
			          break;
	          }
          }
     }

     //do we connect to right?
     if (x+1 <= GameMap::getWidth()-1)
     {
          tile = GameMap::getInstance()->getTile(x+1,y);
          if (tile && (tile->getBuilding() == Maptile::ROAD))
          {
	          switch (rl->getObjectAt(x+1,y)->getType())
	          {
		          case 0: case 2: case 3: case 6: 
		          case 7: case 9: case 10: case 11:
			          r = 1;
			          break;
	          }
          }
     }

     if ((u == 0) && (b == 0) && (l == 0) && (r == 0))
	     type = 7;
     else if ((u == 1) && (b == 1) && (l == 1) && (r == 1))
	     type = 2;
     else if ((u == 0) && (b == 1) && (l == 1) && (r == 1))
	     type = 10;
     else if ((u == 1) && (b == 0) && (l == 1) && (r == 1))
	     type = 9;
     else if ((u == 1) && (b == 1) && (l == 0) && (r == 1))
	     type = 8;
     else if ((u == 1) && (b == 1) && (l == 1) && (r == 0))
	     type = 11;
     else if ((u == 1) && (b == 1) && (l == 0) && (r == 0))
	     type = 1;
     else if ((u == 0) && (b == 0) && (l == 1) && (r == 1))
	     type = 0;
     else if ((u == 1) && (b == 0) && (l == 1) && (r == 0))
	     type = 3;
     else if ((u == 1) && (b == 0) && (l == 0) && (r == 1))
	     type = 4;
     else if ((u == 0) && (b == 1) && (l == 1) && (r == 0))
	     type = 6;
     else if ((u == 0) && (b == 1) && (l == 0) && (r == 1))
	     type = 5;
     else if ((u == 1) && (b == 0) && (l == 0) && (r == 0))
	     type = 1;
     else if ((u == 0) && (b == 1) && (l == 0) && (r == 0))
	     type = 1;
     else if ((u == 0) && (b == 0) && (l == 1) && (r == 0))
	     type = 0;
     else if ((u == 0) && (b == 0) && (l == 0) && (r == 1))
	     type = 0;
     return type;
}

int E_Bigmap::getStoneType()
{
     int type;
     int tx = d_pos.x%d_tilesize;
     int ty = d_pos.y%d_tilesize;
     if (tx > 43)
     {
          if (ty > 43)
               type = 5;
          else if (ty > 21)
               type = 3;
          else
               type = 2;
     }
     else if (tx > 21)
     {
          if (ty > 43)
               type = 6;
          else if (ty > 21)
               type = 4;
          else
               type = 1;
     }
     else
     {
          if (ty > 43)
               type = 7; 
          else if (ty > 21)
               type = 8;
          else
               type = 0;
     }
     return type;
}

void E_Bigmap::drawStatus()
{
    // nothing to do
    if ((d_status == NONE) || (d_pos.x < 0) || d_pos.y < 0
                           || (d_pos.x >= my_width) || (d_pos.y >= my_height))
        return;

    int x = d_pos.x - (d_pos.x % d_tilesize);
    int y = d_pos.y - (d_pos.y % d_tilesize);

    // In case of a 1x1 area, draw a rect around the maptile under the cursor.
    // To make things simpler, dpos gives the mouse position in relative
    // coordinates
    if (d_status == AREA1x1)
        DrawRectWH(x, y, d_tilesize, d_tilesize, PG_Color(20, 20, 20));

    // In case of a 3x3 area, draw a rect around the maptile under the cursor
    // and the maptiles next to it.
    if (d_status == AREA3x3)
        for (int i = -1; i < 2; i++)
            for (int j = -1; j < 2; j++)
                DrawRectWH(x+i*d_tilesize, y+j*d_tilesize, d_tilesize, d_tilesize, PG_Color(20, 20, 20));

    // In case of erase, draw a red rectangle at the location of the cursor
    if (d_status == ERASE)
        DrawRectWH(x, y, d_tilesize, d_tilesize, PG_Color(200, 50, 50));
    
    // In the case of a city placement, draw a city
    if (d_status == CITY)
    {
        Player* neutral = Playerlist::getInstance()->getNeutral();
        SDL_Surface* citypic = GraphicsCache::getInstance()->getCityPic(0, neutral);

        PG_Rect r(x,y,d_tilesize*2,d_tilesize*2);
        SDL_BlitSurface(citypic, 0, my_srfObject, &r);
    }

    // in case of ruin placement, draw a ruin
    if (d_status == RUIN)
    {
        PG_Rect r(x,y,d_tilesize,d_tilesize);
        SDL_BlitSurface(d_ruinpic, 0, my_srfObject, &r);
    }

    // in case of sign placement, draw a signpost
    if (d_status == SIGNPOST)
    {
        PG_Rect r(x,y,d_tilesize,d_tilesize);
        SDL_BlitSurface(d_signpostpic, 0, my_srfObject, &r);
    }

    // in case of a temple, draw a temple picture
    if (d_status == TEMPLE)
    {
        SDL_Surface* templepic = GraphicsCache::getInstance()->getTemplePic(0);

        PG_Rect r(x,y,d_tilesize,d_tilesize);
        SDL_BlitSurface(templepic, 0, my_srfObject, &r);
    }

    // in case of a road, draw a road picture
    if (d_status == ROAD)
    {
        SDL_Surface* roadpic = GraphicsCache::getInstance()->getRoadPic(7);

        PG_Rect r(x,y,d_tilesize,d_tilesize);
        SDL_BlitSurface(roadpic, 0, my_srfObject, &r);
    }

    // in case of a stone, draw a stone picture
    if (d_status == STONE)
    {
	int type;
	type = getStoneType(); // which stone pic shall we show?
        SDL_Surface* stonepic = GraphicsCache::getInstance()->getStonePic(type);
        PG_Rect r(x,y,d_tilesize,d_tilesize);
        SDL_BlitSurface(stonepic, 0, my_srfObject, &r);
        DrawRectWH(x, y, d_tilesize, d_tilesize, PG_Color(200, 50, 50));
    }

    // in case of a stack, draw an idealistic stack image
    if (d_status == STACK)
    {
        PG_Rect r(x,y,d_tilesize,d_tilesize);
        SDL_BlitSurface(d_stackpic, 0, my_srfObject, &r);
    }
}

void E_Bigmap::changeMap()
{
    // nothing to do
    if ((d_status == NONE) || (d_pos.x < 0) || (d_pos.y < 0)
                           || (d_pos.x >= my_width) || (d_pos.y > my_height))
        return;

    int x = d_pos.x/d_tilesize + d_rect.x;
    int y = d_pos.y/d_tilesize + d_rect.y;

    // in case of a 1x1 area, change the terrain at cursor position, smooth all
    // terrain around it and return.
    if (d_status == AREA1x1)
    {
        // Don't change terrain to water if there is a building on the location,
        // don't change the terrain to anything else than grass if there is a
        // city.
        Maptile* tile = GameMap::getInstance()->getTile(x,y);
        TileSet* ts = GameMap::getInstance()->getTileSet();
        Tile::Type tiletype = (*ts)[d_data]->getType();
        
        if ((tile->getBuilding() != Maptile::NONE) && (tiletype == Tile::WATER))
            return;
        if ((tile->getBuilding() == Maptile::CITY) && (tiletype != Tile::GRASS))
            return;

        tile->setType(d_data);

        // we expect the renderer to catch out of bound errors
        for (int i = x-1; i <= x+1; i++)
            for (int j = y-1; j <= y+1; j++)
                d_renderer->smooth(i, j);
    }

    // in case of a 3x3 area, change the terrain at and around cursor position,
    // smooth it including the adjacent terrain and return. This is almost the
    // same as a 1x1 area, just with other bounds.
    if (d_status == AREA3x3)
    {
        TileSet* ts = GameMap::getInstance()->getTileSet();
        Tile::Type tiletype = (*ts)[d_data]->getType();

        for (int i = x-1; i <= x+1; i++)
            for (int j = y-1; j <= y+1; j++)
            {
                if ((i < 0) || (j < 0) || (i >= GameMap::getWidth()) || (j >= GameMap::getHeight()))
                    continue;

                // Don't change terrain to water if there is a building on the location,
                // don't change the terrain to anything else than grass if there is a
                // city.
                Maptile* tile = GameMap::getInstance()->getTile(i,j);
                
                if ((tile->getBuilding() != Maptile::NONE) && (tiletype == Tile::WATER))
                    return;
                if ((tile->getBuilding() == Maptile::CITY) && (tiletype != Tile::GRASS))
                    return;
                
                tile->setType(d_data);
            }

        for (int i = x-2; i <= x+2; i++)
            for (int j = y-2; j <= y+2; j++)
                d_renderer->smooth(i,j);
    }

    // in case of erase, check if there is a building or a stack there
    // and remove it.
    if (d_status == ERASE)
    {
        Maptile* tile = GameMap::getInstance()->getTile(x,y);

        // remove a stack, because it is above everything else ...
        Stack* s = Stacklist::getObjectAt(x,y);
        if (s != 0)
        {
            s->getPlayer()->deleteStack(s);
            return;
        }

        
        tile->setBuilding(Maptile::NONE);
        
        // ... or a temple ...
        Templelist* ts = Templelist::getInstance();
        if (ts->getObjectAt(x,y) != 0)
            for (Templelist::iterator it = ts->begin(); it != ts->end(); it++)
                if ((*it).getPos().x == x && (*it).getPos().y == y)
                {
                    ts->erase(it);
                    return;
                }

        // ... or a stone ...
        Stonelist* ss = Stonelist::getInstance();
        if (ss->getObjectAt(x,y) != 0)
            for (Stonelist::iterator it = ss->begin(); it != ss->end(); it++)
                if ((*it).getPos().x == x && (*it).getPos().y == y)
                {
                    ss->erase(it);
                    return;
                }

        // ... or a ruin ...
        Ruinlist* rs = Ruinlist::getInstance();
        if (rs->getObjectAt(x,y) != 0)
            for (Ruinlist::iterator it = rs->begin(); it != rs->end(); it++)
                if ((*it).getPos().x == x && (*it).getPos().y == y)
                {
                    rs->erase(it);
                    return;
                }

        // ... or a road ...
        Roadlist* rl = Roadlist::getInstance();
        if (rl->getObjectAt(x,y) != 0)
            for (Roadlist::iterator it = rl->begin(); it != rl->end(); it++)
                if ((*it).getPos().x == x && (*it).getPos().y == y)
                {
                    rl->erase(it);
                    return;
                }

        // ... or a signpost ...
        Signpostlist* sp = Signpostlist::getInstance();
        if (sp->getObjectAt(x,y) != 0)
            for (Signpostlist::iterator it = sp->begin(); it != sp->end(); it++)
                if ((*it).getPos().x == x && (*it).getPos().y == y)
                {
                    sp->erase(it);
                    return;
                }

        // ... else it must be a city
        Citylist* cl = Citylist::getInstance();
        for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        {
            // consider that cities span 2x2 tiles
            PG_Point pos = (*it).getPos();
            if ((pos.x != x && pos.x != x+1 && pos.x != x-1)
                || (pos.y != y && pos.y != y+1 && pos.y != y-1))
                continue;

            cl->erase(it);
            GameMap::getInstance()->getTile(pos.x,pos.y)->setBuilding(Maptile::NONE);
            GameMap::getInstance()->getTile(pos.x+1,pos.y)->setBuilding(Maptile::NONE);
            GameMap::getInstance()->getTile(pos.x,pos.y+1)->setBuilding(Maptile::NONE);
            GameMap::getInstance()->getTile(pos.x+1,pos.y+1)->setBuilding(Maptile::NONE);
            return;
        }
    }

    // in case of a city, first check if any other building has already been
    // placed there and the coordinates are ok, then place the city and change
    // the underlying terrain to grass.
    if (d_status == CITY)
    {
        // check if we can place the city
        for (int i = x; i <= x+1; i++)
            for (int j = y; j <= y+1; j++)
            {
                if ((i >= GameMap::getWidth()) || (j >= GameMap::getHeight()))
                    return;
                if (GameMap::getInstance()->getTile(i,j)->getBuilding() != Maptile::NONE)
                    return;
            }


        // create the city
        City c(PG_Point(x,y));
        c.setPlayer(Playerlist::getInstance()->getNeutral());
        Citylist::getInstance()->push_back(c);

        // find the index of the "grass" tile
        TileSet* ts = GameMap::getInstance()->getTileSet();
        unsigned int index;
        for (index = 0; index < ts->size(); index++)
            if ((*ts)[index]->getType() == Tile::GRASS)
                break;
        
        // notify the maptiles that a city has been placed here
        for (int i = x; i <= x+1; i++)
            for (int j = y; j <= y+1; j++)
            {
                Maptile* tile = GameMap::getInstance()->getTile(i,j);
                tile->setBuilding(Maptile::CITY);
                tile->setType(index);
            }

        // finally, smooth the surrounding map
        for (int i = x-1; i <= x+2; i++)
            for (int j = y-1; j <= y+2; j++)
               d_renderer->smooth(i,j);
    }

    // in case of a ruin, check if there is already another building or water
    // there and place the ruin.
    if (d_status == RUIN)
    {
        Maptile* tile = GameMap::getInstance()->getTile(x,y);
        if ((tile->getBuilding() != Maptile::NONE) 
           || (tile->getMaptileType() == Tile::WATER))
            return;
        
        tile->setBuilding(Maptile::RUIN);
        Ruinlist::getInstance()->push_back(Ruin(PG_Point(x,y)));
    }

    // in case of a signpost, check if there is already another building or 
    // water there and place the signpost.
    if (d_status == SIGNPOST)
    {
        Maptile* tile = GameMap::getInstance()->getTile(x,y);
        if ((tile->getBuilding() != Maptile::NONE) 
           || (tile->getMaptileType() != Tile::GRASS))
            return;
        
        tile->setBuilding(Maptile::SIGNPOST);
        Signpostlist::getInstance()->push_back(Signpost(PG_Point(x,y)));
    }

    // in case of a temple, check if there is already another building or water
    // there and place the temple.
    if (d_status == TEMPLE)
    {
        Maptile* tile = GameMap::getInstance()->getTile(x,y);
        if ((tile->getBuilding() != Maptile::NONE) 
           || (tile->getMaptileType() == Tile::WATER))
            return;
        
        tile->setBuilding(Maptile::TEMPLE);
        Templelist::getInstance()->push_back(Temple(PG_Point(x,y)));
    }

    // in case of a stone, check if there is already another building or water
    // there and place the stone.  Replace the stone if there's already one
    // there.
    if (d_status == STONE)
    {
	int type;
        Maptile* tile = GameMap::getInstance()->getTile(x,y);
        if (tile->getMaptileType() != Tile::GRASS)
            return;
        
	type = getStoneType();
	if (tile->getBuilding() == Maptile::STONE)
	     Stonelist::getInstance()->getObjectAt(x,y)->setType(type);
	else if (tile->getBuilding() == Maptile::NONE)
	{
             tile->setBuilding(Maptile::STONE);
             Stonelist::getInstance()->push_back(Stone(PG_Point(x,y), "",type));
	}
    }

    if (d_status == ROAD)
    {
	int type = getRoadType();
        Maptile* tile = GameMap::getInstance()->getTile(x,y);
	//which type is it?
	//well it depends on the surrounding pieces.
	if (tile->getBuilding() == Maptile::ROAD)
	     Roadlist::getInstance()->getObjectAt(x,y)->setType(type);
	else if (tile->getBuilding() == Maptile::NONE)
	{
             tile->setBuilding(Maptile::ROAD);
             Roadlist::getInstance()->push_back(Road(PG_Point(x,y), "", type));
	}
    }

    // TODO: in case of a stack, we need to handle stack placement on water
    // somehow. On the other hand, one could refer to the creator's sanity...
    if (d_status == STACK)
    {
        if (Stacklist::getObjectAt(x,y))
                return;

        // Create a new dummy stack. As we don't want to have empty stacks
        // hanging around, I assume that the default armyset has at least 
        // one entry.
        Playerlist* pl = Playerlist::getInstance();
        Stack* s = new Stack(pl->getNeutral(), PG_Point(x,y));
        const Armysetlist* al = Armysetlist::getInstance();
        Army* a = new Army(*(al->getArmy(al->getStandardId(), 0)), pl->getNeutral());

        s->push_back(a);
        pl->getNeutral()->addStack(s);
    }

    schangingMap.emit(true);
}

bool E_Bigmap::eventMouseButtonDown(const SDL_MouseButtonEvent* ev)
{
    if (ev->button == SDL_BUTTON_LEFT)
    {
        d_pressed = true;

        d_pos.x = ev->x - my_xpos;
        d_pos.y = ev->y - my_ypos;

        changeMap();
        Redraw();
    }

    if (ev->button == SDL_BUTTON_RIGHT)
    {
        int xpos = (ev->x - my_xpos)/d_tilesize + d_rect.x;
        int ypos = (ev->y - my_ypos)/d_tilesize + d_rect.y;
        
        // first, we need to check what is located at the position
        Temple* t = Templelist::getInstance()->getObjectAt(xpos, ypos);
        Stone* st = Stonelist::getInstance()->getObjectAt(xpos, ypos);
        City* c = Citylist::getInstance()->getObjectAt(xpos, ypos);
        Ruin* r = Ruinlist::getInstance()->getObjectAt(xpos, ypos);
        Road* ro = Roadlist::getInstance()->getObjectAt(xpos, ypos);
        Signpost* sp = Signpostlist::getInstance()->getObjectAt(xpos, ypos);
        Stack* s = 0;
        Playerlist* pl = Playerlist::getInstance();
        for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
        {
            s = (*it)->getStacklist()->getObjectAt(xpos, ypos);
            if (s)
                break;
        }

        // Now we need to find out if two items are stacked on each other.
        // Assuming that the editor is mostly free of bugs and the user not
        // evil, thie only possibility is a stacking of a stack and another
        // item (city, ruin etc.). If this is the case, query the user what
        // to modify
        bool modifyStack = true;
        if (s && (t || r || c || sp || st || ro))
        {
            PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-75, 200, 150),
                             _("Ambiguity"),
                             _("Do you want to modify the stack or the object beneath?"),
                             PG_Rect(30, 110, 60, 30), _("Stack"),
                             PG_Rect(110, 110, 60, 30), _("Other"));
            mb.Show();
            modifyStack = (mb.RunModal() == 1);
            mb.Hide();
        }
        
        // if right-clicking on a stack, open the stack dialog
        if (s && modifyStack)
        {
            E_StackDialog d(this, PG_Rect(my_width/2-200, my_height/2-200, 400, 400), s);
            d.Show();
            d.RunModal();
            d.Hide();

            if (s->empty())
                s->getPlayer()->deleteStack(s);

            Redraw();
            return true;
        }
        
        // if right-clicked on a temple, open the temple dialog
        if (t != 0)
        {
            E_TempleDialog d(this, PG_Rect(my_width/2-150, my_height/2-100, 300, 200), t);
            d.Show();
            d.RunModal();
            d.Hide();
            Redraw();
            return true;
        }

        // if right-clicked on a city, open the city dialog
        if (c != 0)
        {
            E_CityDialog d(this, PG_Rect(my_width/2-250, 0, 500, 440), c);
            d.Show();
            d.RunModal();
            d.Hide();
            Redraw();
            return true;
        }

        // if right-clicking on a ruin, open the ruin dialog
        if (r != 0)
        {
            E_RuinDialog d(this, PG_Rect(my_width/2-150, my_height/2-100, 300, 200), r);
            d.Show();
            d.RunModal();
            d.Hide();
            Redraw();
            return true;
        }

        // if right-clicking on a signpost, open the ruin dialog
        if (sp != 0)
        {
            E_SignpostDialog d(this, PG_Rect(my_width/2-150, my_height/2-100, 300, 200), sp);
            d.Show();
            d.RunModal();
            d.Hide();
            Redraw();
            return true;
        }
    }
        
    
    return true;
}

bool E_Bigmap::eventMouseButtonUp(const SDL_MouseButtonEvent* ev)
{
    if (ev->button == SDL_BUTTON_LEFT)
        d_pressed = false;
    
    return true;
}

bool E_Bigmap::eventMouseMotion(const SDL_MouseMotionEvent* ev)
{
    // actualize the mouse position
    PG_Point oldpos = d_pos;
    d_pos.x = ev->x - my_xpos;
    d_pos.y = ev->y - my_ypos;

    // if the mouse has crossed a tile boundary while moving, redraw
    int diff_xtiles = d_pos.x/d_tilesize - oldpos.x/d_tilesize;
    int diff_ytiles = d_pos.y/d_tilesize - oldpos.y/d_tilesize;

    if ((diff_xtiles == 0) && (diff_ytiles == 0))
    {
	 // hmm no movement, have have we crossed 1/6 of a tile and we're
	 // currently dropping stones?
         if (d_status == STONE)
         {
              diff_xtiles = d_pos.x/(d_tilesize/6) - oldpos.x/(d_tilesize/6);
              diff_ytiles = d_pos.y/(d_tilesize/6) - oldpos.y/(d_tilesize/6);
              if ((diff_xtiles == 0) && (diff_ytiles == 0))
	 	   return true;
         }
	 else
		 return true;
    }

    // if the mouse button is pressed, change the terrain under the mouse cursor
    if (d_pressed)
        changeMap();
    
    Redraw();

    smovingMouse.emit(PG_Point(d_pos.x/d_tilesize+d_rect.x, d_pos.y/d_tilesize+d_rect.y));
    
    return true;
}

void E_Bigmap::eventMouseLeave()
{
    //set the pos value to a negative value, so the status is not drawn
    d_pos.x = -51;
    d_pressed = false;
    smovingMouse.emit(PG_Point(-1,-1));
    Redraw();
}
