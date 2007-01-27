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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

//  Andrea Paternesi 17/02/2005

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include "expat.h"
#include "string_tokenizer.h"

#ifdef __WIN32__
#include "../win32.h"
#endif

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

#define BUFFSIZE 8192

using namespace std;

char Buff[BUFFSIZE];
string elname;
string mydata="";
string mydatatoken="";
int tagnumber=0;

void start(void *data, const char *el, const char **attr) {
  elname=string(el);
  mydata="";
}  

void end(void *data, const char *el) {

  elname=string("");
  if (mydata!="") {

    std::vector<int> positions;
    std::vector<std::string> *s=0;

    stringTokenizer *sit= new stringTokenizer(mydata,"\""); 
    s=sit->getTokens();
    positions=sit->getPositions();
    
    std::vector<std::string>::const_iterator it=s->begin();
    int pos=0;
    while (it!=s->end()) {
      if (*it!=mydata) {

	if (positions[pos]==0 && s->size()==1) {
	  debug("UNIQUE TOKEN :-> " << *it)
	  mydatatoken+=string("\\\"")+*it+string("\\\"");
	}
	else if (positions[pos]==0 && s->size()!=1) {
	  debug("INITIAL TOKEN WITH \":-> " << *it);
	  mydatatoken+=string("\\\"")+*it+string("\\\"");
	}
	else if ((*it)==((*s)[s->size()-1]) && (positions[pos]+1+((*it).size()))==(mydata.size()-1)) {
	  debug("FINAL TOKEN WITH \" :-> " << *it)
	  //std::cerr << "lun1=" << ((*positions)[pos]+1+((*it).size())) << " LUN2=" << (mydata.size()-1) << std::endl;
	  mydatatoken+=*it+string("\\\"");
        }
        else if ((*it)==((*s)[s->size()-1]) && (positions[pos]+1)!=(int)mydata.size()) {
	  debug("FINAL TOKEN WITHOUT \" :-> " << *it)
	  mydatatoken+=*it;
        }
	else {
	  debug("INTERNAL TOKEN :-> " << *it)
	  mydatatoken+=*it+string("\\\"");
	}
	pos++;
      }
      else mydatatoken=mydata;
      it++;
    }
    mydata=mydatatoken;
    
    delete s;
    delete sit;
   
    cout << string("_(\"") << mydata << string("\");") << endl;
  }
  mydata="";
  mydatatoken="";
}  

void charh(void *data, const char *txt, int txtlen) {

  /*  if (((elname==string("d_name")) && tagnumber==0 ||
     ((elname==string("d_message")) && tagnumber==1)) ||
     (elname==string("d_comment"))) { 
  */

  if ((elname==string("d_name")) ||
     (elname==string("d_message")) ||
     (elname==string("d_comment")) ||
     (elname == string("d_description"))) { 
       
    char tmp[txtlen];
    string tmp1;
    tmp[txtlen]='\0';
    strncpy(tmp,txt,txtlen);
    tmp1=string(tmp);

    if ( (txtlen!=1) && (tmp1!="\n")) {
      mydata+=tmp1;
    }
  }
}  

int main(int argc, char **argv) {

  if (argc!=2) {
    fprintf(stdout, "Usage: extractv tagnumber(0=name,1=message)\n");
    exit(-1);
  }
  
  tagnumber=atoi(argv[1]);  

  XML_Parser p = XML_ParserCreate(NULL);
  if (! p) {
    cerr << "Couldn't allocate memory for parser\n" << endl;
    exit(-1);
  }

  XML_SetElementHandler(p, start, end);
  XML_SetCharacterDataHandler(p, charh);

  for (;;) {
    int done;
    int len;

    len = fread(Buff, 1, BUFFSIZE, stdin);
    if (ferror(stdin)) {
      cerr << "Read error\n" <<endl;
      exit(-1);
    }
    done = feof(stdin);

    if (! XML_Parse(p, Buff, len, done)) {
      cerr << "Parse error at line " 
           << XML_GetCurrentLineNumber(p) 
           << ":\n"
           << XML_ErrorString(XML_GetErrorCode(p)) 
           << endl;
      exit(-1);
    }

    if (done)
      break;
  }
  return 0;
}

