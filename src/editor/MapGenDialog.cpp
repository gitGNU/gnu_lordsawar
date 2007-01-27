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

#include "MapGenDialog.h"

E_MapGenDialog::E_MapGenDialog(PG_Widget* parent, PG_Rect rect, MapGenerator* gen)
    :PG_Window(parent, rect, _("Set map properties"), PG_Window::MODAL),
    d_generator(gen)
{
    int w, h, noSignposts, noStones;
    PG_Rect r(50, 50, my_width-100, 30);
    d_city = new TerrainConfig(this, r, _("Cities"), 0, 20, 10);

    r.y += 35;
    d_temple = new TerrainConfig(this, r, _("Temples"), 0, 20, 10);
    r.y += 35;
    d_ruin = new TerrainConfig(this, r, _("Ruins"), 0, 20, 10);
    r.y += 35;
    d_grass = new TerrainConfig(this, r, _("Grass"), 0, 100, 78);
    r.y += 35;
    d_water = new TerrainConfig(this, r, _("Water"), 0, 100, 7);
    r.y += 35;
    d_forest = new TerrainConfig(this, r, _("Forest"), 0, 100, 3);
    r.y += 35;
    d_swamp = new TerrainConfig(this, r, _("Swamp"), 0, 100, 2);
    r.y += 35;
    d_hills = new TerrainConfig(this, r, _("Hills"), 0, 100, 5);
    r.y += 35;
    d_mountains = new TerrainConfig(this, r, _("Mountains"), 0, 100, 5);
    r.y += 35;
    w = GameMap::getInstance()->getWidth();
    h = GameMap::getInstance()->getHeight();
    noSignposts = static_cast<int>(w * h * (d_grass->getValue() / 100.0) * 0.003);
    d_generator->setNoSignposts(noSignposts);
    d_signposts = new TerrainConfig(this, r, _("Signposts"), 0, 100, noSignposts);
    r.y += 35;
    noStones = static_cast<int>(w * h * (d_grass->getValue() / 100.0) * 0.0022);
    d_stones = new TerrainConfig(this, r, _("Stones"), 0, 100, noStones);

    d_b_ok = new PG_Button(this, PG_Rect(my_width/2 - 40, my_height - 40, 80, 30),
                           _("OK"));
    d_b_ok->sigClick.connect(slot(*this, &E_MapGenDialog::okClicked));
}

E_MapGenDialog::~E_MapGenDialog()
{
    delete d_city;
    delete d_temple;
    delete d_ruin;
    delete d_grass;
    delete d_water;
    delete d_forest;
    delete d_swamp;
    delete d_hills;
    delete d_mountains;
    delete d_signposts;
    delete d_stones;
}

bool E_MapGenDialog::okClicked(PG_Button* btn)
{
    // fill the generator with data
    d_generator->setNoCities(d_city->getValue());
    d_generator->setNoRuins(d_ruin->getValue());
    d_generator->setNoTemples(d_temple->getValue());
    d_generator->setNoSignposts(d_signposts->getValue());
    d_generator->setNoStones(d_stones->getValue());
    
    // the percentages code is stolen from CreateScenario, which we don't use here
    int grass, water, forest, swamp, hills, mountains;

    grass = d_grass->getValue();
    water = d_water->getValue();
    forest = d_forest->getValue();
    swamp = d_swamp->getValue();
    hills = d_hills->getValue();
    mountains = d_mountains->getValue();

    // if sum > 100 (percent), divide everything by a factor, the numeric error
    // is said to be grass
    int sum = grass+water+forest+swamp+hills+mountains;

    if (sum > 100)
    {
        double factor = 100/static_cast<double>(sum);
        water = static_cast<int>(water/factor);
        forest = static_cast<int>(forest/factor);
        swamp = static_cast<int>(swamp/factor);
        hills = static_cast<int>(hills/factor);
        mountains = static_cast<int>(mountains/factor);
    }
    
    d_generator->setPercentages(water, forest, swamp, hills, mountains);
    
    // and leave the dialog
    QuitModal();
    return true;
}
