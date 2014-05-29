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

#include "tarhelper.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include "File.h"
#include <errno.h>
#include "ucompose.hpp"

Tar_Helper::Tar_Helper(Glib::ustring file, std::ios::openmode mode, bool &broken)
{
  t = NULL;
  broken = Open(file, mode);
}

bool Tar_Helper::Open(Glib::ustring file, std::ios::openmode mode)
{
  t = NULL;
  bool broken = false;
  if (mode == std::ios::in && is_tarfile (file) == false)
    {
      broken = true;
      return broken;
    }
  int m;
  int perms = 0;
  openmode = mode;
  if (mode & std::ios::in) 
    m = O_RDONLY;
  else if (mode & std::ios::out)
    {
      m = O_WRONLY|O_CREAT;
      perms = 0644;
    }
  else
    return broken;

  char *f = strdup(file.c_str());
  int retval = tar_open (&t, f, NULL, m, perms, TAR_GNU);
  if (retval < 0)
    {
      t = NULL;
      broken = true;
      return broken;
    }
      
  if (mode & std::ios::in)
    {
      std::list<Glib::ustring> files = getFilenames();
      tmpoutdir = String::ucompose("%1/%2.%3/", Glib::get_tmp_dir(), File::get_basename(f,true), getpid());
      File::create_dir(tmpoutdir);
    }
  else
    tmpoutdir = "";
  return broken;
}

bool Tar_Helper::saveFile(TAR *t, Glib::ustring filename, Glib::ustring destfile)
{
  char *f = strdup(filename.c_str());
  char *b;
  if (destfile == "")
    b = strdup(File::get_basename(filename,true).c_str());
  else
    b = strdup(destfile.c_str());
  int retval = tar_append_file(t, f, b);
  free (f);
  free (b);
  if (retval != 0)
    return false;
  return true;
}

bool Tar_Helper::saveFile(Glib::ustring filename, Glib::ustring destfile)
{
  return saveFile(t, filename, destfile);
}

void Tar_Helper::Close()
{
  if (t)
    {
      free (t->pathname);
      if (openmode & std::ios::out)
	tar_append_eof(t);
    
      tar_close(t);
      t = NULL;
      if (tmpoutdir != "")
	File::erase_dir(tmpoutdir);
    }
}
    
Glib::ustring Tar_Helper::getFirstFile(std::list<Glib::ustring> exts, bool &broken)
{
  for (std::list<Glib::ustring>::iterator i = exts.begin(); i != exts.end(); i++)
    {
      Glib::ustring file = getFirstFile(*i, broken);
      if (file != "")
        return file;
    }
  return "";
}

Glib::ustring Tar_Helper::getFirstFile(Glib::ustring extension, bool &broken)
{
  std::list<Glib::ustring> files = getFilenamesWithExtension(extension);
  if (files.size() == 0)
    return "";
  return getFile(files.front(), broken);
}

Glib::ustring Tar_Helper::getFile(TAR *t, Glib::ustring filename, bool &broken, Glib::ustring tmpoutdir)
{
  if (File::exists(tmpoutdir + filename) == true)
    return tmpoutdir + filename;
  char buf[T_BLOCKSIZE];
  lseek(t->fd, 0, SEEK_SET);
  int i, k;
  bool found = false;
  //cycle through looking to find a file by that name.
  while ((i = th_read(t)) == 0)
    {
      char *f = th_get_pathname(t);
      if (strcmp(f, filename.c_str()) == 0)
	  {
	    found = true;
	    break;
	  }
      int size = th_get_size(t);
      for (int i = size; i > 0; i -= T_BLOCKSIZE)
	{
	  k = tar_block_read(t, buf);
	  if (k != T_BLOCKSIZE)
	    {
	      broken = true;
	      return "";
	    }
	}
    }
  if (!found)
    {
      broken = true;
      return "";
    }

  int size = th_get_size(t);
  char *data = (char* )malloc(size);
  size_t bytesread = 0;
  for (int i = size; i > 0; i -= T_BLOCKSIZE)
    {
      k = tar_block_read(t, buf);
      if (k != T_BLOCKSIZE)
	{
	  broken = true;
	  return "";
	}
      memcpy (&data[bytesread], buf, i > T_BLOCKSIZE ? T_BLOCKSIZE : i);
      bytesread +=  (i > T_BLOCKSIZE) ? T_BLOCKSIZE : i;
    }
  Glib::ustring outfile = tmpoutdir + filename;
  FILE *fileptr = fopen(outfile.c_str(), "w");
  if (!fileptr)
    {
      broken = true;
      outfile = "";
    }
  else
    {
      fwrite(data, 1, size, fileptr);
      fclose(fileptr);
      broken = false;
    }

  return outfile;
}

Glib::ustring Tar_Helper::getFile(Glib::ustring filename, bool &broken)
{
  return getFile(t, filename, broken, tmpoutdir);
}

std::list<Glib::ustring> Tar_Helper::getFilenames(TAR *t)
{
  std::list<Glib::ustring> result;
  int i, k;
  char buf[T_BLOCKSIZE];
  lseek(t->fd, 0, SEEK_SET);
  //cycle through looking to find files with that extension.
  while ((i = th_read(t)) == 0)
    {
      char *f = th_get_pathname(t);
      result.push_back(f);
	
      int size = th_get_size(t);
      for (int i = size; i > 0; i -= T_BLOCKSIZE)
	{
	  k = tar_block_read(t, buf);
	  if (k != T_BLOCKSIZE)
	    return result;
	}
    }
  return result;
}

std::list<Glib::ustring> Tar_Helper::getFilenames()
{
  return getFilenames(t);
}

Glib::ustring Tar_Helper::getFirstFilenameWithExtension(Glib::ustring ext)
{
  std::list<Glib::ustring> result = getFilenamesWithExtension(ext);
  if (result.empty())
    return "";
  return result.front();
}

std::list<Glib::ustring> Tar_Helper::getFilenamesWithExtension(Glib::ustring ext)
{
  std::list<Glib::ustring> result;
  int i, k;
  char buf[T_BLOCKSIZE];
  lseek(t->fd, 0, SEEK_SET);
  //cycle through looking to find files with that extension.
  while ((i = th_read(t)) == 0)
    {
      char *f = th_get_pathname(t);
      if (File::nameEndsWith(f, ext) == true)
	result.push_back(f);
	
      int size = th_get_size(t);
      for (int i = size; i > 0; i -= T_BLOCKSIZE)
	{
	  k = tar_block_read(t, buf);
	  if (k != T_BLOCKSIZE)
	    return result;
	}
    }
  return result;
}

Tar_Helper::~Tar_Helper()
{
  if (t)
    Close();
}

bool Tar_Helper::is_tarfile (Glib::ustring file)
{
  char *filename = strdup(file.c_str());

  FILE *f = fopen (filename, "rb");
  free(filename);
  if (f == NULL)
    return false;
  bool retval = true;
  struct tar_header header;
  memset (&header, 0, sizeof (header));
  size_t bytesread = fread (&header, sizeof (header), 1, f);
  if (bytesread != 1)
    retval = false;
  else
    {
      if (strncmp(header.magic, "ustar", 5) != 0)
        retval = false;
    }
  fclose (f);
  return retval;
}

bool Tar_Helper::removeFile(Glib::ustring filename)
{
  return replaceFile(filename, "");
}

bool Tar_Helper::replaceFile(Glib::ustring filename, Glib::ustring newfilename)
{
  bool broken = false;
  //copy all files except this one into a new file
  int m;
  int perms = 0;
  m = O_WRONLY|O_CREAT;
  perms = 0644;
  TAR *new_tar = NULL;
  if (newfilename != "" && File::exists(newfilename) == false)
    return false;
  Glib::ustring newtmpoutdir = String::ucompose("%1/%2.%3.replace/", Glib::get_tmp_dir(), File::get_basename(t->pathname), getpid());
  File::create_dir(newtmpoutdir);
  Glib::ustring new_tar_file = newtmpoutdir +"/" + File::get_basename(t->pathname);
  char *f = strdup (new_tar_file.c_str());
  tar_open (&new_tar, f, NULL, m, perms, TAR_GNU);
  free (f);
  std::list<Glib::ustring> files = getFilenames();
  std::list<Glib::ustring> delfiles;
  for (std::list<Glib::ustring>::iterator it = files.begin(); it != files.end(); 
       ++it)
    {
      if (*it == filename) //here we skip over the one we want to remove
        continue;
      Glib::ustring extracted_file = getFile(t, *it, broken, newtmpoutdir);
      delfiles.push_back(extracted_file);
      if (broken)
        break;
    }
  for (std::list<Glib::ustring>::iterator it = delfiles.begin(); 
       it != delfiles.end(); it++)
    {
      if (!broken)
        saveFile(new_tar, *it);
      File::erase(*it);
    }
  if (broken)
    return false;
  //now add the new file, if we specified one at all.
  if (newfilename != "")
    saveFile(new_tar, newfilename);
  //okay, now we get rid of the old tar file and put the new one in it's place.
  Glib::ustring orig_tar_file = t->pathname;

  tar_close(new_tar);
  Close();

  File::copy(new_tar_file, orig_tar_file);
  File::erase(new_tar_file);
  File::erase_dir(newtmpoutdir);
  broken = Open (orig_tar_file, std::ios::in);
  return !broken;
}

bool Tar_Helper::copy(Glib::ustring oldfilename, Glib::ustring newfilename)
{
  bool broken = false;
  Tar_Helper in(oldfilename, std::ios::in, broken);
  if (broken)
    return false;
  Tar_Helper out(newfilename, std::ios::out, broken);
  if (broken)
    {
      in.Close();
      return false;
    }
  std::list<Glib::ustring> delfiles;
  std::list<Glib::ustring> files = in.getFilenames();
  for (std::list<Glib::ustring>::iterator i = files.begin(); i != files.end(); 
       i++)
    {
      Glib::ustring filename = in.getFile (*i, broken);
      if (broken)
        break;
      delfiles.push_back(filename);
      bool success = false;
      if (*i == File::get_basename(oldfilename, true))
        success = out.saveFile(filename, File::get_basename(newfilename, true));
      else
        success = out.saveFile(filename, File::get_basename(filename, true));
      if (success == false)
        {
          broken = true;
          break;
        }
    }
  for (std::list<Glib::ustring>::iterator i = delfiles.begin(); 
       i != delfiles.end(); i++)
    File::erase(*i);
  in.Close();
  out.Close();
  if (broken)
    return false;
  return true;
}

void Tar_Helper::clean_tmp_dir(Glib::ustring filename)
{
  Glib::ustring tmpoutdir = 
    String::ucompose("%1/%2.%3/", Glib::get_tmp_dir(), 
                     File::get_basename(filename, true), getpid());
  File::clean_dir(tmpoutdir);
}
