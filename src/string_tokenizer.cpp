// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2005 Ulf Lorenz
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

#include "string_tokenizer.h"

using namespace std;

stringTokenizer::stringTokenizer() 
    :d_str(0), d_start(0), d_end(0) 
{
}

stringTokenizer::stringTokenizer(const std::string &_str, const char * _separator)
    :d_separator(_separator),d_str(&_str),d_end(0) 
{
     findNext();
}

stringTokenizer::stringTokenizer(const stringTokenizer & t) 
    :d_separator(t.d_separator), d_str(t.d_str), d_start(t.d_start),
    d_end(t.d_end), s_start_separator_pos(t.s_start_separator_pos)
{
}

stringTokenizer::~stringTokenizer() 
{
}

stringTokenizer & stringTokenizer::operator++()
{
    findNext();
    return *this;
}

stringTokenizer stringTokenizer::operator++(int)
{
    stringTokenizer temp(*this);
    ++(*this);
    return temp;
}
  
std::string stringTokenizer::operator*() const
{
    return std::string(*d_str, d_start, d_end - d_start);
}

bool stringTokenizer::operator==(const stringTokenizer & t) const
{
    return ((t.d_str == d_str) && (t.d_start == d_start) && (t.d_end == d_end));
}

bool stringTokenizer::operator!=(const stringTokenizer & t) const
{
    return !(t == *this);
}

std::vector<std::string>* stringTokenizer::getTokens() 
{
    stringTokenizer temp(*this);
    std::vector <std::string> *t=new std::vector<std::string>;
    while (temp!=stringTokenizer()) 
    {    
        t->push_back(*temp);
        temp++;
    }    
    return t;
}

std::vector<int> & stringTokenizer::getPositions() 
{
    return s_start_separator_pos;
}

std::string stringTokenizer::getFirstToken() 
{
    return *(*this);
}

std::string stringTokenizer::getLastToken() 
{
    stringTokenizer temp(*this);
    stringTokenizer temp1;
 
    while (temp!=stringTokenizer()) 
    {
        temp1=temp;
        temp++;
    }
    return *temp1;
}

void stringTokenizer::findNext(void)
{
    d_start = d_str->find_first_not_of(d_separator, d_end);
    if (d_start==1) s_start_separator_pos.push_back(0);
    if(d_start == std::string::npos)
    {
        d_start = d_end = 0;
        d_str = 0;
        return;
    }
    d_end = d_str->find_first_of(d_separator, d_start);
    s_start_separator_pos.push_back(d_end);
}
