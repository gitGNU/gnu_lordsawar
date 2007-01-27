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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "File.h"
#include "defs.h"
#include <pgfilearchive.h>
#include <SDL_image.h>
#include "Configuration.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

SDL_Surface* File::loadImage(string filename, bool alpha)
{
    SDL_Surface* tmp = IMG_Load(filename.c_str());
    if (!tmp)
    {
        cerr << _("ERROR: Couldn't load image ") << filename << endl;
        return 0;
    }

    SDL_Surface* convertedImage;
    // 1:1 copy
    if (alpha)
        convertedImage = SDL_DisplayFormatAlpha(tmp);
    // strip alpha channel if any
    else
        convertedImage = SDL_DisplayFormat(tmp);
    SDL_FreeSurface(tmp);

    return convertedImage;

}

std::list<std::string> File::scanArmysets()
{
    std::list<std::string> retlist;
    
    string path = Configuration::s_dataPath + "/army/";
    PG_FileArchive::RemoveAllArchives();
    PG_FileArchive::AddArchive(path.c_str());
    char** filelist = PG_FileArchive::EnumerateFiles("");
    for (int i = 0; filelist[i] != NULL; ++i)
    {
        string entry(filelist[i]);
        string::size_type idx = entry.find(".xml");
        if (idx != string::npos)
        {
            entry.replace(idx, 4, "");  //substitute the ".xml" with ""
            retlist.push_back(entry);
        }
    }

    if (retlist.empty())
    {
        cerr << _("Couldn't find a single armyset!") << endl;
        cerr << _("Please check the path settings in /etc/freelordsrc or ~/.freelordsrc") << endl;
        cerr << _("Exiting!") << endl;
        exit(-1);
    }

    //free the filelist
    for (int i = 0; filelist[i] != NULL; ++i)
    {
        free(filelist[i]);
    }
    free(filelist);

    return retlist;
}


string File::getArmyset(string armysetname)
{
    return Configuration::s_dataPath + "/army/" + armysetname + ".xml";
}

SDL_Surface* File::getArmyPicture(string armysetname, string pic)
{
    return loadImage(Configuration::s_dataPath + "/army/" + armysetname + "/" + pic);
}


string File::getMapset(string mapsetname)
{
    return Configuration::s_dataPath + "/tilesets/" + mapsetname + "/" + mapsetname + ".xml";
}

SDL_Surface* File::getMapsetPicture(string mapsetname, string picname)
{
    return loadImage(Configuration::s_dataPath + "/tilesets/" + mapsetname + "/" + picname);
}

SDL_Surface* File::getMapsetMask(string mapsetname, string picname)
{
    string filename = Configuration::s_dataPath + "/tilesets/" + mapsetname + "/" + picname;
    return IMG_Load(filename.c_str());
}


string File::getMiscFile(string filename)
{
    return Configuration::s_dataPath + "/" + filename;
}

SDL_Surface* File::getMiscPicture(string picname, bool alpha)
{
    return loadImage(Configuration::s_dataPath + "/various/" + picname,alpha);
}


SDL_Surface* File::getItemPicture(string picname)
{
    return loadImage(Configuration::s_dataPath + "/various/items/" + picname + ".png");
}

string File::getItemDescription()
{
    return Configuration::s_dataPath + "/various/items/items.xml";
}


SDL_Surface* File::getBorderPic(std::string filename)
{
    return loadImage(Configuration::s_dataPath + "/various/borders/" + filename + ".png");
}


SDL_Surface* File::getEditorPic(std::string filename)
{
    return loadImage(Configuration::s_dataPath + "/various/editor/" + filename + ".png");
}

std::string File::getMusicFile(std::string filename)
{
    return std::string(Configuration::s_dataPath + "/music/" + filename.c_str());
}

string File::getSavePath()
{
    return Configuration::s_savePath + "/";
}

list<string> File::scanTilesets(PG_DropDown* drop_down)
{
    string path = Configuration::s_dataPath + "/tilesets/";
    std::list<std::string> retlist;
    
    PG_FileArchive::RemoveAllArchives();
    PG_FileArchive::AddArchive(path.c_str());
    char** filelist = PG_FileArchive::EnumerateFiles("");

    for (int i = 0; filelist[i] != NULL; ++i) 
    {
        if (PG_FileArchive::IsDirectory(filelist[i]) )
            PG_FileArchive::AddArchive((path+string(filelist[i])).c_str());
    }
    
    filelist = PG_FileArchive::EnumerateFiles("");

    for (int j = 0; filelist[j] != NULL; ++j)
    {
        string entry(filelist[j]);
        string::size_type idx = entry.find(".xml");
        if (idx != string::npos)
        {
            entry.replace(idx, 4, "");  //substitute the ".xml" with ""
            retlist.push_back(entry);
            drop_down->AddItem(entry.c_str());
            cerr << "Found  TileSet \"" << entry << "\"." << endl;
        }
    }  
    
    if (retlist.empty())
    {
        cerr << _("Couldn't find a single tileset!") << endl;
        cerr << _("Please check the path settings in /etc/freelordsrc or ~/.freelordsrc") << endl;
        cerr << _("Exiting!") << endl;
        exit(-1);
    }

    drop_down->SetText(retlist.front().c_str());
    PG_FileArchive::RemoveAllArchives();
    
    return retlist;
}

list<string> File::scanMaps()
{
    string path = Configuration::s_dataPath + "/map/";
    std::list<std::string> retlist;
    
    PG_FileArchive::RemoveAllArchives();
    PG_FileArchive::AddArchive(path.c_str());
    char** filelist = PG_FileArchive::EnumerateFiles("");

    for (int i = 0; filelist[i] != NULL; ++i) 
    {
        if (PG_FileArchive::IsDirectory(filelist[i]) )
            PG_FileArchive::AddArchive((path+string(filelist[i])).c_str());
    }
    
    filelist = PG_FileArchive::EnumerateFiles("");

    for (int j = 0; filelist[j] != NULL; ++j)
    {
        string entry(filelist[j]);
        string::size_type idx = entry.find(".map");
        if (idx != string::npos)
        {
            entry.replace(idx, 4, "");  //substitute the ".xml" with ""
            retlist.push_back(entry);
            cerr << "Found Map \"" << entry << "\"." << endl;
        }
    }  
    
    if (retlist.empty())
    {
        cerr << _("Couldn't find a single map!") << endl;
        cerr << _("Please check the path settings in /etc/freelordsrc or ~/.freelordsrc") << endl;
    }

    PG_FileArchive::RemoveAllArchives();
    
    return retlist;
}


// End of file
