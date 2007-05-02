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

#ifndef MAPCREATIONDIALOG_H
#define MAPCREATIONDIALOG_H

#include <pgwindow.h>
#include <pgprogressbar.h>
#include "GameScenario.h"

/** This is a simple dialog consiting of a progress bar and a label. It is
  * displayed when a new map is created.
  */

class MapCreationDialog : public PG_Window
{
    public:
        // CREATORS
        MapCreationDialog(PG_Widget* parent, Rectangle rect);
        ~MapCreationDialog();
    
        /** Change the label and progress.
          * 
          * With this function, you display e.g. "50% distributing cities".
          * 
          * @param p        the new value for the progress bar
          * @param text     the new value for the label
          */
        void setProgress(double p, std::string text);
        
    private:
        // DATA
        PG_Label* label;
        PG_ProgressBar* progressBar;
};

#endif // MAPCREATIONDIALOG_H
