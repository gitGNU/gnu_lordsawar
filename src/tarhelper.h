// Copyright (C) 2010, 2011 Ben Asselstine
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
#include <string>
#include <iostream>
#include <list>
class Tar_Helper
{
public:

    //! Constructor
    Tar_Helper(std::string file, std::ios::openmode mode, bool &broken);

    //! Destructor
    ~Tar_Helper();

    bool saveFile(std::string file, std::string destfile = "");

    std::string getFile(std::string filename, bool &broken);

    //std::string getFirstFile(bool &broken);
    std::string getFirstFile(std::string extension, bool &broken);
    std::string getFirstFile(std::list<std::string> exts, bool &broken);

    std::list<std::string> getFilenamesWithExtension(std::string ext);

    std::list<std::string> getFilenames();


    bool removeFile(std::string filename);
    bool replaceFile(std::string old_filename, std::string new_filename);

    bool Open(std::string file, std::ios::openmode mode);
    void Close();

    static bool is_tarfile (std::string file);

    static std::string getFile(TAR *t, std::string filename, bool &broken, std::string tmpoutdir);
    static std::list<std::string> getFilenames(TAR *t);
    static bool saveFile(TAR *t, std::string filename, std::string destfile = "");
    //copies a tar file to another place, renaming one of the files inside.
    static bool copy(std::string filename, std::string newfilename);
    static void clean_tmp_dir(std::string filename);
private:

    // DATA
    TAR *t;
    std::ios::openmode openmode;
    std::string tmpoutdir;
};
