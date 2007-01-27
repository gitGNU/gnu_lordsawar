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

#include "MapRenderer.h"
#include "GameMap.h"
#include "defs.h"
#include <SDL.h>
#include <string>

using namespace std;

MapRenderer::MapRenderer(SDL_Surface* surface)
{
    d_surface = surface;

    // UL: what was this for?
    // doesn't seem to do anything if i comment this out. ??
//    SDL_Surface* dummy = SDL_CreateRGBSurface(0, 64, 64, 16, 0, 0, 0, 0);
//    SDL_FreeSurface(dummy);

    for (int tilex = 0; tilex < GameMap::getInstance()->getWidth(); tilex++)
        for (int tiley = 0; tiley < GameMap::getInstance()->getHeight(); tiley++)
            smooth(tilex, tiley);
}
 

MapRenderer::~MapRenderer()
{
}

// set transitions

int MapRenderer::smoothCorner1(bool lt, bool t, bool l)
{
    if (lt && t && l) return 0;
    if (l && !t) return 4;
    if (t && !l) return 8;
    if (!t && !l) return 12;
    if (!lt && t && l) return 16;
    return 0;
}

int MapRenderer::smoothCorner2(bool t, bool rt, bool r)
{
    if (t && rt && r) return 1;
    if (!t && r) return 5;
    if (t && !r) return 9;
    if (!t && !r) return 13;
    if (t && !rt && r) return 17;
    return 1;
}

int MapRenderer::smoothCorner3(bool l, bool lb, bool b)
{
    if (l && lb && b) return 2;
    if (l && !b) return 6;
    if (!l && b) return 10;
    if (!l && !b) return 14;
    if (l && !lb && b) return 18;
    return 2;
}

int MapRenderer::smoothCorner4(bool r, bool b, bool rb)
{
    if (r && b && rb) return 3;
    if (r && !b) return 7;
    if (!r && b) return 11;
    if (!r && !b) return 15;
    if (r && b && !rb) return 19;
    return 3;
}

void MapRenderer::smooth(int x, int y)
{
    if ((x < 0) || (y < 0) || (x >= GameMap::getWidth()) || (y >= GameMap::getHeight()))
        return;
    
    Uint32 basetile = GameMap::getInstance()->getTile(x,y)->getType();
    bool l = true;
    bool r = true;
    bool t = true;
    bool b = true;
    bool lt = true;
    bool rt = true;
    bool lb = true;
    bool rb = true;

    // flags are set to true if tile is the same
    if (x-1 >= 0)
        l = (basetile == GameMap::getInstance()->getTile(x-1,y)->getType());
    if (x+1 <= GameMap::getWidth()-1)
        r = (basetile == GameMap::getInstance()->getTile(x+1,y)->getType());
    if (y-1 >= 0)
        t = (basetile == GameMap::getInstance()->getTile(x,y-1)->getType());
    if (y+1 <= GameMap::getHeight()-1)
        b = (basetile == GameMap::getInstance()->getTile(x,y+1)->getType());
    if (x-1 >= 0 && y-1 >= 0) 
        lt = (basetile == GameMap::getInstance()->getTile(x-1,y-1)->getType());
    if (x+1 <= GameMap::getWidth()-1 && y-1 >= 0) 
        rt = (basetile == GameMap::getInstance()->getTile(x+1,y-1)->getType());
    if (x-1 >= 0 && y+1 <= GameMap::getHeight()-1) 
        lb = (basetile == GameMap::getInstance()->getTile(x-1,y+1)->getType());
    if (x+1 <= GameMap::getWidth()-1 && y+1 <= GameMap::getHeight()-1) 
        rb = (basetile == GameMap::getInstance()->getTile(x+1,y+1)->getType());

    int corner[4];
    corner[0] = smoothCorner1(lt, t, l);
    corner[1] = smoothCorner2(t, rt, r);
    corner[2] = smoothCorner3(l, lb, b);
    corner[3] = smoothCorner4(r, b, rb);

    // use variation tiles if the result it the basetile
    if (corner[0] == 0 && corner[1] == 1 && corner[2] == 2 && corner[3] == 3)
    {
        // use one of the possible 3 variations
        int var = rand() % 4;
        // if var == 0 use base tile => no change needed
        if (var > 0)
        {
            for (int i = 0; i < 4; i++)
            {
                // variations start at corner 20
                corner[i] += 20 + (var-1)*4;
            }
        }
    }

    // set calculated corner tiles
    GameMap::getInstance()->getTile(x,y)->setCorners(corner[0], corner[1], corner[2], corner[3]);
}

void MapRenderer::render(int x, int y, int tileStartX, int tileStartY,
        int columns, int rows)
{
    SDL_Rect r;
    GameMap* map = GameMap::getInstance();
    r.w = r.h = map->getTileSet()->getTileSize() / 2;
    int drawY = y;
    
    for(int tileY = tileStartY; tileY < (tileStartY + rows); tileY++)
    {
        int drawX = x;
        for(int tileX = tileStartX; tileX < (tileStartX + columns); tileX++)
        {
            // get correct tile
            Tile* type = (*map->getTileSet())[map->getTile(tileX,tileY)->getType()];

            // render all 4 corners
            r.x = drawX;
            r.y = drawY;
            int corner = map->getTile(tileX,tileY)->getCorner(0);
            SDL_BlitSurface(type->getSurface(corner), 0, d_surface, &r);

            r.x = drawX+r.w;
            r.y = drawY;
            corner = map->getTile(tileX,tileY)->getCorner(1);
            SDL_BlitSurface(type->getSurface(corner), 0, d_surface, &r);

            r.x = drawX;
            r.y = drawY+r.h;
            corner = map->getTile(tileX,tileY)->getCorner(2);
            SDL_BlitSurface(type->getSurface(corner), 0, d_surface, &r);

            r.x = drawX + r.w;
            r.y = drawY + r.h;
            corner = map->getTile(tileX,tileY)->getCorner(3);
            SDL_BlitSurface(type->getSurface(corner), 0, d_surface, &r);

            drawX += map->getTileSet()->getTileSize();
        }
        drawY += map->getTileSet()->getTileSize();
    }

    // Now, with the implementation of the diagonal tiles, we need a second run.
    // Here, we have to check if we have typical diagonal transitions and if so,
    // blit the river-with-bridge structure over the terrain. We loop over two
    // tiles more since we always check for the lower right or lower left tile.
    int width = GameMap::getWidth();
    int height = GameMap::getHeight();
    r.w = r.h = map->getTileSet()->getTileSize();

    // shortcut: get the index of the water tile; this saves us a lot of
    // shuffling later
    Uint32 waterindex = 0;
    TileSet* ts = map->getTileSet();
    for (unsigned int i = 0; i < ts->size(); i++)
        if ((*ts)[i]->getType() == Tile::WATER)
        {
            waterindex = i;
            break;
        }
    
    drawY = y - ts->getTileSize();
    for (int tileY = tileStartY-1; tileY < (tileStartY + rows+1); tileY++)
    {
        int drawX = x - 2*ts->getTileSize();

        // we do not deal with the last row, this does not fit to the algorithm
        // (always look to the lower left or right for a target to apply
        if (tileY < 0 || tileY >= height-1)
        {
            drawY += ts->getTileSize();
            continue;
        }
        
        for (int tileX = tileStartX-1; tileX < (tileStartX + columns+1); tileX++)
        {
            drawX += ts->getTileSize();

            if (tileX < 0 || tileX >= width)
                continue;

            Uint32 type = map->getTile(tileX, tileY)->getType();
            
            // if the tile is no water, we ignore it; also, as we look for
            // diagonal transitions, the tile below the tile-in-view has to be
            // != water
            if (type != waterindex
                || map->getTile(tileX, tileY+1)->getType() == waterindex)
                continue;

            // Now we handle two cases. First, we look to the lower right if there
            // is a transition
            if (tileX < width-1 && map->getTile(tileX+1, tileY)->getType() != waterindex
                && map->getTile(tileX+1, tileY+1)->getType() == waterindex)
            {
                r.x = drawX + ts->getTileSize()/2;
                r.y = drawY + ts->getTileSize()/2;
                SDL_BlitSurface(ts->getDiagPic(0), 0, d_surface, &r);
            }

            // now we look for the lower left if there is a transition
            if (tileX > 0 && map->getTile(tileX-1,tileY)->getType() != waterindex
                && map->getTile(tileX-1, tileY+1)->getType() == waterindex)
            {
                r.x = drawX - ts->getTileSize()/2;
                r.y = drawY + ts->getTileSize()/2;
                SDL_BlitSurface(ts->getDiagPic(1), 0, d_surface, &r);
            }
        }
        drawY += ts->getTileSize();
    }
}

// End of file
