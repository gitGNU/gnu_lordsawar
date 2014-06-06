// Copyright (C) 2010, 2011, 2014 Ben Asselstine
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

#include <libtar.h>
#include <glibmm.h>
#include <iostream>
#include <list>
class Tar_Helper
{
public:

    //! Constructor
    Tar_Helper(Glib::ustring file, std::ios::openmode mode, bool &broken);

    //! Destructor
    ~Tar_Helper();

    bool saveFile(Glib::ustring file, Glib::ustring destfile = "");

    Glib::ustring getFile(Glib::ustring filename, bool &broken);

    //Glib::ustring getFirstFile(bool &broken);
    Glib::ustring getFirstFile(Glib::ustring extension, bool &broken);
    Glib::ustring getFirstFile(std::list<Glib::ustring> exts, bool &broken);

    std::list<Glib::ustring> getFilenamesWithExtension(Glib::ustring ext);
    Glib::ustring getFirstFilenameWithExtension(Glib::ustring ext);

    std::list<Glib::ustring> getFilenames();


    bool removeFile(Glib::ustring filename);
    //! Replaces old_filename with new_filename, or adds it if not present.
    /**
     * delete old_filename from the archive if present.
     * add new_filename to the archive.
     * @return returns True if successful.
     */
    bool replaceFile(Glib::ustring old_filename, Glib::ustring new_filename);

    bool Open(Glib::ustring file, std::ios::openmode mode);
    void Close();

    static bool is_tarfile (Glib::ustring file);

    static Glib::ustring getFile(TAR *t, Glib::ustring filename, bool &broken, Glib::ustring tmpoutdir);
    static std::list<Glib::ustring> getFilenames(TAR *t);
    static bool saveFile(TAR *t, Glib::ustring filename, Glib::ustring destfile = "");
    //copies a tar file to another place, renaming one of the files inside.
    static bool copy(Glib::ustring filename, Glib::ustring newfilename);
    static void clean_tmp_dir(Glib::ustring filename);
private:

    // DATA
    TAR *t;
    std::ios::openmode openmode;
    Glib::ustring tmpoutdir;
};
