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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include "MapCreationDialog.h"
#include "defs.h"
#include <pglabel.h>

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

MapCreationDialog::MapCreationDialog(PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("Creating Map"), PG_Window::MODAL)
{
    label = new PG_Label(this, PG_Rect(0, 35, rect.w, 20),
                        _("Generating random map"));

    label->SetAlignment(PG_Label::CENTER);// middle
    
    progressBar = new PG_ProgressBar(this, PG_Rect(10, 65, rect.w - 20, 25));
}

MapCreationDialog::~MapCreationDialog()
{
    delete label;
    delete progressBar;
}

void MapCreationDialog::setProgress(double d, std::string text)
{
    label->SetText(text.c_str());
    progressBar->SetProgress(d);
}

// End of file
