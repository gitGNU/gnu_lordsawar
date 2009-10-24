//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include <iostream>
#include <sstream>
#include "shieldstyle.h"
#include "GraphicsCache.h"
#include "xmlhelper.h"
#include "File.h"
#include "shieldset.h"
#include "gui/image-helpers.h"

std::string ShieldStyle::d_tag = "shieldstyle";

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

ShieldStyle::ShieldStyle(XML_Helper* helper)
  :d_image(0), d_mask(0)
{
  std::string type_str;
  helper->getData(type_str, "type");
  d_type = shieldStyleTypeFromString(type_str);
  helper->getData(d_image_name, "image");
}

ShieldStyle::~ShieldStyle()
{
}

        
std::string ShieldStyle::shieldStyleTypeToString(const ShieldStyle::Type type)
{
  switch (type)
    {
      case ShieldStyle::SMALL:
	return "ShieldStyle::SMALL";
	break;
      case ShieldStyle::MEDIUM:
	return "ShieldStyle::MEDIUM";
	break;
      case ShieldStyle::LARGE:
	return "ShieldStyle::LARGE";
	break;
    }
  return "ShieldStyle::SMALL";
}

ShieldStyle::Type ShieldStyle::shieldStyleTypeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return ShieldStyle::Type(atoi(str.c_str()));
  if (str == "ShieldStyle::SMALL")
    return ShieldStyle::SMALL;
  else if (str == "ShieldStyle::MEDIUM")
    return ShieldStyle::MEDIUM;
  else if (str == "ShieldStyle::LARGE")
    return ShieldStyle::LARGE;
  return ShieldStyle::SMALL;
}

bool ShieldStyle::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  std::string s = shieldStyleTypeToString(ShieldStyle::Type(d_type));
  retval &= helper->saveData("type", s);
  retval &= helper->saveData("image", d_image_name);
  retval &= helper->closeTag();
  return retval;
}
void ShieldStyle::instantiateImages(std::string filename, Shieldset *s)
{
  if (filename.empty() == true)
    return;
  // The shield image consists of two halves. On the left is the shield 
  // image, on the right the mask.
  std::vector<PixMask* > half = disassemble_row(filename, 2);

  int xsize = 0;
  int ysize = 0;
  switch (getType())
    {
    case ShieldStyle::SMALL:
      xsize = s->getSmallWidth(); ysize = s->getSmallHeight(); break;
    case ShieldStyle::MEDIUM:
      xsize = s->getMediumWidth(); ysize = s->getMediumHeight(); break;
    case ShieldStyle::LARGE:
      xsize = s->getLargeWidth(); ysize = s->getLargeHeight(); break;
    }
  PixMask::scale(half[0], xsize, ysize);
  PixMask::scale(half[1], xsize, ysize);
  setImage(half[0]);
  setMask(half[1]);
}

void ShieldStyle::uninstantiateImages()
{
  if (getImage())
    {
      delete getImage();
      setImage(NULL);
    }
  if (getMask())
    {
      delete getMask();
      setMask(NULL);
    }
}
