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

#include <config.h>

#include <SDL_image.h>
#include <assert.h>
#include <stdlib.h>

#include "gamebigmap.h"

#include "army.h"
#include "path.h"
#include "stacklist.h"
#include "stack.h"
#include "citylist.h"
#include "ruinlist.h"
#include "signpostlist.h"
#include "templelist.h"
#include "roadlist.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "road.h"
#include "playerlist.h"
#include "defs.h"
#include "File.h"
#include "GameMap.h"
#include "GameScenario.h"
#include "GraphicsCache.h"
#include "game.h"

#include "timing.h"


#include <iostream>
using namespace std;
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

namespace 
{
    int selection_timeout = 150;	// controls speed of selector rotation
}

GameBigMap::GameBigMap()
{
  current_tile.x = current_tile.y = 0;
  mouse_state = NONE;
  input_locked = false;

  d_waypoints = File::getMiscPicture("waypoints.png");
  prev_mouse_pos = Vector<int>(0, 0);

  // setup timeout
  selection_timeout_handler = 
    Timing::instance().register_timer(
				      sigc::mem_fun(*this, &GameBigMap::on_selection_timeout),
				      selection_timeout);
  shift_key_is_down = false;
}

GameBigMap::~GameBigMap()
{
  SDL_FreeSurface(d_waypoints);
}

void GameBigMap::select_active_stack()
{
  Stack* stack = Playerlist::getActiveplayer()->getActivestack();
  if (!stack)
    return;

  if (!stack->getPath()->checkPath(stack))
    {
      //error handling is required here, up to now we only barf on cerr
      cerr << _("original path of stack was blocked\n");
    }

  stack_selected.emit(stack);
}

void GameBigMap::unselect_active_stack()
{
  draw();
  stack_selected.emit(0);
}

bool GameBigMap::on_selection_timeout()
{
  // redraw to update the selection
  if (Playerlist::getActiveplayer()->getActivestack())
    draw();

  return Timing::CONTINUE;
}

void GameBigMap::mouse_button_event(MouseButtonEvent e)
{
  if (input_locked)
    return;

  Vector<int> tile = mouse_pos_to_tile(e.pos);

  if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::PRESSED)
    {
      Stack* stack = Playerlist::getActiveplayer()->getActivestack();

      if (stack)
	{
	  // ask for military advice
	  if (d_cursor == GraphicsCache::QUESTION)
	    {
	      Playerlist::getActiveplayer()->stackFightAdvise(stack, tile); 
	      set_shift_key_down (false);
	      return;
	    }
	  Vector<int> p;
	  p.x = tile.x; p.y = tile.y;

	  // clicked on the already active stack
	  if (stack->getPos().x == tile.x && stack->getPos().y == tile.y)
	    {
	      // clear the path
	      stack->getPath()->flClear();
	      path_set.emit();
	      draw();
	      return;
	    }

	  //clicked on an enemy city that is too far away
	  City *c = Citylist::getInstance()->getObjectAt(tile);
	  if (c)
	    {
	      //restrict going into enemy cities unless they're only
	      //one square away
	      if (c->getPlayer() != Playerlist::getActiveplayer())
		{
		  int delta = abs(tile.x - stack->getPos().x);
		  if (delta <= 1)
		    delta = abs(tile.y - stack->getPos().y);
		  if (delta > 1)
		    return;
		}
	    }

	  // split if ungrouped
	  Playerlist::getActiveplayer()->stackSplit(stack); 

	  stack->getPath()->calculate(stack, p);

	  Vector<int>* dest = 0;
	  if (!stack->getPath()->empty())
	    dest = *stack->getPath()->rbegin();

	  if (dest && dest->x == tile.x && dest->y == tile.y)
	    {
	      Playerlist::getActiveplayer()->stackMove(stack);
	      if (!Playerlist::getActiveplayer()->getActivestack())
		return;
	      else
		{
		  //grab our stack again because maybe we joined another stack
		  stack = Playerlist::getActiveplayer()->getActivestack();
		  if (stack->canMove() == false)
		    {
		      Player *player = Playerlist::getActiveplayer();
		      player->getStacklist()->setActivestack(0);
		      unselect_active_stack();
		    }
		}
	    }
	  //else
	    //stack->getPath()->calculate(stack, p);

	  path_set.emit();

	  draw();
	}
      // Stack hasn't been active yet
      else
	{
	  stack = Stacklist::getObjectAt(tile.x, tile.y);
	  if (stack && stack->isFriend(Playerlist::getActiveplayer()))
	    {
	      Playerlist::getActiveplayer()->getStacklist()->setActivestack(stack);
	      select_active_stack();
	    }
	  else
	    {
	      if (City* c = Citylist::getInstance()->getObjectAt(tile.x, tile.y))
		{
		  if (!c->isBurnt())
		    {
		      if (GameScenario::s_see_opponents_production == true)
			city_queried (c, false);
		      else
			{
			  if (c->getPlayer() == Playerlist::getActiveplayer())
			    city_queried (c, false);
			}
		    }
		}
	      else if (Ruin *r = Ruinlist::getInstance()->getObjectAt(tile))
		{
		  ruin_queried (r, false);
		}
	      else if (Temple *t = Templelist::getInstance()->getObjectAt(tile))
		{
		  temple_queried (t, false);
		}
	    }
	}
    }


  // right mousebutton to get information about things on the map and to
  // unselect the active stack
  else if (e.button == MouseButtonEvent::RIGHT_BUTTON)
    {
      if (e.state == MouseButtonEvent::PRESSED)
	{
	  if (City* c = Citylist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
	      city_queried (c, true);
	      mouse_state = SHOWING_CITY;
	    }
	  else if (Ruin* r = Ruinlist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
	      if ((r->isHidden() == true && 
		   r->getOwner() == Playerlist::getActiveplayer()) ||
		  r->isHidden() == false)
		{
		  ruin_queried (r, true);
		  mouse_state = SHOWING_RUIN;
		}
	    }
	  else if (Signpost* s = Signpostlist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
	      signpost_queried (s);
	      mouse_state = SHOWING_SIGNPOST;
	    }
	  else if (Temple* t = Templelist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
	      temple_queried.emit(t, true);
	      mouse_state = SHOWING_TEMPLE;
	    }
	  else if (Stack *st = Stacklist::getObjectAt(tile.x, tile.y))
	    {
	      if (GameScenario::s_see_opponents_stacks == true)
		{
		  stack_queried.emit(st);
		  mouse_state = SHOWING_STACK;
		}
	      else if (st->getPlayer() == Playerlist::getActiveplayer() && 
		       GameScenario::s_see_opponents_stacks == false)
		{
		  stack_queried.emit(st);
		  mouse_state = SHOWING_STACK;
		}
	    }
	}
      else // button released
	{
	  switch(mouse_state)
	    {
	    case DRAGGING:
	      break;

	    case SHOWING_CITY:
	      city_queried.emit(0, true);
	      break;

	    case SHOWING_RUIN:
	      ruin_queried.emit(0, true);
	      break;

	    case SHOWING_TEMPLE:
	      temple_queried.emit(0, true);
	      break;

	    case SHOWING_SIGNPOST:
	      signpost_queried.emit(0);
	      break;

	    case SHOWING_STACK:
	      stack_queried.emit(0);
	      break;

	    case NONE:
	      Stack* stack = Playerlist::getActiveplayer()->getActivestack();
	      if (stack)
		{
		  Playerlist::getActiveplayer()->getStacklist()->setActivestack(0);
		  unselect_active_stack();
		}
	      break;
	    }

	  // in any case reset mouse state
	  mouse_state = NONE;
	}
    }
}

void GameBigMap::determine_mouse_cursor(Stack *stack, Vector<int> tile)
{
  if (stack)
    {
      d_cursor = GraphicsCache::FEET;
      if (stack->getPos() == tile)
	d_cursor = GraphicsCache::TARGET;
      else
	{
	  City *c = Citylist::getInstance()->getObjectAt(tile);
	  if (c)
	    {
	      if (c->getPlayer() == Playerlist::getActiveplayer())
		d_cursor = GraphicsCache::FEET;
	      else if (c->isBurnt() == true)
		d_cursor = GraphicsCache::FEET;
	      else
		{
		  int delta = abs(tile.x - stack->getPos().x);
		  if (delta <= 1)
		    delta = abs(tile.y - stack->getPos().y);
		  if (delta <= 1)
		    {
		      if (is_shift_key_down())
			d_cursor = GraphicsCache::QUESTION;
		      else
			{
			  bool friendly = false;
			  if (friendly)
			    d_cursor = GraphicsCache::HEART;
			  else
			    d_cursor = GraphicsCache::SWORD;
			}
		    }
		  else
		    {
		      //can i see other ppl's cities?
		      if (GameScenario::s_see_opponents_production == true)
			d_cursor = GraphicsCache::ROOK;
		      else
			d_cursor = GraphicsCache::HAND;
		    }
		}
	    }
	  else
	    {
	      Maptile *t = GameMap::getInstance()->getTile(tile);
	      Stack *st;
	      st = Stacklist::getObjectAt(tile);
	      if (!st)
		{
		  Stack *empty = new Stack (*stack);
		  if (empty->getPath()->calculate(empty, tile) == 0)
		    d_cursor = GraphicsCache::HAND;
		  else
		    {
		      if (t->getMaptileType() == Tile::WATER)
			d_cursor = GraphicsCache::SHIP;
		      else
			d_cursor = GraphicsCache::FEET;
		    }
		  delete empty;
		}
	      else
		{
		  if (st->getPlayer() != Playerlist::getActiveplayer())
		    {
		      int delta = abs(stack->getPos().x - st->getPos().x);
		      if (delta <= 1)
			delta = abs(stack->getPos().y - st->getPos().y);
		      if (delta <= 1)
			{
			  if (is_shift_key_down())
		    	    d_cursor = GraphicsCache::QUESTION;
			  else
			    {
			      bool friendly = false;
			      if (friendly)
				d_cursor = GraphicsCache::HEART;
			      else
				d_cursor = GraphicsCache::SWORD;
			    }
			}
		      else
			d_cursor = GraphicsCache::HAND;
		    }
		}

	    }
	}
    }
  else
    {
      d_cursor = GraphicsCache::HAND;
      Stack *st;

      st = Playerlist::getActiveplayer()->getStacklist()->getObjectAt(tile);
      if (st)
	{
	  if (st->getPlayer() == Playerlist::getActiveplayer())
	    d_cursor = GraphicsCache::TARGET;
	  else
	    d_cursor = GraphicsCache::HAND;
	}
      else
	{
	  Maptile *t = GameMap::getInstance()->getTile(tile);
	  if (t->getBuilding() == Maptile::CITY)
	    {
	      City *c = Citylist::getInstance()->getObjectAt(tile);
	      if (c->isBurnt() == true)
		d_cursor = GraphicsCache::FEET;
	      else if (c->getPlayer() == Playerlist::getActiveplayer())
		d_cursor = GraphicsCache::ROOK;
	      else if (GameScenario::s_see_opponents_production == true)
		d_cursor = GraphicsCache::ROOK;
	    }
	  else if (t->getBuilding() == Maptile::RUIN)
	    d_cursor = GraphicsCache::RUIN;
	  else if (t->getBuilding() == Maptile::TEMPLE)
	    d_cursor = GraphicsCache::RUIN;
	}

    }
  cursor_changed.emit(d_cursor);
}

void GameBigMap::mouse_motion_event(MouseMotionEvent e)
{
  static Vector<int> last_tile;
  if (input_locked)
    return;

  // drag with left mouse button
  if (e.pressed[MouseMotionEvent::LEFT_BUTTON]
      && (mouse_state == NONE || mouse_state == DRAGGING))
    {
      Vector<int> delta = -(e.pos - prev_mouse_pos);

      // ignore very small drags to ensure that a shaking mouse does not
      // prevent the user from making right clicks
      if (mouse_state == NONE && length(delta) <= 2)
	return;

      int ts = GameMap::getInstance()->getTileSet()->getTileSize();
      SDL_Surface *screen = SDL_GetVideoSurface();
      Vector<int> screen_dim(screen->w, screen->h);
      view_pos = clip(Vector<int>(0, 0),
		      view_pos + delta,
		      GameMap::get_dim() * ts - screen_dim);

      // calculate new view position in tiles, rounding up
      Vector<int> new_view = (view_pos + Vector<int>(ts - 1, ts - 1)) / ts;

      bool redraw_buffer = false;

      if (new_view != view.pos)
	{
	  view.x = new_view.x;
	  view.y = new_view.y;
	  view_changed.emit(view);
	  redraw_buffer = true;
	}

      draw(redraw_buffer);
      mouse_state = DRAGGING;
    }

  // the following block of code shows the correct mouse cursor
  Stack* stack = Playerlist::getActiveplayer()->getActivestack();
  Vector<int> tile = mouse_pos_to_tile(e.pos);
  if (tile == last_tile)
    {
      prev_mouse_pos = e.pos;
      return;
    }
  determine_mouse_cursor (stack, tile);

  prev_mouse_pos = e.pos;
  last_tile = tile;
}

void GameBigMap::after_draw()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();

  Stack* stack = Playerlist::getActiveplayer()->getActivestack();

  // Draw Path
  if (stack && stack->getPath()->size())
    {
      Vector<int> pos;
      SDL_Color c;
      c.r = c.g = c.b = 0;

      // draw all waypoints
      Uint32 pathcount = 0;
      bool canMoveThere = true;
      for (list<Vector<int>*>::iterator it = stack->getPath()->begin();
	   it != stack->getPath()->end(); it++)
	{
	  size_t wpsize = 40; //waypoint images are always 40x40
	  pos = tile_to_buffer_pos(**it);
	  SDL_Rect r1, r2;
	  r1.y = 0;
	  r1.w = r1.h = wpsize; 
	  int offset = (tilesize - wpsize) / 2;
	  if (offset < 0)
	    offset = 0;
	  r2.x = pos.x + offset;
	  r2.y = pos.y + offset;
	  r2.w = r2.h = wpsize;

	  //fixme: check to see if we can move here
	  canMoveThere = (pathcount < stack->getMovesExhaustedAtPoint());
	  if (canMoveThere)
	    r1.x = 0;
	  else
	    r1.x = wpsize;

	  SDL_BlitSurface(d_waypoints, &r1, buffer, &r2);
	  pathcount++;

	}

    }

  if (stack)
    {
      // draw the selection
      Vector<int> p = stack->getPos();
      if (is_inside(buffer_view, Vector<int>(p.x, p.y)))
	{
	  static int bigframe = -1;
	  static int smallframe = -1;

	  bigframe++;
	  if (bigframe > 5)
	    bigframe = 0;

	  smallframe++;
	  if (smallframe > 3)
	    smallframe = 0;

	  p = tile_to_buffer_pos(p);
	  SDL_Rect r;
	  r.x = p.x;
	  r.y = p.y;
	  r.w = r.h = tilesize;
	  SDL_Surface *tmp;
	  int num_selected = 0;
	  for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
	    {
	      if ((*it)->isGrouped())
		num_selected++;
	    }

	  draw_stack (stack);
	  if (num_selected > 1)
	    tmp = gc->getSelectorPic(0, bigframe, stack->getPlayer());
	  else
	    tmp = gc->getSelectorPic(1, smallframe, stack->getPlayer());
	  SDL_BlitSurface(tmp, 0, buffer, &r);
	}
    }
}

void GameBigMap::set_shift_key_down (bool down)
{
  if (GameScenario::s_military_advisor == false)
    return;
  static GraphicsCache::CursorType prev_cursor = GraphicsCache::HEART;

  Stack* stack = Playerlist::getActiveplayer()->getActivestack();
  if (!stack)
    return;

  if (d_cursor == GraphicsCache::HEART ||
      d_cursor == GraphicsCache::SWORD)
    {
      if (shift_key_is_down == false)
	{
	  shift_key_is_down = down;
	  prev_cursor = d_cursor;
	  d_cursor = GraphicsCache::QUESTION;
	  cursor_changed.emit(d_cursor);
	}
    }
  else if (d_cursor == GraphicsCache::QUESTION)
    {
      if (shift_key_is_down == true)
	{
	  shift_key_is_down = down;
	  d_cursor = prev_cursor;
	  cursor_changed.emit(d_cursor);
	}
    }
  else
    shift_key_is_down = down;
}

