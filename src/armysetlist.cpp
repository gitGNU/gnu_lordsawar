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
#include <expat.h>
#include <SDL_image.h>
#include <SDL.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "armysetlist.h"
#include "File.h"
#include "defs.h"



using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Armysetlist* Armysetlist::s_instance = 0;

Armysetlist* Armysetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Armysetlist();

    return s_instance;
}

void Armysetlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Armysetlist::Armysetlist()
    :d_standard(1), d_heroes(2), d_loading(0)
{
    // load all armysets
    std::list<std::string> armysets = File::scanArmysets();

    std::list<std::string>::const_iterator it;
    for (it = armysets.begin(); it != armysets.end(); it++)
        loadArmyset(*it);
}

Armysetlist::~Armysetlist()
{
    // remove all army entries
    ArmyMap::iterator it;
    for (it = d_armies.begin(); it != d_armies.end(); it++)
        while (!(*it).second.empty())
            delete ((*it).second)[0];
}

const Army* Armysetlist::getArmy(Uint32 id, Uint32 index) const
{
    // always use ArmyMap::find for searching, else a default entry is created,
    // which can produce really bad results!!
    ArmyMap::const_iterator it = d_armies.find(id);

    // armyset does not exist
    if (it == d_armies.end())
        return 0;

    // index too large
    if (index >= (*it).second.size())
        return 0;

    return ((*it).second)[index];
}

Uint32 Armysetlist::getSize(Uint32 id) const
{
    ArmyMap::const_iterator it = d_armies.find(id);

    // armyset does not exist
    if (it == d_armies.end())
        return 0;

    return (*it).second.size();
}

std::string Armysetlist::getName(Uint32 id) const
{
    NameMap::const_iterator it = d_names.find(id);

    // armyset does not exist
    if (it == d_names.end())
        return 0;

    return (*it).second;
}

std::vector<Uint32> Armysetlist::getArmysets(bool force_all) const
{
    std::vector<Uint32> retlist;
    
    NameMap::const_iterator it;
    for (it = d_names.begin(); it != d_names.end(); it++)
    {
        if (!force_all && ((*it).first == d_standard || (*it).first == d_heroes))
            continue;
        retlist.push_back((*it).first);
    }

    return retlist;
}

bool Armysetlist::loadArmyset(std::string name)
{
    debug("Loading armyset " <<name)

    d_file = name;
    
    // first, load the description file; loadGlobalStuff will care for setting
    // up d_loading (the id of the currently loaded armyset) and an entry in
    // the lists.
    XML_Helper helper(File::getArmyset(name), ios::in, false);
    helper.registerTag("armyset", sigc::mem_fun((*this), &Armysetlist::loadGlobalStuff));
    helper.registerTag("army", sigc::mem_fun((*this), &Armysetlist::loadArmy));
    
    if (!helper.parse())
    {
        std::cerr <<_("Error, while loading an armyset. Armyset Name: ");
        std::cerr <<name <<std::endl <<std::flush;
        exit(-1);
    }

    return true;
}

bool Armysetlist::loadGlobalStuff(std::string tag, XML_Helper* helper)
{
    bool retval = true;
    std::string name;
    
    // load the data
    retval &= helper->getData(d_loading, "id");
    retval &= helper->getData(name, "name");

    // create the neccessary entry in the name list
    d_names[d_loading] = name;

    file_names[d_file] = d_loading;
    
    return retval;
}

bool Armysetlist::loadArmy(string tag, XML_Helper* helper)
{
    std::string s;
    static int armysize = 54;   // army pic has this size
    
    // First step: Load the army data
    Army* a = new Army(helper, true);
    a->setArmyset(d_loading, d_armies[d_loading].size());

    d_armies[d_loading].push_back(a);


    // Second step: load the army picture. This is done here to avoid confusion
    // since the armies are used as prototypes as well as actual units in the
    // game.
    // The army image consists of two halves. On the left is the army image, on the
    // right the mask.
    helper->getData(s, "image");
    SDL_Surface* pic = File::getArmyPicture(d_file, s + ".png");
    if (!pic)
    {
        std::cerr <<"Could not load army image: " << s <<std::endl;
	// FIXME: more gentle way of reporting error than just exiting?
        exit(-1);
    }

    // don't use alpha information, just copy the channel! very important
    SDL_SetAlpha(pic, 0, 0);
    SDL_PixelFormat* fmt = pic->format;

    // mask out the army image 
    SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, armysize, armysize,
                fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_Rect r;
    r.x = r.y = 0;
    r.w = r.h = armysize;
    SDL_BlitSurface(pic, &r, tmp, 0);

    SDL_Surface* pixmap = SDL_DisplayFormatAlpha(tmp);
    a->setPixmap(pixmap);

    SDL_FreeSurface(tmp);

    // now extract the mask; it should have a certain data format since the player
    // colors are applied by modifying the RGB shifts
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, armysize, armysize, 32,
                               0xFF000000, 0xFF0000, 0xFF00, 0xFF);

    r.x = armysize;
    SDL_BlitSurface(pic, &r, tmp, 0);
    a->setMask(tmp);

    SDL_FreeSurface(pic);

    // and along the way, try to load a portrait as well
    helper->getData(s, "portrait");
    if (!s.empty())
        a->setPortrait(File::getArmyPicture(d_file, s + ".jpg"));
     
    return true;
}

