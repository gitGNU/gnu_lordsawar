//  Copyright (C) 2011 Ben Asselstine
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

#ifndef FILE_COMPAT_H
#define FILE_COMPAT_H

#include <gtkmm.h>
#include <string>
#include <list>
#include <sigc++/trackable.h>

#include "xmlhelper.h"

class UpgradeDetails;

class FileDetails
{
public:
  FileDetails(guint32 k, std::string f, std::string t, bool ta) 
    {type = k; file_extension = f; tag = t; tar = ta;};
  guint32 type;
  std::string file_extension;
  std::string tag;
  bool tar;
};

//! An interface to provide backwards compatibility for files.
class FileCompat: public std::list<FileDetails>, public sigc::trackable
{
    public:

        enum Type {
          UNKNOWN = 0,
          CONFIGURATION,
          ITEMLIST,
          PROFILELIST,
          RECENTLYPLAYEDGAMELIST,
          GAMELIST,
          RECENTLYEDITEDFILELIST,
          ARMYSET,
          TILESET,
          CITYSET,
          SHIELDSET,
          GAMESCENARIO,
        };

        //! upgrade common files.
        void initialize();

        typedef sigc::slot<bool, std::string, std::string, std::string> Slot;
        void support_type (guint32 k, std::string f, std::string t, bool ta) 
          {push_back(FileDetails(k,f,t,ta));};
        void support_version(guint32 k, std::string from, std::string to, FileCompat::Slot slot);

        bool contains(FileCompat::Type type) const;
        FileCompat::Type getType(std::string filename) const;
        FileCompat::Type getTypeByFileInspection(std::string filename) const;

        bool isTarFile(FileCompat::Type type) const;

        bool rewrite_with_updated_version(std::string filename, FileCompat::Type type, std::string tag, std::string version);
        std::string getTag(FileCompat::Type type) const;
        FileCompat::Type getTypeByFileExtension(std::string ext) const;
        std::string getFileExtension(FileCompat::Type type) const;
        bool get_tag_and_version_from_file(std::string filename, FileCompat::Type type, std::string &tag, std::string &version) const;

        bool upgrade(std::string filename, bool &same) const;


	// Static Methods
        static void support_backward_compatibility_for_common_files();
        static Glib::ustring typeToString(const FileCompat::Type type);

        //! return the singleton instance of this class.
        static FileCompat * getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

    protected:
        //! Default Constructor.
        FileCompat();

        //! Destructor.
        ~FileCompat();

        // helpers
        FileCompat::Type getTypeByXmlFileInspection(std::string filename) const;
        FileCompat::Type getTypeByTarFileInspection(std::string filename) const;

        bool upgradeGameScenario(std::string filename, std::string old_version) const;

        bool can_upgrade_to(FileCompat::Type type, std::string version) const;
        bool get_upgrade_method(FileCompat::Type type, std::string version, std::string &next_version, FileCompat::Slot &slot) const;

    private:
	// DATA
        std::list<UpgradeDetails> versions[GAMESCENARIO + 1];


        //! A static pointer for the singleton instance.
        static FileCompat* s_instance;
};

#endif // FILE_COMPAT_H

