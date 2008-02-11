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

#ifndef CITYSETLIST_H
#define CITYSETLIST_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "cityset.h"


/** List of all available citysets
  * 
  * This class contains a list of all citysets used in the game. 
  * Since several classes access this class, it is implemented as a singleton.
  */

class Citysetlist : public std::list<CitySet*>, public sigc::trackable
{
    public:
        //! return the singleton instance of this class
        static Citysetlist* getInstance();

        //! Explicitly delete the singleton instance of this class
        static void deleteInstance();

        //! Returns the names of all citysets
	std::list<std::string> getNames();

        /** Returns the subdir of a specific cityset by name
          * 
          * @param cityset       the name of the cityset
          * @return the name of the directory that holds the cityset
          */
	std::string getCitysetDir(std::string name) {return d_dirs[name];}

	CitySet *getCityset(std::string dir) { return d_citysets[dir];}

    private:
        //! Constructor; loads all citysets it can find
        Citysetlist();
        
        //! Destructor; mainly clears the lists
        ~Citysetlist();

        //! Callback for loading. See XML_Helper for details.
	bool load(std::string tag, XML_Helper *helper);

        //! Loads a specific armyset
        bool loadCityset (std::string name);
        
        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, CitySet*> CitySetMap;

        DirMap d_dirs;
        CitySetMap d_citysets;

        static Citysetlist* s_instance;
};

#endif // CITYSETLIST_H

