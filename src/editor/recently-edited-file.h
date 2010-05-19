//  Copyright (C) 2010 Ben Asselstine
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

#ifndef RECENTLY_EDITED_FILE_H
#define RECENTLY_EDITED_FILE_H

#include <gtkmm.h>
#include <string>


#include <sys/time.h>

class XML_Helper;
class Shieldset;
class Tileset;
class Armyset;
class Cityset;
class GameScenario;

//! A single file entry in the recently edited files list.
/**
 *
 */
class RecentlyEditedFile
{
    public:

	//! The xml tag of this object in a recently edited file.
	static std::string d_tag; 

	//! Loading constructor.
        /**
	 * Make a new recently edited file object by reading it in from an 
	 * opened recently edited files list file.
	 *
         * @param helper  The opened recently edited files list file to read the
	 *                file entry from.
         */
        RecentlyEditedFile(XML_Helper* helper);

	//! Default constructor.
        RecentlyEditedFile(std::string filename);
        
	//! Destructor.
        virtual ~RecentlyEditedFile();

	// Get Methods

	//! Get time of when this file was last edited (seconds past the epoch).
	time_t getTimeOfLastEdit() const { return d_time;};

	//! Get the name of the file.
	std::string getFileName() const {return d_filename;};

	// Set Methods

	//! Set the last time we saw something happen in this file.
	void setTimeOfLastEdit(time_t then) { d_time = then;};

	// Methods that operate on the class data but do not modify it.

	//! Save the file entry to an opened file.
	bool save(XML_Helper* helper) const;

	//! Save the file entry, but not the enclosing tags.
	bool saveContents(XML_Helper *helper) const;


	// Static Methods

	/**
	 * static load function (see XML_Helper)
	 * 
	 * Whenever a file entry is loaded, this function is called. It
	 * examines the file type and calls the constructor of the appropriate
	 * recently edited file class.
	 *
	 * @param helper       the XML_Helper instance for the savefile
	 */
	static RecentlyEditedFile* handle_load(XML_Helper *helper);

    protected:

	//! Save the entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const = 0;

	// DATA
	
	//! When the file was last saved.
	time_t d_time;

	//! The name of the file.
	std::string d_filename;

};

class RecentlyEditedShieldsetFile : public RecentlyEditedFile
{
    public:
	//! Make a new shieldset file entry.
	RecentlyEditedShieldsetFile(std::string filename);

	//! Load a new shieldset file from an opened file.
	RecentlyEditedShieldsetFile(XML_Helper *helper);

	//! Destroy a shieldset file entry.
	~RecentlyEditedShieldsetFile();


	// Methods that operate on the class data but do not modify it.
	
	//! Save the shieldset file entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;

        std::string getName() const {return d_name;};

	// Methods that operate on the class data and modify it.

	//! Assign the shieldset info to the entry.
	bool fillData(Shieldset *shieldset);

    private:
	std::string d_name;
};

class RecentlyEditedTilesetFile : public RecentlyEditedFile
{
    public:
	//! Make a new tileset file entry.
	RecentlyEditedTilesetFile(std::string filename);

	//! Load a new tileset file from an opened saved-file file.
	RecentlyEditedTilesetFile(XML_Helper *helper);

	//! Destroy a tileset file entry.
	~RecentlyEditedTilesetFile();


	// Methods that operate on the class data but do not modify it.
	
	//! Save the tileset file entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;

        std::string getName() const {return d_name;};

	// Methods that operate on the class data and modify it.

	//! Assign the tileset info to the entry.
	bool fillData(Tileset *tileset);
    private:

	// DATA
	
	//! The name of the tileset for this entry.
        std::string d_name;
};

class RecentlyEditedArmysetFile : public RecentlyEditedFile
{
    public:
	//! Make a new armyset file entry.
	RecentlyEditedArmysetFile(std::string filename);

	//! Load a new armyset file from an opened file.
	RecentlyEditedArmysetFile(XML_Helper *helper);

	//! Destroy an armyset file entry.
	~RecentlyEditedArmysetFile();


	// Methods that operate on the class data but do not modify it.

	//! Save the armyset file entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;

        std::string getName() const {return d_name;};

	// Methods that operate on the class data and modify it.

	bool fillData(Armyset *armyset);

    private:

	// DATA
	
	//! The name of the armyset.
	std::string d_name;
};

class RecentlyEditedCitysetFile : public RecentlyEditedFile
{
    public:
	//! Make a new cityset file entry.
	RecentlyEditedCitysetFile(std::string filename);

	//! Load a new cityset file from an opened saved-file file.
	RecentlyEditedCitysetFile(XML_Helper *helper);

	//! Destroy a cityset file entry.
	~RecentlyEditedCitysetFile();


	// Methods that operate on the class data but do not modify it.
	
	//! Save the cityset file entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;

        std::string getName() const {return d_name;};

	// Methods that operate on the class data and modify it.

	//! Assign the cityset info to the entry.
	bool fillData(Cityset *cityset);
    private:

	// DATA
	
	//! The name of the cityset.
	std::string d_name;
};

class RecentlyEditedMapFile : public RecentlyEditedFile
{
    public:
	//! Make a new map file entry.
	RecentlyEditedMapFile(std::string filename);

	//! Load a new map file from an opened saved-file file.
	RecentlyEditedMapFile(XML_Helper *helper);

	//! Destroy a map file entry.
	~RecentlyEditedMapFile();


	// Methods that operate on the class data but do not modify it.
	
	//! Save the cityset file entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;

        std::string getName() const {return d_name;};

	// Methods that operate on the class data and modify it.

	//! Assign the map info to the entry.
	bool fillData(std::string name, guint32 players, guint32 cities);
    private:

	// DATA
	
	//! The name of the map.
	std::string d_name;

        guint32 d_num_players;

        guint32 d_num_cities;
};
#endif // RECENTLY_EDITED_FILE_H
