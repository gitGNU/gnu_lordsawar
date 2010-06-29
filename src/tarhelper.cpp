#include "tarhelper.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include "File.h"
#include <errno.h>
#include "ucompose.hpp"

Tar_Helper::Tar_Helper(std::string file, std::ios::openmode mode, bool &broken)
{
  t = NULL;
  broken = Open(file, mode);
}

bool Tar_Helper::Open(std::string file, std::ios::openmode mode)
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
      std::list<std::string> files = getFilenames();
      tmpoutdir = String::ucompose("%1/%2.%3/", Glib::get_tmp_dir(), files.front(), getpid());
      File::create_dir(tmpoutdir);
    }
  else
    tmpoutdir = "";
  return broken;
}

bool Tar_Helper::saveFile(TAR *t, std::string filename, std::string destfile)
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

bool Tar_Helper::saveFile(std::string filename, std::string destfile)
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
    
std::string Tar_Helper::getFirstFile(std::string extension, bool &broken)
{
  std::list<std::string> files = getFilenamesWithExtension(extension);
  return getFile(files.front(), broken);
}

std::string Tar_Helper::getFirstFile(bool &broken)
{
  std::list<std::string> files = getFilenames();
  return getFile(files.front(), broken);
}

std::string Tar_Helper::getFile(TAR *t, std::string filename, bool &broken, std::string tmpoutdir)
{
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
  std::string outfile = tmpoutdir + filename;
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

std::string Tar_Helper::getFile(std::string filename, bool &broken)
{
  return getFile(t, filename, broken, tmpoutdir);
}

std::list<std::string> Tar_Helper::getFilenames(TAR *t)
{
  std::list<std::string> result;
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
std::list<std::string> Tar_Helper::getFilenames()
{
  return getFilenames(t);
}
std::list<std::string> Tar_Helper::getFilenamesWithExtension(std::string ext)
{
  std::list<std::string> result;
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

bool Tar_Helper::is_tarfile (std::string file)
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
      if (strcmp(header.magic, "ustar  ") != 0)
        retval = false;
    }
  fclose (f);
  return retval;
}

bool Tar_Helper::removeFile(std::string filename)
{
  return replaceFile(filename, "");
}

bool Tar_Helper::replaceFile(std::string filename, std::string newfilename)
{
  bool broken = false;
  //copy all files except this one into a new file
  int m;
  int perms = 0;
  m = O_WRONLY|O_CREAT;
  perms = 0644;
  TAR *new_tar = NULL;
  std::string newtmpoutdir = String::ucompose("%1/%2.%3/", Glib::get_tmp_dir(), File::get_basename(t->pathname), getpid());
  File::create_dir(newtmpoutdir);
  std::string new_tar_file = newtmpoutdir +"/" + File::get_basename(t->pathname);
  char *f = strdup (new_tar_file.c_str());
  tar_open (&new_tar, f, NULL, m, perms, TAR_GNU);
  free (f);
  std::list<std::string> files = getFilenames();
  std::list<std::string> delfiles;
  for (std::list<std::string>::iterator it = files.begin(); it != files.end(); 
       ++it)
    {
      if (*it == filename) //here we skip over the one we want to remove
        continue;
      std::string extracted_file = getFile(t, *it, broken, newtmpoutdir);
      delfiles.push_back(extracted_file);
      if (broken)
        break;
    }
  for (std::list<std::string>::iterator it = delfiles.begin(); 
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
  std::string orig_tar_file = t->pathname;

  tar_close(new_tar);
  Close();

  File::copy(new_tar_file, orig_tar_file);
  File::erase(new_tar_file);
  File::erase_dir(newtmpoutdir);
  broken = Open (orig_tar_file, std::ios::in);
  return !broken;
}

bool Tar_Helper::copy(std::string oldfilename, std::string newfilename)
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
  std::list<std::string> delfiles;
  std::list<std::string> files = in.getFilenames();
  for (std::list<std::string>::iterator i = files.begin(); i != files.end(); 
       i++)
    {
      std::string filename = in.getFile (*i, broken);
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
  for (std::list<std::string>::iterator i = delfiles.begin(); 
       i != delfiles.end(); i++)
    File::erase(*i);
  in.Close();
  out.Close();
  if (broken)
    return false;
  return true;
}

void Tar_Helper::clean_tmp_dir(std::string filename)
{
  std::string tmpoutdir = 
    String::ucompose("%1/%2.%3/", Glib::get_tmp_dir(), 
                     File::get_basename(filename, true), getpid());
  File::clean_dir(tmpoutdir);
}
