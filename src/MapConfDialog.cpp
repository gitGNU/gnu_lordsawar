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

#include "MapConfDialog.h"
#include <pglabel.h>
#include <pgslider.h>
#include <pgbutton.h>

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

TerrainConfig::TerrainConfig(PG_Widget* parent, Rectangle rect, char* name,
        int min, int max, int pos)
        :d_value(pos)
{
    char buffer[10];
    sprintf(buffer, "%i", pos);

    d_l_name = new PG_Label(parent, Rectangle(rect.x, rect.y, 80, 20), name);
    d_l_name->SetFontColor (PG_Color (0, 0, 0));
    d_slider = new PG_Slider(parent, Rectangle(rect.x + 90, rect.y, rect.w - 160, 20),PG_ScrollBar::HORIZONTAL,1);
    d_slider->SetFontColor (PG_Color (0, 0, 0));
    d_l_value = new PG_Label(parent, Rectangle(rect.x + rect.w - 60, rect.y, 30, 20), buffer);
    d_l_value->SetFontColor (PG_Color (0, 0, 0));
    d_button = new PG_Button(parent, Rectangle(rect.x + rect.w - 20, rect.y, 20, 20), "?",2);
    d_button->SetFontColor (PG_Color (0, 0, 0));

    d_slider->SetRange(min, max);
    d_slider->SetPosition(pos);
    d_slider->sigSlide.connect(slot(*this, &TerrainConfig::slide));
}

TerrainConfig::~TerrainConfig()
{
    delete d_l_name;
    delete d_slider;
    delete d_l_value;
    delete d_button;
}

bool TerrainConfig::slide(PG_ScrollBar* slider, long pos)
{
    char buffer[10];
    d_value = pos;
    sprintf(buffer, "%li", pos);
    d_l_value->SetText(buffer);
    return true;
}

void TerrainConfig::hide()
{
    d_l_name->Hide();
    d_slider->Hide();
    d_l_value->Hide();
    d_button->Hide();
}

void TerrainConfig::show()
{
    d_l_name->Show();
    d_slider->Show();
    d_l_value->Show();
    d_button->Show();
}

void TerrainConfig::setValue(int value)
{
    d_value = value;
    d_slider->SetPosition(value);

    char buffer[10];
    sprintf(buffer, "%i", value);
    d_l_value->SetText(buffer);
}
// End of file
