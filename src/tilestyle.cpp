// Copyright (C) 2007, 2008, 2009, 2010, 2014 Ben Asselstine
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
#include <iomanip>


#include "tilestyle.h"
#include "ucompose.hpp"
#include "defs.h"

std::string TileStyle::d_tag = "tilestyle";
using namespace std;

TileStyle::TileStyle()
  : d_image(0)
{
}
        
TileStyle::TileStyle(const TileStyle& t)
{
  if (t.d_image != NULL)
    d_image = t.d_image->copy();
  else
    d_image = NULL;

  d_type = t.d_type;

  d_id = t.d_id;
}

TileStyle::TileStyle(guint32 id, TileStyle::Type type)
        : d_image(0), d_type(type), d_id(id)
{
}

TileStyle::TileStyle(XML_Helper* helper)
  : d_image(0)
{
  int i;
  char *end = NULL;
  std::string idstr;

  helper->getData(idstr, "id");
  unsigned long int val = 0;
  val = strtoul (idstr.c_str(), &end, 0);
  d_id = (guint32) val;
  helper->getData(i, "type");
  d_type = static_cast<TileStyle::Type>(i);

}
    
TileStyle::~TileStyle()
{
}

bool TileStyle::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  Glib::ustring idstr;
  
  idstr = String::ucompose ("0x%1", idToString(d_id));
  retval &= helper->saveData("id", idstr);
  retval &= helper->saveData("type", d_type);
  retval &= helper->closeTag();

  return retval;
}

std::string TileStyle::getTypeName() const
{
  return getTypeName(d_type);
}

std::string TileStyle::getTypeName(Type type)
{
  switch (type)
    {
    case LONE:
      return "Lone";
      break;
    case OUTERTOPLEFT:
      return "Outer Top-Left";
      break;
    case OUTERTOPCENTER:
      return "Outer Top-Centre";
      break;
    case OUTERTOPRIGHT:
      return "Outer Top-Right";
      break;
    case OUTERBOTTOMLEFT:
      return "Outer Bottom-Left";
      break;
    case OUTERBOTTOMCENTER:
      return "Outer Bottom-Centre";
      break;
    case OUTERBOTTOMRIGHT:
      return "Outer Bottom-Right";
      break;
    case OUTERMIDDLELEFT:
      return "Outer Middle-Left";
      break;
    case INNERMIDDLECENTER:
      return "Outer Middle-Centre";
      break;
    case OUTERMIDDLERIGHT:
      return "Outer Middle-Right";
      break;
    case INNERTOPLEFT:
      return "Inner Top-Left";
      break;
    case INNERTOPRIGHT:
      return "Inner Top-Right";
      break;
    case INNERBOTTOMLEFT:
      return "Inner Bottom-Left";
      break;
    case INNERBOTTOMRIGHT:
      return "Inner Bottom-Right";
      break;
    case TOPLEFTTOBOTTOMRIGHTDIAGONAL:
      return "Top-Left To Bottom-Right Diagonal";
      break;
    case BOTTOMLEFTTOTOPRIGHTDIAGONAL:
      return "Bottom-Left to Top-Right Diagonal";
      break;
    case OTHER:
      return "Other";
      break;
    case UNKNOWN:
      return "Unknown";
      break;
    default:
      return "Unknown";
    }
}
	
TileStyle::Type TileStyle::typeNameToType(std::string name)
{
  if (name == "Lone")
    return LONE;
  else if (name == "Outer Top-Left")
    return OUTERTOPLEFT;
  else if (name == "Outer Top-Centre")
    return OUTERTOPCENTER;
  else if (name == "Outer Top-Right")
    return OUTERTOPRIGHT;
  else if (name == "Outer Bottom-Left")
    return OUTERBOTTOMLEFT;
  else if (name == "Outer Bottom-Centre")
    return OUTERBOTTOMCENTER;
  else if (name == "Outer Bottom-Right")
    return OUTERBOTTOMRIGHT;
  else if (name == "Outer Middle-Left")
    return OUTERMIDDLELEFT;
  else if (name == "Outer Middle-Centre")
    return INNERMIDDLECENTER;
  else if (name == "Outer Middle-Right")
    return OUTERMIDDLERIGHT;
  else if (name == "Inner Top-Left")
    return INNERTOPLEFT;
  else if (name == "Inner Top-Right")
    return INNERTOPRIGHT;
  else if (name == "Inner Bottom-Left")
    return INNERBOTTOMLEFT;
  else if (name == "Inner Bottom-Right")
    return INNERBOTTOMRIGHT;
  else if (name == "Top-Left To Bottom-Right Diagonal")
    return TOPLEFTTOBOTTOMRIGHTDIAGONAL;
  else if (name == "Bottom-Left to Top-Right Diagonal")
    return BOTTOMLEFTTOTOPRIGHTDIAGONAL;
  else if (name == "Other")
    return OTHER;
  else if (name == "Unknown")
    return UNKNOWN;
  else
    return UNKNOWN;
}

guint32 TileStyle::calculateHexDigits(guint32 id)
{
  if (id < 256)
    return 2;
  else if (id < 4096)
    return 3;
  else if (id < 65536)
    return 4;
  else
    return 5;
}

std::string TileStyle::idToString(guint32 id, guint32 digits)
{
  guint32 num_digits;
  if (digits != 0)
    num_digits = digits;
  else
    num_digits = calculateHexDigits(id);

  return String::ucompose ("%1", Glib::ustring::format(std::hex, std::setfill(L'0'), std::setw(num_digits), id));
}
// End of file
