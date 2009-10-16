// Copyright (C) 2008 Ben Asselstine
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

#include <SDL_image.h>
#include <sigc++/functors/mem_fun.h>

#include "cityset.h"

#include "File.h"
#include "xmlhelper.h"

std::string Cityset::d_tag = "cityset";

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Cityset::Cityset(XML_Helper *helper, bool p)
{
    private_collection =  p;
    helper->getData(d_id, "id"); 
    helper->getData(d_name, "name"); 
    helper->getData(d_info, "info");
    helper->getData(d_tileSize, "tilesize");
    helper->getData(d_cities_filename, "cities");
    helper->getData(d_razedcities_filename, "razed_cities");
    helper->getData(d_port_filename, "port");
    helper->getData(d_signpost_filename, "signpost");
    helper->getData(d_ruins_filename, "ruins");
    helper->getData(d_temples_filename, "temples");
    helper->getData(d_towers_filename, "towers");
    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
      citypics[i] = NULL;
    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
      razedcitypics[i] = NULL;
    for (unsigned int i = 0; i < RUIN_TYPES; i++)
      ruinpics[i] = NULL;
    for (unsigned int i = 0; i < TEMPLE_TYPES; i++)
      templepics[i] = NULL;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      towerpics[i] = NULL;
    port = NULL;
    signpost = NULL;
}

Cityset::~Cityset()
{
}

class CitysetLoader
{
public:
    CitysetLoader(std::string name, bool p) 
      {
	cityset = NULL;
	private_collection = p;
	std::string filename = "";
	if (private_collection == false)
	  filename = File::getCityset(name);
	else
	  filename = File::getUserCityset(name);
	XML_Helper helper(filename, ios::in, false);
	helper.registerTag(Cityset::d_tag, sigc::mem_fun((*this), &CitysetLoader::load));
	if (!helper.parse())
	  {
	    std::cerr << "Error, while loading an cityset. Cityset Name: ";
	    std::cerr <<name <<std::endl <<std::flush;
	  }
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Cityset::d_tag)
	  {
	    cityset = new Cityset(helper, private_collection);
	    return true;
	  }
	return false;
      };
    bool private_collection;
    Cityset *cityset;
};
Cityset *Cityset::create(std::string file, bool private_collection)
{
  CitysetLoader d(file, private_collection);
  return d.cityset;
}
// End of file
