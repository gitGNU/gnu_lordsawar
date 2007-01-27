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

#ifndef E_MAPGENDIALOG_H
#define E_MAPGENDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include "../MapGenerator.h"
#include "../MapConfDialog.h"
#include "../GameMap.h"

class E_MapGenDialog : public PG_Window
{
    public:
        E_MapGenDialog(PG_Widget* parent, PG_Rect rect, MapGenerator* gen);
        ~E_MapGenDialog();

        bool okClicked(PG_Button* btn);

    private:

        TerrainConfig* d_city;
        TerrainConfig* d_temple;
        TerrainConfig* d_ruin;
        TerrainConfig* d_grass;
        TerrainConfig* d_water;
        TerrainConfig* d_forest;
        TerrainConfig* d_swamp;
        TerrainConfig* d_hills;
        TerrainConfig* d_mountains;
        TerrainConfig* d_signposts;
        TerrainConfig* d_stones;
        PG_Button* d_b_ok;
        
        MapGenerator* d_generator;
};

#endif //E_MAPGENDIALOG_H
