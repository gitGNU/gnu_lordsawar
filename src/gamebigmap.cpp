//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <config.h>

#include <SDL_image.h>
#include <assert.h>
#include <stdlib.h>
#include <glibmm/timeval.h>

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
#include "bridgelist.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "road.h"
#include "playerlist.h"
#include "File.h"
#include "GameMap.h"
#include "GraphicsCache.h"
#include "GraphicsLoader.h"
#include "game.h"
#include "FogMap.h"
#include "LocationBox.h"
#include "Configuration.h"

#include "timing.h"


#include <iostream>
using namespace std;
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

namespace 
{
    int selection_timeout = 150;	// controls speed of selector rotation
}

GameBigMap::GameBigMap(bool intense_combat, bool see_opponents_production,
		       bool see_opponents_stacks, bool military_advisor)
:d_fighting(LocationBox(Vector<int>(-1,-1)))
{
  d_intense_combat = intense_combat;
  d_see_opponents_production = see_opponents_production;
  d_see_opponents_stacks = see_opponents_stacks;
  d_military_advisor = military_advisor;

  current_tile.x = current_tile.y = 0;
  mouse_state = NONE;
  input_locked = false;

  d_waypoints = GraphicsLoader::getMiscPicture("waypoints.png");
  prev_mouse_pos = Vector<int>(0, 0);

  // setup timeout
  selection_timeout_handler = 
    Timing::instance().register_timer(
				      sigc::mem_fun(*this, &GameBigMap::on_selection_timeout),
				      selection_timeout);
  shift_key_is_down = false;
  control_key_is_down = false;
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
      cerr << "original path of stack was blocked\n";
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

  Player *active = Playerlist::getActiveplayer();
  Vector<int> tile = mouse_pos_to_tile(e.pos);
  current_tile = tile;

  if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::PRESSED)
    {
      bool double_clicked = false;
      static Glib::TimeVal last_clicked;
      if (last_clicked.as_double() == 0.0)
	last_clicked.assign_current_time();
      else
	{
	  Glib::TimeVal clicked_now;
	  clicked_now.assign_current_time();
	  double click_delta = clicked_now.as_double() - 
	    last_clicked.as_double();
	  if (click_delta <= Configuration::s_double_click_threshold / 1000.0)
	    double_clicked = true;
	  last_clicked = clicked_now;
	}
      if (active->getFogMap()->isCompletelyObscuredFogTile(tile) == true)
	return;

      Stack* stack = Playerlist::getActiveplayer()->getActivestack();

      if (d_cursor == GraphicsCache::HAND)
	return;

      if (stack)
	{
	  bool path_already_set = stack->getPath()->size() > 0;
	  // ask for military advice
	  if (d_cursor == GraphicsCache::QUESTION)
	    {
	      set_shift_key_down (false);
	      Playerlist::getActiveplayer()->stackFightAdvise
		(stack, tile, d_intense_combat); 
	      return;
	    }
	  else if (d_cursor == GraphicsCache::RUIN)
	    {
	      if (Ruin *r = Ruinlist::getInstance()->getObjectAt(tile))
		{
		  if ((r->isHidden() == true && 
		       r->getOwner() == Playerlist::getActiveplayer()) ||
		      r->isHidden() == false)
		    {
		      set_shift_key_down (false);
		      ruin_queried (r, false);
		    }
		}
	      else if (Temple *t = Templelist::getInstance()->getObjectAt(tile))
		{
		  temple_queried (t, false);
		  set_shift_key_down (false);
		}
	      return;
	    }
	  else if (d_cursor == GraphicsCache::ROOK)
	    {
	      City* c = Citylist::getInstance()->getObjectAt(tile);
	      if (c != NULL)
		{
		  if (!c->isBurnt())
		    {
		      set_control_key_down (false);
		      if (d_see_opponents_production == true)
			{
			  city_queried (c, false);
			  set_shift_key_down (false);
			  return;
			}
		      else
			{
			  if (c->getOwner() == Playerlist::getActiveplayer())
			    {
			      city_queried (c, false);
			      set_shift_key_down (false);
			      return;
			    }
			}
		    }
		}
	    }
	  Vector<int> p;
	  p.x = tile.x; p.y = tile.y;

	  // clicked on the already active stack
	  if (stack->getPos() == tile)
	    {
	      if (double_clicked == true)
		{
		  if (stack->isGrouped() == true)
		    stack->ungroup();
		  else
		    stack->group();
		  draw();
		  stack_grouped_or_ungrouped.emit(stack);
		  return;
		}
	      else
		{
		  // clear the path
		  stack->getPath()->flClear();
		  path_set.emit();
		  draw();
		  return;
		}
	    }

	  //clicked on an enemy city that is too far away
	  City *c = Citylist::getInstance()->getObjectAt(tile);
	  if (c)
	    {
	      //restrict going into enemy cities unless they're only
	      //one square away
	      if (c->getOwner() != Playerlist::getActiveplayer())
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

	  int dist = stack->getPath()->calculate(stack, p);
	  if (dist == -2)
	    cerr << "error calculating path!";

	  Vector<int>* dest = 0;
	  if (!stack->getPath()->empty())
	    dest = *stack->getPath()->rbegin();

	  if (dest && dest->x == tile.x && dest->y == tile.y)
	    {
	      Playerlist::getActiveplayer()->stackMove(stack);
	      if (!Playerlist::getActiveplayer()->getActivestack())
		{
		  unselect_active_stack();
		  return;
		}
	      else
		{
		  //grab our stack again because maybe we joined another stack
		  stack = Playerlist::getActiveplayer()->getActivestack();

		  //deslect when:
		  //1. we've moved our stack too far and we've gone as far
		  //   as we can on our path.
		  //2. we've set in a second path and we've gone as far as
		  //   we can on our path.
		  //3. we're next to an enemy city, out of moves, and we 
		  //   try to attack the city.
		  //note that special care is taken to not deselect when
		  //we've proceeded along our path and ran out of nodes
		  //to follow, but we still have moves to make.
		  bool deselect = false;
		  if (path_already_set)
		    {
		      if (!stack->getPath()->empty() && 
			  stack->enoughMoves() == false)
			deselect = true;
		      else if (!stack->getPath()->empty() &&
			       stack->getPath()->getMovesExhaustedAtPoint() == 0)
			deselect = true;
		    }
		  else
		    {
		      if (stack->getPath()->empty() == false && 
			  stack->getPath()->getMovesExhaustedAtPoint() == 0)
			deselect = true;
		      if ((d_cursor == GraphicsCache::SWORD ||
			   d_cursor == GraphicsCache::HEART) &&
			  stack->canMove() == false)
			deselect = true;
		      if ((d_cursor == GraphicsCache::FEET ||
			   d_cursor == GraphicsCache::SHIP) &&
			  stack->canMove() == false)
			deselect = true;
		    }

		  if (deselect)
		    {
		      Player *player = Playerlist::getActiveplayer();
		      player->getStacklist()->setActivestack(0);
		      unselect_active_stack();
		    }
		}
	    }

	  path_set.emit();

	  draw();
	}
      // Stack hasn't been active yet
      else
	{
	  stack = Stacklist::getObjectAt(tile.x, tile.y);
	  if (stack && stack->isFriend(Playerlist::getActiveplayer()) && 
	      d_cursor == GraphicsCache::TARGET)
	    {
	      Playerlist::getActiveplayer()->getStacklist()->setActivestack(stack);
	      select_active_stack();
	    }
	  else
	    {
	      City* c = Citylist::getInstance()->getObjectAt(tile);
	      if (c != NULL && d_cursor == GraphicsCache::ROOK)
		{
		  if (!c->isBurnt())
		    {
		      set_control_key_down (false);
		      if (d_see_opponents_production == true)
			{
			  city_queried (c, false);
			  set_shift_key_down (false);
			}
		      else
			{
			  if (c->getOwner() == Playerlist::getActiveplayer())
			    {
			      city_queried (c, false);
			      set_shift_key_down (false);
			    }
			}
		    }
		}
	      else if (Ruin *r = Ruinlist::getInstance()->getObjectAt(tile))
		{
		  if ((r->isHidden() == true && 
		       r->getOwner() == Playerlist::getActiveplayer()) ||
		      r->isHidden() == false)
		    ruin_queried (r, false);
		}
	      else if (Temple *t = Templelist::getInstance()->getObjectAt(tile))
		{
		  temple_queried (t, false);
		}
	    }
	}
    }
  else if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::RELEASED)
    {
      if (mouse_state == DRAGGING_STACK)
	{
	  Stack* stack = Playerlist::getActiveplayer()->getActivestack();
	  //march a dragged stack!
	  mouse_state = NONE;
	  d_cursor = GraphicsCache::FEET;
	  cursor_changed.emit(d_cursor);
	  //watch out here.  
	  //we recurse for least amount of code and most programmer confusion.
	  e.state = MouseButtonEvent::PRESSED;
	  //go get the final spot in the path
	  if (stack->getPath()->empty() == false)
	    {
	      //check if we dropped on the same tile that the stack lives on.
	      if (mouse_pos_to_tile(e.pos) != stack->getPos())
		{
		  int ts = GameMap::getInstance()->getTileset()->getTileSize();
		  e.pos = tile_to_buffer_pos (*stack->getPath()->back());
		  e.pos.x -= ts/2;
		  e.pos.y -= ts/2;
		  mouse_button_event(e);
		}
	    }
	}
      else
	mouse_state = NONE;
    }


  // right mousebutton to get information about things on the map and to
  // unselect the active stack
  else if (e.button == MouseButtonEvent::RIGHT_BUTTON)
    {
      if (e.state == MouseButtonEvent::PRESSED)
	{
	  if (active->getFogMap()->isCompletelyObscuredFogTile(tile) == true)
	    return;
	  if (City* c = Citylist::getInstance()->getObjectAt(tile))
	    {
	      city_queried (c, true);
	      mouse_state = SHOWING_CITY;
	    }
	  else if (Ruin* r = Ruinlist::getInstance()->getObjectAt(tile))
	    {
	      if ((r->isHidden() == true && 
		   r->getOwner() == Playerlist::getActiveplayer()) ||
		  r->isHidden() == false)
		{
		  ruin_queried (r, true);
		  mouse_state = SHOWING_RUIN;
		}
	    }
	  else if (Signpost* s = Signpostlist::getInstance()->getObjectAt(tile))
	    {
	      signpost_queried (s);
	      mouse_state = SHOWING_SIGNPOST;
	    }
	  else if (Temple* t = Templelist::getInstance()->getObjectAt(tile))
	    {
	      temple_queried.emit(t, true);
	      mouse_state = SHOWING_TEMPLE;
	    }
	  else if (Stack *st = Stacklist::getObjectAt(tile))
	    {
	      if (d_see_opponents_stacks == true)
		{
		  stack_queried.emit(st);
		  mouse_state = SHOWING_STACK;
		}
	      else if (st->getOwner() == Playerlist::getActiveplayer() && 
		       d_see_opponents_stacks == false)
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

	    case DRAGGING_STACK:
	      //no-op
	      break;

	    case DRAGGING_MAP:
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
  else if (e.button == MouseButtonEvent::WHEEL_UP)
    {
      zoom_in();
    }
  else if (e.button == MouseButtonEvent::WHEEL_DOWN)
    {
      zoom_out();
    }
}

void GameBigMap::zoom_in()
{
  if (input_locked)
    return;
  if ((zoom_step / 100.0) + magnification_factor <= max_magnification_factor / 100.0)
    {
      SDL_Surface *v = SDL_GetVideoSurface();
      int ts = GameMap::getInstance()->getTileset()->getTileSize();
      Rectangle new_view;
      double mag = magnification_factor + (zoom_step / 100.0);
      new_view.w = v->w / (ts * mag) + 1;
      new_view.h = v->h / (ts * mag) + 1;
      if (new_view.w <= GameMap::getWidth() && 
	  new_view.h <= GameMap::getHeight() && 
	  new_view.w >= 0 && new_view.h >= 0)
	zoom_view(zoom_step);
    }
}

void GameBigMap::zoom_out()
{
  if (input_locked)
    return;
  if (magnification_factor - (zoom_step / 100.0) >= min_magnification_factor / 100.0)
    {
      SDL_Surface *v = SDL_GetVideoSurface();
      int ts = GameMap::getInstance()->getTileset()->getTileSize();
      Rectangle new_view;
      double mag = magnification_factor - (zoom_step / 100.0);
      new_view.w = v->w / (ts * mag) + 1;
      new_view.h = v->h / (ts * mag) + 1;
      if (new_view.w <= GameMap::getWidth() && 
	  new_view.h <= GameMap::getHeight() && 
	  new_view.w >= 0 && new_view.h >= 0)
	zoom_view(-(const double)zoom_step);
    }
}

void GameBigMap::determine_mouse_cursor(Stack *stack, Vector<int> tile)
{
  Player *active = Playerlist::getActiveplayer();
  if (active->getFogMap()->isCompletelyObscuredFogTile(tile))
    {
      d_cursor = GraphicsCache::HAND;
    }
  else if (stack && mouse_state == DRAGGING_STACK)
    {
      d_cursor = GraphicsCache::GOTO_ARROW;
    }
  else if (stack)
    {
      d_cursor = GraphicsCache::FEET;
      if (stack->getPos() == tile)
	d_cursor = GraphicsCache::TARGET;
      else
	{
	  City *c = Citylist::getInstance()->getObjectAt(tile);
	  if (c)
	    {
	      if (c->getOwner() == active)
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
			  Player *me = stack->getOwner();
			  Player *them = c->getOwner();
			  bool friendly = (me->getDiplomaticState(them) == 
					   Player::AT_PEACE);
			  if (friendly)
			    d_cursor = GraphicsCache::HEART;
			  else
			    d_cursor = GraphicsCache::SWORD;
			}
		    }
		  else
		    {
		      //can i see other ppl's cities?
		      if (d_see_opponents_production == true)
			d_cursor = GraphicsCache::ROOK;
		      else
			d_cursor = GraphicsCache::HAND;
		    }
		}
	    }
	  else
	    {
	      Maptile *t = GameMap::getInstance()->getTile(tile);
	      Stack *st = Stacklist::getObjectAt(tile);
	      if (st && st->getOwner() != active)
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
			  Player *me = stack->getOwner();
			  Player *them = st->getOwner();
			  bool friendly = (me->getDiplomaticState(them) == 
					   Player::AT_PEACE);
			  if (friendly)
			    d_cursor = GraphicsCache::HEART;
			  else
			    d_cursor = GraphicsCache::SWORD;
			}
		    }
		  else
		    d_cursor = GraphicsCache::HAND;
		}
	      else
		{
		  Path path;
		  int moves = path.calculate(stack, tile);
		  if (moves == 0)
		    d_cursor = GraphicsCache::HAND;
		  else
		    {
		      Bridgelist *bl = Bridgelist::getInstance();
		      if (t->getMaptileType() == Tile::WATER &&
			  bl->getObjectAt(tile) == NULL)
			{
			  if (stack->isFlying() == true)
			    d_cursor = GraphicsCache::FEET;
			  else
			    d_cursor = GraphicsCache::SHIP;
			}
		      else
			d_cursor = GraphicsCache::FEET;
		    }
		}
	    }
	}
    }
  else
    {
      d_cursor = GraphicsCache::HAND;
      Stack *st;

      st = active->getStacklist()->getObjectAt(tile);
      if (st)
	{
	  if (st->getOwner() == active)
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
		d_cursor = GraphicsCache::HAND;
	      else if (c->getOwner() == active)
		d_cursor = GraphicsCache::ROOK;
	      else if (d_see_opponents_production == true)
		d_cursor = GraphicsCache::ROOK;
	    }
	  else if (t->getBuilding() == Maptile::RUIN)
	    {
	      Ruin *ruin = Ruinlist::getInstance()->getObjectAt(tile);
	      if (ruin->isHidden() == true && ruin->getOwner() == active)
		d_cursor = GraphicsCache::RUIN;
	      else if (ruin->isHidden() == false)
		d_cursor = GraphicsCache::RUIN;
	    }
	  else if (t->getBuilding() == Maptile::TEMPLE)
	    d_cursor = GraphicsCache::RUIN;
	}

    }
  cursor_changed.emit(d_cursor);
  //debugFogTile(tile.x, tile.y);
}

void GameBigMap::mouse_motion_event(MouseMotionEvent e)
{
  static Vector<int> last_tile;
  if (input_locked)
    return;

  Player *active = Playerlist::getActiveplayer();
  Stack* stack = active->getActivestack();
  Vector<int> tile = mouse_pos_to_tile(e.pos);
  current_tile = tile;
  if (tile.x < 0)
    tile.x = 0;
  if (tile.y < 0)
    tile.y = 0;
  if (tile.x >= GameMap::getWidth())
    tile.x = GameMap::getWidth() - 1;
  if (tile.y >= GameMap::getHeight())
    tile.y = GameMap::getHeight() - 1;


  if (e.pressed[MouseMotionEvent::LEFT_BUTTON]
      && (mouse_state == NONE || mouse_state == SHOWING_STACK) && 
      stack && stack->getPos() == tile && active->getFogMap()->isCompletelyObscuredFogTile(tile) == false && d_cursor != GraphicsCache::HAND)
    {
      //initial drag
      mouse_state = DRAGGING_STACK;
    }
  else if (e.pressed[MouseMotionEvent::LEFT_BUTTON]
	   && (mouse_state == NONE || mouse_state == DRAGGING_MAP) && d_cursor == GraphicsCache::HAND)
    {
      Vector<int> delta = -(e.pos - prev_mouse_pos);

      // ignore very small drags to ensure that a shaking mouse does not
      // prevent the user from making right clicks
      if (mouse_state == NONE && length(delta) <= 2)
	return;

      int ts = GameMap::getInstance()->getTileset()->getTileSize();
      SDL_Surface *screen = SDL_GetVideoSurface();
      Vector<int> screen_dim(screen->w, screen->h);
      view_pos = clip(Vector<int>(0, 0),
		      view_pos + delta,
		      GameMap::get_dim() * ts *magnification_factor - screen_dim);

      // calculate new view position in tiles, rounding up
      Vector<int> new_view = (view_pos + Vector<int>(ts * magnification_factor - 1, ts * magnification_factor - 1)) / (ts * magnification_factor);

      bool redraw_buffer = false;

      if (new_view != view.pos)
	{
	  view.x = new_view.x;
	  view.y = new_view.y;
	  view_changed.emit(view);
	  redraw_buffer = true;
	}

      draw(redraw_buffer);
      mouse_state = DRAGGING_MAP;
    }

  // the following block of code shows the correct mouse cursor
  if (tile == last_tile)
    {
      prev_mouse_pos = e.pos;
      return;
    }

  // drag stack with left mouse button
  if (e.pressed[MouseMotionEvent::LEFT_BUTTON]
      && (mouse_state == DRAGGING_STACK) && 
      active->getFogMap()->isCompletelyObscuredFogTile(tile) == false)
    {
      //subsequent dragging
      //alright.  calculate the path, and show it but don't move
      //be careful that we don't drop our path on bad objects
      //also, slide the whole view if we drag out of view
      if (is_inside(view, tile) == false)
	{
	  Vector<int> delta(0,0);
	  if (tile.x >= view.x + view.w)
	    delta.x += 1;
	  if (tile.x < view.x)
	    delta.x -= 1;
	  if (tile.y > view.y + view.h)
	    delta.y += 1;
	  if (tile.y < view.y)
	    delta.y -= 1;
	  Rectangle new_view = view;
	  new_view.pos += delta;
	  set_view (new_view);
	  view_changed.emit(view);
	}
      mouse_state = NONE;
      determine_mouse_cursor(stack, tile);
      mouse_state = DRAGGING_STACK;
      if (d_cursor == GraphicsCache::FEET ||
	  d_cursor == GraphicsCache::SHIP || 
	  d_cursor == GraphicsCache::TARGET)
	{
	  stack->getPath()->calculate(stack, tile);
	  draw();
	}
    }

  determine_mouse_cursor (stack, tile);
  if (control_key_is_down == true)
    set_control_key_down (true);
  if (shift_key_is_down == true)
    set_shift_key_down (true);

  prev_mouse_pos = e.pos;
  last_tile = tile;
}

void GameBigMap::reset_zoom()
{
  magnification_factor = 1.0;
  screen_size_changed();
  draw(true);
  view_changed.emit(view);
}

void GameBigMap::zoom_view(double percent)
{
  magnification_factor += percent / 100.0;
  //call with +2, or -2
  //Rectangle new_view = view;
  //new_view.dim += Vector<int>(tiles, tiles);
  //new_view.pos += Vector<int>(tiles*-1/2, tiles*-1/2);
  //set_view (new_view);
  screen_size_changed();
  draw(true);
  view_changed.emit(view);
}

void GameBigMap::after_draw()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();

  Stack* stack = Playerlist::getActiveplayer()->getActivestack();

  // Draw Path
  if (stack && stack->getPath()->size() && 
      stack->getOwner()->getType() == Player::HUMAN)
    {
      Vector<int> pos;
      SDL_Color c;
      c.r = c.g = c.b = 0;

      // draw all waypoints
      guint32 pathcount = 0;
      bool canMoveThere = true;
      list<Vector<int>*>::iterator end = stack->getPath()->end();
      //if we're dragging, we don't draw the last waypoint circle
      if (stack->getPath()->size() > 0 && mouse_state == DRAGGING_STACK)
	end--;
      for (list<Vector<int>*>::iterator it = stack->getPath()->begin();
	   it != end; it++)
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

	  canMoveThere = (pathcount < stack->getPath()->getMovesExhaustedAtPoint());
	  if (canMoveThere)
	    r1.x = 0;
	  else
	    r1.x = wpsize;

	  SDL_BlitSurface(d_waypoints, &r1, buffer, &r2);
	  pathcount++;

	}

      if (mouse_state == DRAGGING_STACK)
	{
	  list<Vector<int>*>::iterator it = stack->getPath()->end();
	  it--;
	  //this is where the ghosted army unit picture goes.
	  Army *a = *stack->begin();
	  SDL_Surface *tmp = gc->getArmyPic(a);
	  size_t wpsize = tmp->w;
	  pos = tile_to_buffer_pos(**it);
	  SDL_Rect r1, r2;
	  r1.y = 0;
	  r1.w = r1.h = tmp->w; 
	  int offset = (tilesize - tmp->w) / 2;
	  if (offset < 0)
	    offset = 0;
	  r2.x = pos.x + offset;
	  r2.y = pos.y + offset;
	  r2.w = r2.h = wpsize;
	  SDL_BlitSurface(tmp, 0, buffer, &r2);
	}
    }

  if (stack && d_fighting.getPos() == Vector<int>(-1,-1))
    {
      // draw the selection
      Vector<int> p = stack->getPos();
      if (is_inside(buffer_view, Vector<int>(p.x, p.y)) &&
	  FogMap::isFogged(p, Playerlist::getActiveplayer()) == false)
	{
	  static int bigframe = -1;
	  static int smallframe = -1;

	  bigframe++;
	  if (bigframe >= (int)gc->getNumberOfLargeSelectorFrames())
	    bigframe = 0;

	  smallframe++;
	  if (smallframe >= (int)gc->getNumberOfSmallSelectorFrames())
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

	  if (input_locked == false)
	    {
	      if (num_selected > 1)
		tmp = gc->getSelectorPic(0, bigframe, stack->getOwner());
	      else
		tmp = gc->getSelectorPic(1, smallframe, stack->getOwner());
	      SDL_BlitSurface(tmp, 0, buffer, &r);
	    }
	}
    }

  if (d_fighting.getPos() != Vector<int>(-1,-1))
    {
      Vector<int> p = tile_to_buffer_pos(d_fighting.getPos());
      SDL_Rect r;
      r.x = p.x;
      r.y = p.y;
      r.w = r.h = tilesize * d_fighting.getSize();
      SDL_Surface *tmp = gc->getExplosionPic();
      SDL_SetAlpha(tmp, 0, 0);
      SDL_PixelFormat* fmt = tmp->format;
      SDL_Surface *explode = SDL_CreateRGBSurface
	(SDL_SWSURFACE, d_fighting.getSize() * tilesize, 
	 d_fighting.getSize() * tilesize,
       fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
      SDL_SoftStretch(tmp, 0, explode, 0);
      SDL_BlitSurface(explode, 0, buffer, &r);
      SDL_FreeSurface(explode);
    }
}

void GameBigMap::set_control_key_down (bool down)
{
  control_key_is_down = down;
  Player *active = Playerlist::getActiveplayer();
  if (active->getFogMap()->isCompletelyObscuredFogTile(current_tile) == true)
    return;
  if (GameMap::getInstance()->getTile(current_tile)->getBuilding() != 
      Maptile::CITY)
      return;
  Stack* active_stack = active->getActivestack();
  if (active_stack)
      return;
  //if the key has been released, just show what we'd normally show.
  if (control_key_is_down == false)
    {
      determine_mouse_cursor(active_stack, current_tile);
      return;
    }
  Stack* stack;
  stack = Playerlist::getActiveplayer()->getStacklist()->getObjectAt(current_tile);
  if (!stack)
    return;

  if (d_cursor == GraphicsCache::TARGET)
    {
      City *city = Citylist::getInstance()->getObjectAt(current_tile);
      if (city->isBurnt() == false)
	{
	  d_cursor = GraphicsCache::ROOK;
	  cursor_changed.emit(d_cursor);
	}
    }
  else if (d_cursor == GraphicsCache::HAND &&
	   d_see_opponents_production == true)
    {
      City *city = Citylist::getInstance()->getObjectAt(current_tile);
      if (city->isBurnt() == false)
	{
	  d_cursor = GraphicsCache::ROOK;
	  cursor_changed.emit(d_cursor);
	}
    }
}

void GameBigMap::set_shift_key_down (bool down)
{
  shift_key_is_down = down;
  Player *active = Playerlist::getActiveplayer();
  if (active->getFogMap()->isCompletelyObscuredFogTile(current_tile) == true)
      return;

  Stack* active_stack = active->getActivestack();

  //if the key has been released, just show what we'd normally show.
  if (shift_key_is_down == false)
    {
      determine_mouse_cursor(active_stack, current_tile);
      return;
    }

  //otherwise the shift key is down and we need to do some more checking
  Maptile::Building b = GameMap::getInstance()->getTile(current_tile)->getBuilding();
  if (b == Maptile::RUIN)
    {
      Ruin *r = Ruinlist::getInstance()->getObjectAt(current_tile);
      if (r)
	{
	  if ((r->isHidden() == true && 
	       r->getOwner() == Playerlist::getActiveplayer()) ||
	      r->isHidden() == false)
	    b = Maptile::RUIN;
	  else
	    b = Maptile::NONE;
	}
    }
  else if (b == Maptile::CITY)
    {
      if (d_cursor == GraphicsCache::TARGET)
	{
	  d_cursor = GraphicsCache::ROOK;
	  cursor_changed.emit(d_cursor);
	}
      else if (d_cursor == GraphicsCache::FEET)
	{
	  d_cursor = GraphicsCache::ROOK;
	  cursor_changed.emit(d_cursor);
	}
      else if (d_cursor == GraphicsCache::HAND &&
	       d_see_opponents_production == true)
	{
	  d_cursor = GraphicsCache::ROOK;
	  cursor_changed.emit(d_cursor);
	}
    }

  if (active_stack)
    {
      if (d_cursor == GraphicsCache::SHIP || d_cursor == GraphicsCache::FEET) 
	{
	  if (b == Maptile::RUIN || b == Maptile::TEMPLE)
	    d_cursor = GraphicsCache::RUIN;
	  else
	    d_cursor = GraphicsCache::HAND;
	  cursor_changed.emit(d_cursor);
	}
    }
  else
    {
      if (d_cursor != GraphicsCache::RUIN)
	{
	  if (b == Maptile::RUIN || b == Maptile::TEMPLE)
	    {
	      d_cursor = GraphicsCache::RUIN;
	      cursor_changed.emit(d_cursor);
	    }
	}
    }

  if (d_military_advisor == true)
    {
      if (active_stack && d_cursor == GraphicsCache::SWORD)
	{
	      d_cursor = GraphicsCache::QUESTION;
	      cursor_changed.emit(d_cursor);
	}
    }
}

