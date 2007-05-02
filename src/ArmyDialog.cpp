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

#include <stdio.h>
#include <sstream>
#include <pgapplication.h>
#include <pgrichedit.h>

#include "ArmyDialog.h"
#include "ItemDialog.h"
#include "GameMap.h"
#include "TileSet.h"
#include "maptile.h"
#include "defs.h"
#include "hero.h"
#include "File.h"

ArmyDialog::ArmyDialog(Army* army, PG_Widget* parent, Rectangle rect)
    :PG_ThemeWidget(parent, rect, true), d_army(army)
{
    char buffer[61];
    buffer[60] = '\0';

    // I assume a size of 600*500 pixels here
    // The screen is divided in 4 sections. The upper left contains a big
    // image of the army or the pixmap if no big image exists. The lower left
    // contains a description of the unit, the upper right pictograms of all unit
    // stats and the lower right the rest, namely level, experience etc.
    
    // First, create the single sections. Start with the upper left...
    // The label is a poor substitute for setting the background of an inlay
    // widget, however, the latter one somehow doesn't work (image is transparent)
    Rectangle r(0, 30, 350, 350);
    PG_Label* ul = new PG_Label(this, r, "");
    if (army->getPortrait())
        ul->SetIcon(army->getPortrait());
    else
        ul->SetIcon(army->getPixmap());

    // above the image, place a button to handle items.
    r.SetRect(250, 330, 80, 25);
    d_b_item = new PG_Button(this, r, _("Items"),3);
    d_b_item->sigClick.connect(slot(*this, &ArmyDialog::b_itemClicked));
    if (!d_army->isHero())
        d_b_item->Hide();

    // upper right; create the labels for the name and the number of shots,
    // the rest of icon drawing is done in fillData
    r.SetRect(380, 40, 200, 20);
    new PG_Label(this, r, _(d_army->getName().c_str()));

    r.SetRect(380, 70, 200, 20);
    snprintf(buffer, 60, ngettext("%i shot", "%i shots", d_army->getStat(Army::SHOTS)),
                d_army->getStat(Army::SHOTS));
    d_l_shots = new PG_Label(this, r, buffer);

    loadIcons();
    
    // lower left
    r.SetRect(5, 385, 340, 110);
//    PG_MultiLineEdit* text = new PG_MultiLineEdit(this, r);
//    text->SetEditable(false);
    PG_RichEdit* text = new PG_RichEdit(this, r);
    text->SetText(assembleText().c_str());
    
    // lower right contains XP, level, upkeep and ok button
    r.SetRect( 370, 420, 100, 20);
    snprintf(buffer, 60, _("xp: %.2f"), d_army->getXP());
    new PG_Label(this, r, buffer);
    
    r.SetRect(370, 450, 100, 20);
    snprintf(buffer, 60, _("level: %i"), d_army->getLevel());
    new PG_Label(this, r, buffer);
    
    r.SetRect(470, 420, 100, 20);
    snprintf(buffer, 60, _("Upkeep: %i"), d_army->getUpkeep());
    new PG_Label(this, r, buffer);
        
    r.SetRect(500, 465, 80, 25);
    d_b_ok = new PG_Button(this, r, _("OK"), 1);
    d_b_ok->sigClick.connect(slot(*this, &ArmyDialog::b_ok_clicked));

    Redraw();
}

ArmyDialog::~ArmyDialog()
{
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 4; j++)
            SDL_FreeSurface(d_icon[i][j]);
}

bool ArmyDialog::b_ok_clicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool ArmyDialog::b_itemClicked(PG_Button* btn)
{
    int xpos = PG_Application::GetScreenWidth() / 2 - 230;
    int ypos = PG_Application::GetScreenHeight()/2 - 200;

    ItemDialog dialog(static_cast<Hero*>(d_army),
                      0, Rectangle(xpos, ypos, 500, 420));

    dialog.Show();
    dialog.RunModal();
    dialog.Hide();

    Redraw();

    return true;
}

bool ArmyDialog::eventKeyDown(const SDL_KeyboardEvent* key)
{
    switch(key->keysym.sym)
    {
        case SDLK_RETURN:
            b_ok_clicked(0);
            break;
        case SDLK_i:
            b_itemClicked(0);
            break;
        default:
            break;
    }
    return true;
}

void ArmyDialog::eventDraw(SDL_Surface* surface, const Rectangle& r)
{
    PG_ThemeWidget::eventDraw(surface, r);
    
    placeIcons(surface, Rectangle(370, 105, 210, 40), Army::STRENGTH);
    placeIcons(surface, Rectangle(370, 155, 210, 40), Army::RANGED);
    placeIcons(surface, Rectangle(370, 205, 210, 40), Army::DEFENSE);
    placeIcons(surface, Rectangle(370, 255, 210, 40), Army::HP);
    placeIcons(surface, Rectangle(370, 305, 210, 40), Army::VITALITY);
    placeIcons(surface, Rectangle(370, 355, 210, 40), Army::MOVES);
}

void ArmyDialog::placeIcons(SDL_Surface* s, const Rectangle r, Army::Stat stat)
{
    // set some basic variables depending on the stat selected
    // namely the stats and the index of the appropriate pic
    int normal, max, cur, index;

    normal = d_army->getStat(stat, false);
    max = d_army->getStat(stat);
    cur = max;

    switch(stat)
    {
        case Army::STRENGTH:
            index = 0;
            break;
        case Army::RANGED:
            index = 1;
            break;
        case Army::DEFENSE:
            index = 2;
            break;
        case Army::HP:
            index = 3;
            cur = d_army->getHP();
            break;
        case Army::VITALITY:
            index = 4;
            break;
        case Army::MOVES:
            index = 5;
            cur = d_army->getMoves();
            break;
        default:
            index = 0;
    }

    // not much to draw
    if (cur == 0 && max == 0 && normal == 0)
        return;

    // second point: we need other values; how many normal icons, how many
    // "reduced" icons, how many increased and lost or just increased icons to draw.
    // for simplicity, this is mapped to an array with the same meaning as the
    // graphical icons.
    int type[4];

    for (int i = 0; i < 4; i++)
        type[i] = 0;

    // if the modified stat is reduced there is no need for the max variable
    // (saves some other if clauses)
    if (max < normal)
        max = normal;
    
    // "normal" images
    type[0] = normal;
    if (cur < normal)
        type[0] = cur;

    // "increased stat" images
    if (max > normal && cur > normal)
        type[1] = cur - normal;

    // "lost stat" images
    if (cur < normal)
        type[2] = normal - cur;

    // "lost increased stat" images
    if (max > normal && cur < max)
    {
        type[3] = max - cur;
        if (type[3] > max - normal)
            type[3] = max - normal;
    }

    // next decision to make: We group the icons in blocks of 5. Now do we have enough
    // space to place them next to each other or do we need to "stack" them, i.e. overlap
    // half of the icon by the next? This has to be calculated horizontally and vertically.
    // rows[0] is the number we can place horizontally without stacking, rows[1] with
    // stacking, the same with cols.
    int rows[2], cols[2];
    
    // size of one 5 block, then number of blocks
    cols[0] = d_icon[index][0]->w * 5 + 5; //small offset
    cols[0] = r.w / cols[0];

    cols[1] = d_icon[index][0]->w * 3 + 5;
    cols[1] = r.w / cols[1];

    // the icons can also be quenched vertically
    rows[0] = r.h / d_icon[index][0]->h;
    rows[1] = (2*r.h) /d_icon[index][0]->h - 1;
    
    int stackhoriz = 0;
    int stackvert = 0;

    if (cols[0] * rows[0] * 5 < max)
    {
        // first, stack the icons horizontally
        stackhoriz = 1;
        if (cols[1] * rows[0] * 5 < max)
        {
            stackvert = 1;
            // TODO: do something, anything...
            if (cols[1] * rows[1] * 5 < max)
                std::cerr <<d_army->getName() <<": Unable to represent unit's stats with icons!\n";
        }
    }

    // Now draw all the icons (puh)
    Rectangle target;
    target.x = r.x;
    target.y = r.y;
    target.w = d_icon[index][0]->w;
    target.h = d_icon[index][0]->h;

    // draw the normal icons first
    int count = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < type[i]; j++, count++)
        {
            if (count%5 == 0 && count != 0)
            {
                // add an offset after a 5 block
                target.x += d_icon[index][i]->w + 5;

                // check if we need to start a new line
                if (count/5 == cols[stackhoriz])
                {
                    target.x = r.x;
                    if (stackvert)
                        target.y += d_icon[index][i]->h/2;
                    else
                        target.y += d_icon[index][i]->h;
                }
            }
            else if (count != 0)
            {
                if (stackhoriz)
                    target.x += d_icon[index][i]->w/2;
                else
                    target.x += d_icon[index][i]->w;
            }

            SDL_BlitSurface(d_icon[index][i], 0, s, &target);
        }
}

void ArmyDialog::loadIcons()
{
    SDL_Surface* stats = File::getMiscPicture("stats.png");
    // copy the alpha channel
    SDL_SetAlpha(stats, 0, 0);

    // now mask out the single images; assuming 4 icons width
    int size = stats->w / 4;

    // should be 6 quadratic icons
    if (stats->h != size * 6)
    {
        std::cerr <<"ArmyDialog: Icons have wrong format\n" <<std::endl <<std::flush;
        return;
    }

    // This should preserve the maximum possible quality.
    SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 32,
                            0xFF000000, 0xFF0000, 0xFF00, 0xFF);
    Rectangle r;
            
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 4; j++)
        {
            r.SetRect(j*size, i*size, size, size);
            SDL_BlitSurface(stats, &r, tmp, 0);
            d_icon[i][j] = SDL_DisplayFormatAlpha(tmp);
        }

    SDL_FreeSurface(tmp);
}

std::string ArmyDialog::assembleText()
{
    std::stringstream ss;
    
    ss <<d_army->getDescription();

    // first the movement boni are added to the description
    Uint32 bonus = d_army->getStat(Army::MOVE_BONUS);
    TileSet* ts = GameMap::getInstance()->getTileSet();
    bool first = true;
    
    if (bonus)
        ss << _("\n\nUnit is especially fast on ");
    
    for (unsigned int i = 0; i < ts->size(); i++)
        if (bonus & (*ts)[i]->getType())
        {
            if (!first)
                ss <<", ";
            ss <<_((*ts)[i]->getName().c_str());
            first = false;
        }

    ss <<"\n\n";

    // now the army boni
    bonus = d_army->getStat(Army::ARMY_BONUS);
    first = true;

    if (bonus)
        ss <<_("Abilities: ");
    
    if (bonus & Army::SHIP)
    {
        ss <<(first?"":", ") <<_("ship");
        first = false;
    }
    if (bonus & Army::LEADER)
    {
        ss <<(first?"":", ") <<_("leadership");
        first = false;
    }
    if (bonus & Army::CAVALRY)
    {
        ss <<(first?"":", ") <<_("mounted");
        first = false;
    }
    if (bonus & Army::ANTICAVALRY)
    {
        ss <<(first?"":", ") <<_("effective vs. mounted troops");
        first = false;
    }
    if (bonus & Army::REGENERATE)
    {
        ss <<(first?"":", ") <<_("regeneration");
        first = false;
    }
    if (bonus & Army::CRITICAL)
    {
        ss <<(first?"":", ") <<_("can score critical hits");
        first = false;
    }

    return ss.str();
}
