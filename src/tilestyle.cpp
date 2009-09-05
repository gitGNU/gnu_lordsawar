// Copyright (C) 2007, 2008 Ben Asselstine
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

#include <iostream>
#include <iomanip>


#include "tilestyle.h"
#include "ucompose.hpp"
#include "defs.h"

std::string TileStyle::d_tag = "tilestyle";
using namespace std;

TileStyle::TileStyle()
{
  d_pixmap = NULL;
}

TileStyle::TileStyle(XML_Helper* helper)
{
  int i;
  d_pixmap = NULL;
  char *end = NULL;
  std::string idstr;

  helper->getData(idstr, "id");
  unsigned long int val = 0;
  val = strtoul (idstr.c_str(), &end, 0);
  d_id = (Uint32) val;
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
  
  idstr = String::ucompose ("0x%1", Glib::ustring::format(std::hex, std::setfill(L'0'), std::setw(2), d_id));
  retval &= helper->saveData("id", idstr);
  retval &= helper->saveData("type", d_type);
  retval &= helper->closeTag();

  return retval;
}

std::string TileStyle::getTypeName()
{
  return getTypeName(d_type);
}

std::string TileStyle::getTypeName(Type type)
{
  switch (type)
    {
    case LONE:
      return _("Lone");
      break;
    case OUTERTOPLEFT:
      return _("Outer Top-Left");
      break;
    case OUTERTOPCENTER:
      return _("Outer Top-Centre");
      break;
    case OUTERTOPRIGHT:
      return _("Outer Top-Right");
      break;
    case OUTERBOTTOMLEFT:
      return _("Outer Bottom-Left");
      break;
    case OUTERBOTTOMCENTER:
      return _("Outer Bottom-Centre");
      break;
    case OUTERBOTTOMRIGHT:
      return _("Outer Bottom-Right");
      break;
    case OUTERMIDDLELEFT:
      return _("Outer Middle-Left");
      break;
    case INNERMIDDLECENTER:
      return _("Outer Middle-Centre");
      break;
    case OUTERMIDDLERIGHT:
      return _("Outer Middle-Right");
      break;
    case INNERTOPLEFT:
      return _("Inner Top-Left");
      break;
    case INNERTOPRIGHT:
      return _("Inner Top-Right");
      break;
    case INNERBOTTOMLEFT:
      return _("Inner Bottom-Left");
      break;
    case INNERBOTTOMRIGHT:
      return _("Inner Bottom-Right");
      break;
    case TOPLEFTTOBOTTOMRIGHTDIAGONAL:
      return _("Top-Left To Bottom-Right Diagonal");
      break;
    case BOTTOMLEFTTOTOPRIGHTDIAGONAL:
      return _("Bottom-Left to Top-Right Diagonal");
      break;
    case OTHER:
      return _("Other");
      break;
    default:
      return "unknown";
    }
}
	
TileStyle::Type TileStyle::typeNameToType(std::string name)
{
  if (name == _("Lone"))
    return LONE;
  else if (name == _("Outer Top-Left"))
    return OUTERTOPLEFT;
  else if (name == _("Outer Top-Centre"))
    return OUTERTOPCENTER;
  else if (name == _("Outer Top-Right"))
    return OUTERTOPRIGHT;
  else if (name == _("Outer Bottom-Left"))
    return OUTERBOTTOMLEFT;
  else if (name == _("Outer Bottom-Centre"))
    return OUTERBOTTOMCENTER;
  else if (name == _("Outer Bottom-Right"))
    return OUTERBOTTOMRIGHT;
  else if (name == _("Outer Middle-Left"))
    return OUTERMIDDLELEFT;
  else if (name == _("Outer Middle-Centre"))
    return INNERMIDDLECENTER;
  else if (name == _("Outer Middle-Right"))
    return OUTERMIDDLERIGHT;
  else if (name == _("Inner Top-Left"))
    return INNERTOPLEFT;
  else if (name == _("Inner Top-Right"))
    return INNERTOPRIGHT;
  else if (name == _("Inner Bottom-Left"))
    return INNERBOTTOMLEFT;
  else if (name == _("Inner Bottom-Right"))
    return INNERBOTTOMRIGHT;
  else if (name == _("Top-Left To Bottom-Right Diagonal"))
    return TOPLEFTTOBOTTOMRIGHTDIAGONAL;
  else if (name == _("Bottom-Left to Top-Right Diagonal"))
    return BOTTOMLEFTTOTOPRIGHTDIAGONAL;
  else if (name == _("Other"))
    return OTHER;
  else
    return OTHER;
}

// End of file
