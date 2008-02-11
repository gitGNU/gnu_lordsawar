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

#include <iostream>
#include <sstream>
#include "shield.h"
#include "shieldsetlist.h"
#include "counter.h"
#include "GraphicsCache.h"
#include "xmlhelper.h"
#include "stacklist.h"
#include "templelist.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Shield::Shield(XML_Helper* helper)
  :d_pixmap(0), d_mask(0), d_shieldset(0)
{
  helper->getData(d_type, "type");
  helper->getData(d_colour, "colour");
  helper->getData(d_image, "image");
}

Shield::~Shield()
{
    if (d_pixmap)
        SDL_FreeSurface(d_pixmap);
    if (d_mask)
        SDL_FreeSurface(d_mask);
}

SDL_Surface* Shield::getPixmap() const
{
    // if we already have a pixmap return it
    if (d_pixmap)
        return d_pixmap;
    
    //use the GraphicsCache to get a picture of the shield's shieldset_shield
    return GraphicsCache::getInstance()->getShieldPic(d_shieldset, d_type,
                                         d_colour);
}

bool Shield::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("shield");
    retval &= helper->closeTag();

    return retval;
}

bool Shield::saveData(XML_Helper* helper) const
{
    bool retval = true;
    
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("colour", d_colour);
    retval &= helper->saveData("image", d_image);
    return retval;
}

void Shield::setPixmap(SDL_Surface* pixmap)
{
  if (d_pixmap)
    SDL_FreeSurface(d_pixmap);
  d_pixmap = pixmap;
}
        
void Shield::setMask(SDL_Surface* mask)
{
  if (d_mask)
    SDL_FreeSurface(d_mask);
  d_mask = mask;
}
