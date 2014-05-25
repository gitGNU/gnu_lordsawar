// Copyright (C) 2011, 2014 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include <cstdio>
#include <limits.h>
#include <fstream>
#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <sigc++/functors/mem_fun.h>

#include "xmlhelper.h"
#include "Configuration.h"
#include "defs.h"
#include "File.h"
#include "file-compat.h"
#include "tarhelper.h"
#include "GameScenario.h"
#include "ucompose.hpp"
#include "recently-edited-file-list.h"
#include "Itemlist.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

FileCompat* FileCompat::s_instance = 0;

class UpgradeDetails
{
public:
  UpgradeDetails(std::string f, std::string t, FileCompat::Slot s) 
    {from_version = f; to_version = t; slot = s;};
  std::string from_version;
  std::string to_version;
  FileCompat::Slot slot;
};

FileCompat* FileCompat::getInstance()
{
  if (s_instance == 0)
    s_instance = new FileCompat();

  return s_instance;
}

void FileCompat::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

bool FileCompat::contains(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i).type == type)
      return true;
  return false;
}

void FileCompat::support_backward_compatibility_for_common_files()
{
  if (FileCompat::getInstance()->contains(CONFIGURATION) == false)
    Configuration::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(ITEMLIST) == false)
    Itemlist::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(RECENTLYEDITEDFILELIST) == false)
    RecentlyEditedFileList::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(ARMYSET) == false)
    Armyset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(TILESET) == false)
    Tileset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(CITYSET) == false)
    Cityset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(SHIELDSET) == false)
    Shieldset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(GAMESCENARIO) == false)
    GameScenario::support_backward_compatibility();
}

FileCompat::FileCompat()
{
}

FileCompat::~FileCompat()
{
}

FileCompat::Type FileCompat::getType(std::string filename) const
{
  if (File::exists(filename) == false)
    return UNKNOWN;
  for (const_iterator i = begin(); i != end(); i++)
    {
      if (File::nameEndsWith(filename, (*i).file_extension) == true &&
          (*i).file_extension != ".xml")
        {
          return FileCompat::Type((*i).type);
        }
    }
  return getTypeByFileInspection(filename);
}
  
FileCompat::Type FileCompat::getTypeByTarFileInspection(std::string filename) const
{
  bool broken = false;
  Tar_Helper t(filename, std::ios::in, broken);
  if (broken)
    return UNKNOWN;

  std::list<std::string> files = t.getFilenames();
  t.Close();
  std::list<FileDetails> details;
  //whittle down the files it can't be
  for (std::list<std::string>::iterator i = files.begin(); i != files.end(); 
       i++)
    {
      bool found = false;
      for (const_iterator j = begin(); j != end(); j++)
        {
          if (File::nameEndsWith(*i, (*j).file_extension) == true)
            {
              if ((*j).type == GAMESCENARIO)
                return GAMESCENARIO;
              details.push_back(*j);
              found = true;
              break;
            }
        }
      if (!found)
        {
          i = files.erase(i);
          continue;
        }
    }

  if (details.size() == 0)
    return UNKNOWN;
  else
    return FileCompat::Type(details.front().type);
}

FileCompat::Type FileCompat::getTypeByXmlFileInspection(std::string filename) const
{
  std::string tag = XML_Helper::get_top_tag(filename, 
                                            Configuration::s_zipfiles);
  if (tag == "")
    tag = XML_Helper::get_top_tag(filename, !Configuration::s_zipfiles);

  if (tag == "")
    return UNKNOWN;
  for (const_iterator i = begin(); i != end(); i++)
    {
      if (tag == (*i).tag)
        return FileCompat::Type((*i).type);
    }
  return UNKNOWN;
}

FileCompat::Type FileCompat::getTypeByFileInspection(std::string filename) const
{
  Type type = getTypeByTarFileInspection(filename);
  if (type == UNKNOWN)
    return getTypeByXmlFileInspection(filename);
  else
    return type;
}

bool FileCompat::get_tag_and_version_from_file(std::string filename, FileCompat::Type type, std::string &tag, std::string &version) const
{
  bool broken = false;
  if (isTarFile(type) == true)
    {
      std::string ext = getFileExtension(type);
      if (ext == "")
        return false;
      Tar_Helper t(filename, std::ios::in, broken);
      if (!broken)
        {
          std::string tmpfile = t.getFirstFile(ext, broken);
          XML_Helper helper(tmpfile, std::ios::in, Configuration::s_zipfiles);
          tag = XML_Helper::get_top_tag(tmpfile, Configuration::s_zipfiles);
          VersionLoader l(tmpfile, tag, version, broken, 
                          Configuration::s_zipfiles);
          t.Close();
          File::erase(tmpfile);
        }
    }
  else
    {
      tag = XML_Helper::get_top_tag(filename, false);
      VersionLoader l(filename, tag, version, broken);
    }
  return !broken;
}

bool FileCompat::isTarFile(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i).type == type)
      return (*i).tar;
  return false;
}

std::string FileCompat::getFileExtension(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i).type == type)
      return (*i).file_extension;
  return "";
}

std::string FileCompat::getTag(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i).type == type)
      return (*i).tag;
  return "";
}

bool FileCompat::upgrade(std::string filename, bool &same) const
{
  std::string tag, version;
  bool upgraded = false;
  if (File::exists(filename) == false)
    return false;
  Type type = getType(filename);
  if (type == UNKNOWN)
    return false;
  if (get_tag_and_version_from_file (filename, type, tag, version) == false)
    return false;
      
  //can we get there from here?
  Slot slot;
  std::string next_version;
  if (get_upgrade_method(type, version, next_version, slot) == false)
    {
      same = can_upgrade_to(type, version);
      return false;
    }

  bool broken = false;
  // ride the upgrade train as far as we can.
  while (1)
    {
      if (get_upgrade_method(type, version, next_version, slot))
        {
          upgraded = (slot)(filename, version, next_version);
          version = next_version;
          if (upgraded == false)
            {
              broken = true;
              break;
            }
        }
      else
        break;
    }
  return !broken;
}
      
bool FileCompat::can_upgrade_to(FileCompat::Type type, std::string version) const
{
  for (std::list<UpgradeDetails>::const_iterator i = versions[type].begin();
       i != versions[type].end(); i++)
    {
      if ((*i).to_version == version)
        return true;
    }
  return false;
}

bool FileCompat::get_upgrade_method(FileCompat::Type type, std::string version, std::string &next_version, FileCompat::Slot &slot) const
{
  for (std::list<UpgradeDetails>::const_iterator i = versions[type].begin();
       i != versions[type].end(); i++)
    {
      if ((*i).from_version == version)
        {
          next_version = (*i).to_version;
          slot = (*i).slot;
          return true;
        }
    }
  return false;
}

bool FileCompat::rewrite_with_updated_version(std::string filename, FileCompat::Type type, std::string tag, std::string version) const
{
  bool broken = false;
  bool upgraded = false;
  if (isTarFile(type) && type != GAMESCENARIO)
    {
      std::string ext = getFileExtension(type);

      Tar_Helper t(filename, std::ios::in, broken);
      if (broken == false)
        {
          std::string tmpfile = t.getFirstFile(ext, broken);
          if (broken == false && version != "")
            upgraded = XML_Helper::rewrite_version(tmpfile, tag, version, 
                                                   Configuration::s_zipfiles);
          if (upgraded)
            t.replaceFile (t.getFilenamesWithExtension(ext).front(), tmpfile);
          t.Close();
          if (tmpfile != "")
            File::erase(tmpfile);
        }
    }
  else if (isTarFile(type) && type == GAMESCENARIO)
    {
      return upgradeGameScenario(filename, version);
    }
  else if (isTarFile(type) == false)
    {
      if (version != "")
        upgraded = XML_Helper::rewrite_version(filename, tag, version, false);
    }
  return upgraded;
}

FileCompat::Type FileCompat::getTypeByFileExtension(std::string ext) const
{
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i).file_extension == ext)
      return FileCompat::Type((*i).type);
  return UNKNOWN;
}

bool FileCompat::upgradeGameScenario(std::string filename, std::string version) const
{
  std::string ext = File::get_extension(filename);
  if (ext == "")
    return false;
  if (getTypeByFileExtension(ext) != GAMESCENARIO)
    return false;
  bool upgraded = false;
  bool broken = false;
  Tar_Helper t(filename, std::ios::in, broken);
  if (!broken)
    {
      std::string tmpfile = t.getFirstFile(ext, broken);
      if (broken == false)
        upgraded = XML_Helper::rewrite_version(tmpfile, getTag(GAMESCENARIO), 
                                               version,
                                               Configuration::s_zipfiles);
      std::list<std::string> delfiles;
      delfiles.push_back(tmpfile);
      if (upgraded)
        {
          bool same;
          t.replaceFile (t.getFilenamesWithExtension(ext).front(), tmpfile);
          //now we need to upgrade the other files.
          std::string f = t.getFilenamesWithExtension
            (getFileExtension(ARMYSET)).front();
          tmpfile = t.getFile(f, broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (f, tmpfile);
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(TILESET), broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (t.getFilenamesWithExtension
                               (getFileExtension(TILESET)).front(), tmpfile);
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(CITYSET), broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (t.getFilenamesWithExtension
                               (getFileExtension(CITYSET)).front(), tmpfile);
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(SHIELDSET), broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (t.getFilenamesWithExtension
                               (getFileExtension(SHIELDSET)).front(), tmpfile);
              delfiles.push_back(tmpfile);
            }
        }
      for (std::list<std::string>::iterator i = delfiles.begin(); i != delfiles.end(); i++)
        File::erase(*i);
      t.Close();
    }
  return upgraded;
}

void FileCompat::initialize()
{
  bool same;
  if (contains(GAMELIST))
    upgrade(File::getUserRecentlyAdvertisedGamesDescription(), same);
  if (contains(GAMELIST))
    upgrade(File::getUserRecentlyHostedGamesDescription(), same);
  if (contains(PROFILELIST))
    upgrade(File::getUserProfilesDescription(), same);
  if (contains(RECENTLYPLAYEDGAMELIST))
    upgrade(File::getUserRecentlyPlayedGamesDescription(), same);
  if (contains(RECENTLYEDITEDFILELIST))
    upgrade(File::getUserRecentlyEditedFilesDescription(), same);
}

Glib::ustring FileCompat::typeToString(const FileCompat::Type type)
{
  switch (type)
    {
    case UNKNOWN:
      return _("unknown file");
    case CONFIGURATION:
      return _("primary configuration file");
    case ITEMLIST:
      return _("item description file");
    case PROFILELIST:
      return _("profiles file");
    case RECENTLYPLAYEDGAMELIST:
      return _("recently played games file");
    case GAMELIST:
      return _("recently hosted or recently advertised games file");
    case RECENTLYEDITEDFILELIST:
      return _("recently edited documents file");
    case PBMTURN:
      return _("play-by-mail turn file");
    case ARMYSET:
      return _("armyset file");
    case TILESET:
      return _("tileset file");
    case CITYSET:
      return _("cityset file");
    case SHIELDSET:
      return _("shieldset file");
    case GAMESCENARIO:
      return _("map or saved-game file");
    }
  return _("unknown file");
}

Glib::ustring FileCompat::typeToCode(const FileCompat::Type type)
{
  switch (type)
    {
    case UNKNOWN:
      return "";
    case CONFIGURATION:
      return "c";
    case ITEMLIST:
      return "il";
    case PROFILELIST:
      return "pl";
    case RECENTLYPLAYEDGAMELIST:
      return "rpg";
    case GAMELIST:
      return "gl";
    case RECENTLYEDITEDFILELIST:
      return "ref";
    case PBMTURN:
      return "pbm";
    case ARMYSET:
      return "as";
    case TILESET:
      return "ts";
    case CITYSET:
      return "cs";
    case SHIELDSET:
      return "ss";
    case GAMESCENARIO:
      return "gs";
    }
  return "";
}
        
        
void FileCompat::support_version(guint32 k, std::string from, std::string to, FileCompat::Slot slot)
{
  versions[FileCompat::Type(k)].push_back(UpgradeDetails(from, to, slot));
}

bool FileCompat::rewrite_with_xslt(std::string filename, FileCompat::Type type, std::string xsl_file) const
{
  bool broken = false;
  bool upgraded = false;
  if (isTarFile(type) && type != GAMESCENARIO)
    {
      std::string ext = getFileExtension(type);

      Tar_Helper t(filename, std::ios::in, broken);
      if (broken == false)
        {
          std::string tmpfile = t.getFirstFile(ext, broken);
          if (broken == false)
            upgraded = xsl_transform(tmpfile, xsl_file);
          if (upgraded)
            t.replaceFile (t.getFilenamesWithExtension(ext).front(), tmpfile);
          t.Close();
          if (tmpfile != "")
            File::erase(tmpfile);
        }
    }
  else if (isTarFile(type) && type == GAMESCENARIO)
    {
      return upgradeGameScenarioWithXslt(filename, xsl_file);
    }
  else if (isTarFile(type) == false)
    {
      upgraded = xsl_transform(filename, xsl_file);
    }
  return upgraded;
}

bool FileCompat::upgradeGameScenarioWithXslt(std::string filename, std::string xsl_file) const
{
  std::string ext = File::get_extension(filename);
  if (ext == "")
    return false;
  if (getTypeByFileExtension(ext) != GAMESCENARIO)
    return false;
  bool upgraded = false;
  bool broken = false;
  Tar_Helper t(filename, std::ios::in, broken);
  if (!broken)
    {
      std::string tmpfile = t.getFirstFile(ext, broken);
      if (broken == false)
        upgraded = xsl_transform(tmpfile, xsl_file);

      std::list<std::string> delfiles;
      delfiles.push_back(tmpfile);
      if (upgraded)
        {
          bool same;
          t.replaceFile (t.getFilenamesWithExtension(ext).front(), tmpfile);
          //now we need to upgrade the other files.
          std::string f = t.getFilenamesWithExtension
            (getFileExtension(ARMYSET)).front();
          tmpfile = t.getFile(f, broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (f, tmpfile);
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(TILESET), broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (t.getFilenamesWithExtension
                               (getFileExtension(TILESET)).front(), tmpfile);
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(CITYSET), broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (t.getFilenamesWithExtension
                               (getFileExtension(CITYSET)).front(), tmpfile);
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(SHIELDSET), broken);
          if (tmpfile != "")
            {
              if (upgrade(tmpfile, same))
                t.replaceFile (t.getFilenamesWithExtension
                               (getFileExtension(SHIELDSET)).front(), tmpfile);
              delfiles.push_back(tmpfile);
            }
        }
      for (std::list<std::string>::iterator i = delfiles.begin(); i != delfiles.end(); i++)
        File::erase(*i);
      t.Close();
    }
  return upgraded;
}

        
bool FileCompat::xsl_transform(std::string filename, std::string xsl_file) const
{
  const char *params[16 + 1];
  //int nbparams = 0;
  memset (params, 0, sizeof (params));
  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  xmlChar *xsl = xmlCharStrdup(xsl_file.c_str());
  cur = xsltParseStylesheetFile(xsl);
  if (cur == NULL)
    return false;
  doc = xmlParseFile(filename.c_str());
  if (doc == NULL)
    return false;
  res = xsltApplyStylesheet(cur, doc, params);
  if (res == NULL)
    return false;

  std::string tmpfile = File::get_tmp_file();

  xmlChar *out = xmlCharStrdup(tmpfile.c_str());
  xsltSaveResultToFilename(tmpfile.c_str(), res, cur, 0);
  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);
  free (xsl);
  free (out);

  xsltCleanupGlobals();
  xmlCleanupParser();
  File::erase(filename);
  rename(tmpfile.c_str(), filename.c_str());
  return true;
}

bool FileCompat::upgrade(std::string filename, std::string old_version, std::string new_version, FileCompat::Type type, std::string tag) const
{
  std::string xsl_filename = File::getXSLTFile(type, old_version, new_version);
  if (xsl_filename != "")
    return rewrite_with_xslt (filename, type, xsl_filename);
  else
    return rewrite_with_updated_version (filename, type, tag, new_version);
}

// End of file
