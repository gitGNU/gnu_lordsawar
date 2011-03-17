//  Copyright (C) 2010, 2011 Ben Asselstine
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

#ifndef RECENTLYEDITEDFILELIST_H
#define RECENTLYEDITEDFILELIST_H

#include <gtkmm.h>
#include <string>
#include <list>
#include <sigc++/trackable.h>

#include "xmlhelper.h"

class RecentlyEditedFile;

//! A list of files that we've recently edited.
/** 
 * This is only used for network files at the moment.
 * It is implemented as a singleton.
 *
 */
class RecentlyEditedFileList: public std::list<RecentlyEditedFile*>, public sigc::trackable
{
    public:

	//! The xml tag of this object in a recently edited file file.
	static std::string d_tag; 
	
	static const int TWO_WEEKS_OLD = 1209600; /* seconds */


	// Methods that operate on the class data and do not modify the class.

	//! Save the recently edited file list to the given file.
	bool saveToFile(std::string filename = "") const;
        bool save();

	//! Save the recently edited file list to an opened file.
	bool save(XML_Helper* helper) const;


	// Methods that operate on the class data and modify the class.

	//! Load the recently file list from the given file.
	bool loadFromFile(std::string filename = "");

	//! Add a file entry to the list of recently edited files.
	void addEntry(std::string filename);

	//! Touch the file in the recently edited list.
	void updateEntry(std::string filename);

	//! Remove a file entry from the list, by it's scenario id.
	bool removeEntry(std::string filename);

	//! Removes files from the list that are too old, or just too numerous.
	void pruneFiles();
	
        std::list<RecentlyEditedFile*> getFilesWithExtension(std::string ext) const;

	// Static Methods

        //! return the singleton instance of this class.
        static RecentlyEditedFileList * getInstance();

        //! Loads the singleton instance from an opened file.
        static RecentlyEditedFileList * getInstance(XML_Helper *helper);

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

        //! Rewrite or delete an old recently edited file list file.
        static bool upgradeOldVersionsOfFile();
        static bool upgradeOldVersionsOfFile(std::string filename);

    protected:
        //! Default Constructor.
        RecentlyEditedFileList();

	//! Loading constructor
        RecentlyEditedFileList(XML_Helper *helper);
        
        //! Destructor.
        ~RecentlyEditedFileList();

    private:
        //! Callback for loading recentlyeditedfiles into this list.
	bool load(std::string tag, XML_Helper *helper);

	//! Helper method to sort the list by it's last-edited time.
	static bool orderByTime(RecentlyEditedFile*rhs, RecentlyEditedFile *lhs);

	//! Remove the old files from the list.
	void pruneOldFiles(int stale = TWO_WEEKS_OLD);

	//! Remove extraneous files from the list.
	void pruneTooManyFiles(int too_many = 1);

        bool filename_in_list(std::string filename) const;

	// DATA

        //! A static pointer for the singleton instance.
        static RecentlyEditedFileList* s_instance;
};

#endif // RECENTLYEDITEDFILELIST_H

