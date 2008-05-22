// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004, 2005 Ulf Lorenz
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

#ifndef STRING_TOKENIZER_H
#define STRING_TOKENIZER_H

#include <string>
#include <iterator>
#include <vector>
#include <iostream>

/** This class takes a string and a list of separators as input and
  * breaks the string down into tokens separated by the separator.
  * This class is used during saving. We test if the filename ends on e.g.
  * ".sav" and append a suffix if neccessary.
  *
  * However, this class has a more general approach, so you can also get
  * a list of all tokens as well etc.
  *
  * Finally, an example of how to use it properly:
  *

  int main()
  {

    string str="one,two..,..three{four five"; // WE ALLOCATE THE STRING TO BE TOKENIZED
    std::vector<std::string> *s=0;

    // WE DECLARE THE TOKENIZER WITH ALL THE SEPARATOR CHARACTERS 
    stringTokenizer *sit= new stringTokenizer(str,",.{ "); 

    //WE GET ALL THE TOKENS AND PRINT THEM TO THE STD OUT
    s=sit->getTokens();
    std::vector<std::string>::const_iterator it=s->begin();

    while (it!=s->end())
    {
        cerr << *it << endl;
        it++;
    }

    cerr << endl;
    // WE PRINT OUT THE LAST TOKEN
    cerr << sit->getLastToken() << endl<<endl;

    // WE PRINT OUT THE FIRST TOKEN
    cerr << sit->getFirstToken() << endl;

    delete sit;
    }
    
  */

class stringTokenizer : public std::iterator<std::input_iterator_tag, std::string>
{
    public:
        /** Null constructor
          * 
          * This constructs a tokenizer without any content. Since you cannot
          * supply a string to be tokenized after initialization, this is
          * rather useless for the normal user, but still required internally.
          */
        stringTokenizer();
        
        /** Standard constructor
          * 
          * @param _str         the string to be analysed
          * @param _separator   a string containing all valid separators
          */
        stringTokenizer(const std::string &_str, const char * _separator =" ");

        //! Copy constructor
        stringTokenizer(const stringTokenizer & t);
        ~stringTokenizer();
        

        /** Searches for next token of the string. Returns an empty token if
          * the whole string has been broken down into tokens
          */
        stringTokenizer & operator++();

        //! Uhm, don't know about this function...
        stringTokenizer operator++(int);

        bool operator==(const stringTokenizer & rhs) const;
        bool operator!=(const stringTokenizer & rhs) const;
        
        //! Returns the last processed token
        std::string operator*() const;

        //! Returns a list of all tokens of the given string
        std::vector<std::string>* getTokens();

        //! Returns a list of starting positions of the single tokens
        std::vector<int>& getPositions();

        //! Returns the first token in the string
        std::string getFirstToken();

        //! Returns the last token in the string
        std::string getLastToken();
 
    private:
        //! Find the next token; clears several data when all tokens have been found
        void findNext(void);

        // DATA
        const char* d_separator; // Characters separators
        const std::string *d_str;  // Pointer to the string buffer to be tokenized
        std::string::size_type d_start; // Start of the token
        std::string::size_type d_end; //   End of the token

        std::vector<int> s_start_separator_pos;
};

#endif // STRING_TOKENIZER_H

// End of file
