// Copyright (C) 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2004, 2005 Andrea Paternesi
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "xmlhelper.h"
#include "defs.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

//they are only needed later for the expat callbacks
std::string my_cdata;
bool error = false;

// forward declarations of the internally used functions
void start_handler(void* udata, const XML_Char* name, const XML_Char** atts);
void character_handler(void* udata, const XML_Char* s, int len);
void end_handler(void* udata, const XML_Char* name);



XML_Helper::XML_Helper(std::string filename, std::ios::openmode mode, bool zip)
  : d_inbuf(0), d_outbuf(0), d_fout(0), d_fin(0), d_out(0), d_in(0),
    d_last_opened(""), d_version(""), d_failed(false), d_zip(zip)
{
    debug("Constructor called  -- " << zip)
        
    // always use a helper either for reading or for writing. Doing both
    // is propably possible, but there is little point in using it anyway.
    if ((mode & std::ios::in) && (mode & std::ios::out))
    {
        std::cerr <<_("XML_Helper: Either open file for reading or writing, not both, exiting\n");
        exit(-1);
    }

    //open input stream if required
    if (mode & std::ios::in)
    {
        d_fin = new std::ifstream(filename.c_str(), std::ios::in);

        if (!(*d_fin))
        //error opening
        {
            std::cerr <<filename <<_(": Error opening file for reading. Exiting\n");
            exit(-1);
        }

        char tmp;
        d_fin->read(&tmp,1);
        debug("IS ZIPPED ->" << tmp)                

        if (d_fin->eof())
        {
            std::cerr <<_("File too short (<=1 byte), propably broken. Skipping load\n");
            d_failed = true;
        }

        if (tmp=='Z') 
        {  
            std::cout <<filename <<_(": The file is obfuscated, attempting to read it....\n"); 
            d_fin->seekg (0,std::ios::end);
            long length = d_fin->tellg();
            d_fin->seekg (1, std::ios::beg);    

            uLongf destlen1=0;
            (*d_fin) >> destlen1;

            debug(destlen1 << " -- "<< length);

            length--;
            length-=sizeof(uLongf);

            char * buffer = (char*) malloc(length);        
            char * buf1 = (char *) malloc(destlen1);

            d_fin->read(buffer,length);
            d_fin->close();

            uncompress((Bytef*)buf1,&destlen1,(Bytef*)buffer,length);

            free(buffer);
            debug(destlen1 << " -- "<< length);
        
            d_inbuf = new std::istringstream(buf1);

            free(buf1);
            d_in=d_inbuf;
        }
        else 
        {
            //std::cout <<filename <<_(": The file is not obfuscated, attempting to read it....\n"); 
            d_fin->seekg(0, std::ios::beg);
            d_in = d_fin;
        }
    }

    if (mode & std::ios::out)
    {
        d_fout = new std::ofstream(filename.c_str(),
                                   std::ios::out & std::ios::trunc);
        if (!(*d_fout))
        {
            std::cerr <<filename <<_(": Error opening file for writing. Exiting\n");
            exit(-1);
        }

        d_outbuf = new std::ostringstream();
        d_out = d_outbuf;
    }
}

XML_Helper::XML_Helper(std::ostream* output)
  : d_inbuf(0), d_outbuf(0), d_fout(0), d_fin(0), d_out(0), d_in(0),
    d_last_opened(""), d_version(""), d_failed(false), d_zip(false)
{
  d_out = output;
}

XML_Helper::XML_Helper(std::istream* input)
  : d_inbuf(0), d_outbuf(0), d_fout(0), d_fin(0), d_out(0), d_in(0),
    d_last_opened(""), d_version(""), d_failed(false), d_zip(false)
{
  d_in = input;
}

XML_Helper::~XML_Helper()
{
    if (d_tags.size() != 0)
    // should never happen unless there was an error
        std::cerr <<_("XML_Helper: dtags not empty!!\n");
    
    debug("Called destructor\n")    
    close();
}

bool XML_Helper::begin(std::string version)
{
    d_version = version;
    (*d_out) <<"<?xml version=\"1.0\"?>\n";

    return true;
}

bool XML_Helper::openTag(std::string name)
{
    if (!d_out)
    {
        std::cerr <<_("XML_Helper: no output stream given\n");
        return false;
    }

    if ((name[0] == 'd') && (name[1] == '_'))
    {
        std::cerr <<name <<_(": The tag name starts with a \"d\". Not creating tag!\n");
        return false;
    }

    addTabs();

    // append the version strin got the first opened tag
    if (d_tags.empty())
        (*d_out) <<"<" <<name <<" version=\"" <<d_version <<"\">\n";
    else
        (*d_out) <<"<" <<name <<">\n";
        
    d_tags.push_front(name);
    return true;
}

bool XML_Helper::closeTag()
{
    if (!d_out)
    {
        std::cerr <<_("XML_Helper: no output stream given.\n");
        return false;
    }

    std::string name = (*d_tags.begin());
    
    //remove tag from list
    d_tags.pop_front();

    addTabs();
    (*d_out) <<"</" <<name <<">\n";

    return true;
}

bool XML_Helper::saveData(std::string name, std::string value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr <<_("XML_Helper: save_data with empty name\n");
        return false;
    }
    if (!d_out)
    {
        std::cerr <<_("XML_Helper: no output stream given.\n");
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(std::string name, int value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr <<_("XML_Helper: save_data with empty name\n");
        return false;
    }
    if (!d_out)
    {
        std::cerr <<_("XML_Helper: no output stream given.\n");
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(std::string name, Uint32 value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr <<_("XML_Helper: save_data with empty name\n");
        return false;
    }
    if (!d_out)
    {
        std::cerr <<_("XML_Helper: no output stream given.\n");
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(std::string name, bool value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr <<_("XML_Helper: save_data with empty name\n");
        return false;
    }
    if (!d_out)
    {
        std::cerr <<_("XML_Helper: no output stream given.\n");
        return false;
    }

    std::string s;
    s = (value? "true" : "false");

    addTabs();
    (*d_out) <<"<" <<name <<">" <<s <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(std::string name, double value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr <<_("XML_Helper: save_data with empty name\n");
        return false;
    }
    if (!d_out)
    {
        std::cerr <<_("XML_Helper: no output stream given.\n");
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

/* This is a wrapper for the AMD64 platform */
bool XML_Helper::saveData(std::string name, unsigned long int value)
{
    return saveData(name, static_cast<Uint32>(value));
}
/* End wrapper AMD64 */

bool XML_Helper::close()
{
    if (d_outbuf)        
    {
        if (d_zip)        
        {
            debug("I zip IT")
            debug(_("Saving game and obfuscating the Savefile.\n")) 
            std::string tmp = d_outbuf->str();
            tmp+='\0';

            long origlength = tmp.length();
            uLongf ziplength = static_cast<uLongf>(origlength + (12+0.01*origlength));
            char *buf1= (char*) malloc(ziplength);
         
            int ret = compress2((Bytef*)buf1, &ziplength,
                                (Bytef*)tmp.c_str(), (uLong)origlength, 9);
            debug("RET="<<ret<<" length=" << origlength << " destlen1=" << ziplength)

            (*d_fout) << 'Z';
            (*d_fout) << origlength;

            d_fout->write(buf1, ziplength);
            d_fout->flush();

            free(buf1);
            delete d_outbuf;

            d_outbuf = 0;
            debug("destroyed d_outbuf")
            ret=0;// to avoid warning 
        }
        else 
        {
            debug("I do not zip IT")
            debug(_("Saving game without obfuscation.\n")) 

            std::string tmp = d_outbuf->str();
            d_fout->write(tmp.c_str(), tmp.length());
            d_fout->flush();

            delete d_outbuf;
            d_outbuf = 0;
            debug("destroyed d_outbuf")
        }
    }

    if (d_inbuf)        
    {
        delete d_inbuf;
        d_inbuf = 0;
        debug("destroyed d_inbuf")
    }

    if (d_fout)
    {
        d_fout->close();
        delete d_fout;
        d_fout = 0;
    }

    if (d_fin)
    {
        d_fin->close();
        delete d_fin;
        d_fin = 0;
    }

    d_out = 0;
    d_in = 0;
        
    return true;
}

void XML_Helper::addTabs()
{
    for (unsigned int i = d_tags.size(); i > 0; i--)
        (*d_out)<<"\t";
}

//loading
bool XML_Helper::registerTag(std::string tag, XML_Slot callback)
{
    //register tag as important
    d_callbacks[tag] = callback;

    return true;
}


bool XML_Helper::unregisterTag(std::string tag)
{
    std::map<std::string, XML_Slot>::iterator it = d_callbacks.find(tag);

    if (it == d_callbacks.end())
    //item doesn't exist
        return false;
    
    d_callbacks.erase(it);
    return true;
}

bool XML_Helper::getData(std::string& data, std::string name)
{
    //the data tags are stored with leadin "d_", so prepend it here
    name = "d_" + name;

    std::map<std::string, std::string>::const_iterator it;

    it = d_data.find(name);
    
    if (it == d_data.end())
    {
        data = "";
        std::cerr<<"XML_Helper::getData(std::string, \"" <<name <<"\") failed\n";
        d_failed = true;
        return false;
    }
    
    data = (*it).second;

    return true;
}

bool XML_Helper::getData(bool& data, std::string name)
{
    //the data tags are stored with leadin "d_", so prepend it here
    name = "d_" + name;

    std::map<std::string, std::string>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<"XML_Helper::getData(bool, \"" <<name <<"\") failed\n";
        d_failed = true;
        return false;
    }
    
    if ((*it).second == "true")
    {
        data = true;
        return true;
    }

    if ((*it).second == "false")
    {
        data = false;
        return true;
    }
    
    return false;
}

bool XML_Helper::getData(int& data, std::string name)
{
    //the data tags are stored with leadin "d_", so prepend it here
    name = "d_" + name;

    std::map<std::string, std::string>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<"XML_Helper::getData(int, \"" <<name <<"\") failed\n";
        d_failed = true;
        return false;
    }
    
    data = atoi((*it).second.c_str());
    return true;
}

bool XML_Helper::getData(Uint32& data, std::string name)
{
    //the data tags are stored with leadin "d_", so prepend it here
    name = "d_" + name;

    std::map<std::string, std::string>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<"XML_Helper::getData(Uint32, \"" <<name <<"\") failed\n";
        d_failed = true;
        return false;
    }
    
    data = static_cast<Uint32>(atoi((*it).second.c_str()));
    return true;

    
}

bool XML_Helper::getData(double& data, std::string name)
{
    //the data tags are stored with leadin "d_", so prepend it here
    name = "d_" + name;

    std::map<std::string, std::string>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<"XML_Helper::getData(double, \"" <<name <<"\") failed\n";
        d_failed = true;
        return false;
    }

    data = strtod((*it).second.c_str(), 0);
    return true;
}

bool XML_Helper::parse()
{
    if (!d_in || d_failed)
        return false;
        //what's the use of parsing no or incorrect input?

    d_parser = XML_ParserCreate("utf-8");

    //set handlers and data
    XML_SetElementHandler(d_parser, start_handler, end_handler);
    XML_SetCharacterDataHandler(d_parser, character_handler);
    XML_SetUserData(d_parser, static_cast<void*>(this));

    while (!d_in->eof() && !d_failed)
    {
        char* buffer = static_cast<char*>(XML_GetBuffer(d_parser,1000));
        d_in->getline(buffer, 1000);
        bool my_eof = d_in->eof();

        if (!XML_ParseBuffer(d_parser, strlen(buffer), my_eof))
        //error parsing
        {
            std::cerr <<_("XML_Helper: error parsing xml document.\n");
            std::cerr <<_("Line ") <<XML_GetCurrentLineNumber(d_parser);
            std::cerr <<": " <<XML_ErrorString(XML_GetErrorCode(d_parser)) <<"\n";
            std::cerr << _("Buffercontent = ") << buffer << std::endl;
            d_failed = true;
        }
    }

    XML_ParserFree(d_parser);

    return (!d_failed);
}

//beginning with here is only internal stuff. Continue reading only if you are
//interested in the xml parsing. :)

/* Parsing works like this: We have three expat callback functions,
 * startelementhandler, endelementhandler and cdatahandler.
 * The startelement handler just calls XML_Helper::tag_open (an object is passed
 * as data), the cdata element handler just sums up the cdata, and the end
 * element handler calls XML_Helper::tag_close, giving it also the final cdata
 * string (the string between opened tag and closed tag) to the XML_Helper.
 * Since data is always stored like "<mydata>data_value</mydata>", having
 * a startelementhandler encounter a non-null summed up cdata string is a
 * serious error and results in a fail.
 *
 * Now the XML_Helper functions: 
 * tag_open looks if another important tag has already been opened last (and not
 * called back). If so, it assumes that all important data has already been
 * stored and calls the callback for the former tag. If not, it just goes on.
 * last_opened is always set to the last opened tag marked as important.
 * If tag_close is called, it is mostly for data. If cdata is != 0 it is some
 * saved data. If the last_opened tag is the same as the closed tag (we disallow
 * and thus ignore constructions like "<mytag> <mytag> </mytag> </mytag>" here,
 * they are IMO pointless), we suppose that the callback has not been called yet
 * and do it now. If not, then there has been another important tag on the way
 * which has led tag_open to already call the callback.
 */

bool XML_Helper::tag_open(std::string tag, std::string version)
{
    if (d_failed)
        return false;
        
    //first of all, register the tag as opened
    d_tags.push_front(tag);

    if (version != "")
        d_version = version;

    //look if the tag starts with "d_". If so, it is a data tag without anything
    //important in between
    if ((tag[0] == 'd') && (tag[1] == '_'))
        return true;
    
    //first of all, look if another important tag has already been opened
    //and call the appropriate callback if so
    std::list<std::string>::iterator ls_it;
    ls_it = d_tags.begin();
    ls_it++;

    if ((ls_it != d_tags.end()) && (d_last_opened == (*ls_it)))
    {
        std::map<std::string, XML_Slot>::iterator it;
        it = d_callbacks.find(*ls_it);

        
        if (it != d_callbacks.end())
        {
            //make the callback (yes that is the syntax, overloaded "()")
            bool retval = (it->second)(*ls_it, this);
            if (retval == false)
            {
                std::cerr <<(*ls_it) <<_(": Callback for tag returned false. Stop parsing document.\n");
                error = true;
                d_failed = true;
            }
        }

        //clear d_data (we are just setting up a new tag)
        d_data.clear();
    }

    d_last_opened = tag;

    return true;
}

bool XML_Helper::tag_close(std::string tag, std::string cdata)
{
    if (d_failed)
        return false;

    //remove tag entry, there is nothing more to be done
    d_tags.pop_front();
    
    if ((tag[0] == 'd') && (tag[1] == '_'))
    {
        // save the data (we close a data tag)
        d_data[tag] = cdata;
        return true;    //data tags end here with their execution
    }

    if ((d_last_opened == tag))
    //callback hasn't been called yet
    {
        std::map<std::string, XML_Slot>::iterator it;
        it = d_callbacks.find(tag);
        
        if (it != d_callbacks.end())
        {
            //make the callback (yes that is the syntax, overloaded "()")
            bool retval = it->second(tag, this);
            
            if (retval == false)
            {
                std::cerr <<tag <<_(": Callback for tag returned false. Stop parsing document.\n");
                error = true;
                d_failed = true;
            }
        }
    }

    //clear d_data (we are just setting up a new tag)
    d_data.clear();
    
    return true;
}

//these functions are the expat callbacks. Their main purpose is to set up
//the parametres and call the tag_open and tag_close functions in XML_Helper.

//the start handler, call open_tag and do misc init stuff
void start_handler(void* udata, const XML_Char* name, const XML_Char** atts)
{
    if (error)
        return;
    
    XML_Helper* helper = static_cast<XML_Helper*>(udata);
    std::string version = "";

    //the only attribute we know and handle are version strings
    if ((atts[0] != 0) && (std::string(atts[0]) == "version"))
        version = std::string(atts[1]);

    my_cdata = "";

    error = !helper->tag_open(std::string(name), version);
}

//the cdata handler, just sums up the string  s
void character_handler(void* udata, const XML_Char* s, int len)
{
    if (error)
        return;

    char buffer[len+1];     //TODO: this is a gcc extension, very handy, but
                            //not neccessarily portable
    
    strncpy(buffer, s, len);
    buffer[len] = '\0';

    //now add the string to the other one
    my_cdata += std::string(buffer);
}

//the end handler: call tag_close and dosome cleanup
void end_handler(void* udata, const XML_Char* name)
{
    if (error)
        return;
    
    XML_Helper* helper = static_cast<XML_Helper*>(udata);

    error = !helper->tag_close(std::string(name), my_cdata);

    my_cdata = "";
}
