#include "tarhelper.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include "File.h"
#include "ucompose.hpp"

Tar_Helper::Tar_Helper(std::string file, std::ios::openmode mode)
{
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
    return;

  char *f = strdup(file.c_str());
  int retval = tar_open (&t, f, NULL, m, perms, TAR_GNU);
  free(f);
  if (retval == -1)
    t = NULL;
      
  if (mode & std::ios::in)
    {
      std::list<std::string> files = getFilenames();
      tmpoutdir = String::ucompose("%1/%2.%3/", Glib::get_tmp_dir(), files.front(), getpid());
      File::create_dir(tmpoutdir);
    }
  else
    tmpoutdir = "";

}

bool Tar_Helper::saveFile(std::string filename, std::string destfile)
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

void Tar_Helper::Close()
{
  if (t)
    {
      if (openmode & std::ios::out)
	tar_append_eof(t);
    
      tar_close(t);
      t = NULL;
      if (tmpoutdir != "")
	File::erase_dir(tmpoutdir);
    }
}
    
std::string Tar_Helper::getFirstFile(bool &broken)
{
  std::list<std::string> files = getFilenames();
  return getFile(files.front(), broken);
}

std::string Tar_Helper::getFile(std::string filename, bool &broken)
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
  fwrite(data, 1, size, fileptr);
  fclose(fileptr);

  broken = false;
  return outfile;
}

std::list<std::string> Tar_Helper::getFilenames()
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

