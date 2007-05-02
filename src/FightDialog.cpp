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

#include <pgapplication.h>
#include "FightDialog.h"
#include "stacklist.h"
#include "stack.h"
#include "army.h"
#include "player.h"
#include "GameMap.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)


// Helper class to keep track of the current status of the single units
class DialogItem
{
    public:
        Army* army;         // the army we refer to
        int hp;             // the current number of hitpoints (we can't use
                            // Army::getHP(), since it has already been modified
        Rectangle pos;        // Location of the image within the dialog
};
        


FightDialog::FightDialog(Stack* attacker, Stack* defender, bool duel)
    :PG_ThemeWidget(0, Rectangle(), true)
{
    // init the fight
    d_fight = new Fight(attacker, defender, duel);
    
    // first, take all participating stacks and populate d_(attackers|defenders)
    std::list<Stack*> lst = d_fight->getAttackers();
    std::list<Stack*>::const_iterator it;
    
    for (it = lst.begin(); it != lst.end(); it++)
        for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
        {
            DialogItem i;
            i.army = (*sit);
            i.hp = i.army->getHP();
            d_attackers.push_back(i);
        }
    
    lst = d_fight->getDefenders();
    for (it = lst.begin(); it != lst.end(); it++)
        for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
        {
            DialogItem i;
            i.army = (*sit);
            i.hp = i.army->getHP();
            d_defenders.push_back(i);
        }

    // Beside other things, initScreen will care for setting up the correct
    // size of the fight dialog, placing all items correctly etc.
    initScreen();
}

FightDialog::FightDialog(std::list<Stack*> attackers, std::list<Stack*> defenders,
                         const std::list<FightItem>& actions)
    :PG_ThemeWidget(0, Rectangle()), d_actions(actions), d_fight(0)
{
    // Basically the same as the other constructor above
    std::list<Stack*>::const_iterator it;
    
    for (it = attackers.begin(); it != attackers.end(); it++)
        for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
        {
            DialogItem i;
            i.army = (*sit);
            i.hp = i.army->getHP();
            d_attackers.push_back(i);
        }
    
    for (it = defenders.begin(); it != defenders.end(); it++)
        for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
        {
            DialogItem i;
            i.army = (*sit);
            i.hp = i.army->getHP();
            d_defenders.push_back(i);
        }

    // Beside other things, initScreen will care for setting up the correct
    // size of the fight dialog, placing all items correctly etc.
    initScreen();
}

FightDialog::~FightDialog()
{
    if (d_fight)
        delete d_fight;
}

void FightDialog::battle()
{
    // first, do the fight if neccessary
    if (d_fight)
    {
        d_fight->battle();
        d_actions = d_fight->getCourseOfEvents();
    }


    // Now display the fight
    int round = 0;
    std::list<FightItem>::const_iterator it;

    for (it = d_actions.begin(); it != d_actions.end(); it++)
    {
      DialogItem* item = findArmy((*it).id);
      if (!item)
          //???? Should not happen, but be nice here
          continue;
      
      item->hp -= (*it).damage;
      if (item->hp < 0)
          item->hp = 0;

      // redraw everything once per turn
      if ((*it).turn > round)
      {
          round++;
          
          char buf[11]; buf[10] = '\0';
          snprintf(buf, 10, "%i", round);
          d_l_turn->SetText(buf);

          Redraw();
          SDL_Delay(ROUND_DELAY);
      }
    }
}

void FightDialog::initScreen()
{
    // The code here is rather complex, so some comments may be useful. In
    // the worst case scenario, up to 9 stacks can participate in a fight
    // with 8 armies each, which makes a total of 72 armies. If we assume that
    // a single army occupies around 5000 pixels (50x50=2500 for the image
    // + hitpoints bar + border), we get 360 kPixels or a minimum size of ca.
    // 700x500 pixels for the dialog. However, for smaller fights this is much
    // to big. So, as a result, we have to determine the size of the dialog
    // depending on the number of armies that participate.
    //
    // Furthermore, for the optics, we wish to separate close combat units from
    // armies with ranged attack. This won't be changed during the battle (this
    // would be _really_ cumbersome), but should be done in the beginning. The
    // code assumes that the fight class puts all units with no ranged attack 
    // (or rather no shots) in the front line, so we do the same here.
    std::list<DialogItem>::iterator it;
    
    // First, to get an overview, count the number of melee/ranged attack units
    int att_close = 0;
    int att_ranged = 0;
    for (it = d_attackers.begin(); it != d_attackers.end(); it++)
    {
        if ((*it).army->getStat(Army::SHOTS) == 0)
            att_close++;
        else
            att_ranged++;
    }

    int def_close = 0;
    int def_ranged = 0;
    for (it = d_defenders.begin(); it != d_defenders.end(); it++)
    {
        if ((*it).army->getStat(Army::SHOTS) == 0)
            def_close++;
        else
            def_ranged++;
    }
    
    // The layout of the dialog shall be something like this:
    //
    //  Player A                <round no.>                 Player B
    //
    //  ranged          close         |     close           ranged   
    //  attackers       attackers     |     defenders       defenders
    //  
    //             ^                  ^                 ^
    //         spacing              line            spacing
    //
    // Another condition is that the game must be playable in 800x600 resolution.
    // Assume 50 pixels height for the header and try to make the dialog as
    // square-like as possible. The calculation for this makes up the following
    // section.

    // the spacing between all the objects; for screens larger than 800x600,
    // we don't have to be too careful here, also if there are only few combatants
    int spacing = 20;
    if (PG_Application::GetScreenWidth() > 1000
            || (att_close + def_close + att_ranged + def_ranged < 10))
        spacing = 40;
    
    // minimum height: top spacing + header more spacing + bottom spacing
    int height = spacing + 20 + spacing + spacing;
    
    // minium width: left/right border spacing + line spacing
    int width = spacing*3;

    // if there are ranged/close combat units, additional spacing is required
    if (att_close > 0 && att_ranged > 0)
        width += spacing;
    if (def_close > 0 && def_ranged > 0)
        width += spacing;

    // in the following part, we assume as space for one single army:
    // width = 60 (50 for the army + 2*5 left/right spacing)
    // height = 70 (50 for army + 2*5 top/bottom spacing + 10 hitpoint bar)

    // maximum number of stacks in a column. Again for large screens, we are more
    // generous.
    int max_items = 6;
    if (PG_Application::GetScreenHeight() > 600)
        max_items = 8;

    std::vector<Rectangle> dim(max_items);
    for (int i = 0; i < max_items; i++)
    {
        dim[i].w = width;
        dim[i].h = height + (i+1) * 70;
        
        // find out the number of columns needed for the ranged attackers
        // and add it to the width for this case.
        dim[i].w += 60 * (att_ranged / (i+1));

        // if there are attackers left, we need to start a new (not full) column
        if (att_ranged % (i+1) > 0)
            dim[i].w += 60;
        
        // now do the same for all other cases
        dim[i].w += 60 * (att_close / (i+1));
        if (att_close % (i+1) > 0)
            dim[i].w += 60;

        dim[i].w += 60 * (def_close / (i+1));
        if (def_close % (i+1) > 0)
            dim[i].w += 60;

        dim[i].w += 60 * (def_ranged / (i+1));
        if (def_ranged % (i+1) > 0)
            dim[i].w += 60;
    }

    // Now check if there are any valid dimensions
    bool valid = false;
    for (int i = 0; i < max_items; i++)
        if (dim[i].w <= PG_Application::GetScreenWidth() - 50)
            valid = true;

    if (!valid)
    {
        // Bad news: We have a small screen and a huge fight. We try to cope
        // by reducing the spacing to 0.
        spacing = 0;
        max_items = 7;
        width = 0;
        height = 20;

        for (int i = 0; i < max_items; i++)
        {
            dim[i].w = width;
            dim[i].h = height + (i+1) * 70;
            
            // find out the number of columns needed for the ranged attackers
            // and add it to the width for this case.
            dim[i].w += 60 * (att_ranged / (i+1));

            // if there are attackers left, we need to start a new (not full) column
            if (att_ranged % (i+1) > 0)
                dim[i].w += 60;
            
            // now do the same for all other cases
            dim[i].w += 60 * (att_close / (i+1));
            if (att_close % (i+1) > 0)
                dim[i].w += 60;

            dim[i].w += 60 * (def_close / (i+1));
            if (def_close % (i+1) > 0)
                dim[i].w += 60;

            dim[i].w += 60 * (def_ranged / (i+1));
            if (def_ranged % (i+1) > 0)
                dim[i].w += 60;
        }

        // the worst case here is: 72 units participate in 10 full and two
        // non-full columns, makes a width of 12*60 = 720. Since the minimum
        // resolution is 800x600, this should always work.
    }


    // Now we have a couple of possible dimensions for ou dialog. The big
    // question is: Which to take?
    // Solution: We take the first one that is wider than it is high
    // ( == the most square-like). If all dimensions fulfill this, then we
    // take the one with the smallest width, i.e. maximum number of items
    // per column.
    
    // After this loop, max_items is set to the number of items per column
    // with optimal dimension.
    for (int i = max_items-1; i >=  0; i--)
        if (dim[i].w > dim[i].h)
        {
            max_items = i+1;
            break;
        }


    // Now set the Dimension we found out
    Rectangle r = dim[max_items-1];
    r.x = (PG_Application::GetScreenWidth() - r.w)/2;
    r.y = (PG_Application::GetScreenHeight() -r.h)/2;
    MoveWidget(r, false);


    // Puh, that was the first and more tedious task. Now the second part:
    // Place all the images etc. Since we need some more data, we start with the
    // placement of the armies. And here we start with the ranged attackers, then
    // come the cose attackers and so on.

    int x = spacing;             // x coordinate to keep track where we are
    int y = 2 * spacing + 20;    // y coordinate; the same
    int row = 0;

    // first, place the ranged attackers
    for (it = d_attackers.begin(); it != d_attackers.end(); it++)
       if ((*it).army->getStat(Army::SHOTS) > 0)
       {
           (*it).pos.x = x + 5;     // remember: border offset
           (*it).pos.y = y + 5;

           row++;
           if (row == max_items)
           {
               row = 0;
               x += 60;
               y = 2 * spacing + 20;
           }
           else
               y += 70;
       }

    if (row != 0)
       x += 60;

    // if neccessary, add spacing
    if (att_ranged > 0 && att_close > 0)
       x += spacing;

    y = 2 * spacing + 20;
    row = 0;

    // place close combat attackers
    for (it = d_attackers.begin(); it != d_attackers.end(); it++)
       if ((*it).army->getStat(Army::SHOTS) == 0)
       {
           (*it).pos.x = x + 5;
           (*it).pos.y = y + 5;

           row++;
           if (row == max_items)
           {
               row = 0;
               x += 60;
               y = 2 * spacing + 20;
           }
           else
               y += 70;
       }

    if (row != 0)
       x += 60;

    // add spacing for the middle line and save the position of the line
    x += spacing;
    y = 2 * spacing + 20;
    row = 0;

    d_line.x = x - (spacing/2);
    d_line.y = y;
    d_line.w = 0;
    d_line.h = max_items * 70;


    // place close combat defenders
    for (it = d_defenders.begin(); it != d_defenders.end(); it++)
       if ((*it).army->getStat(Army::SHOTS) == 0)
       {
           (*it).pos.x = x + 5;
           (*it).pos.y = y + 5;

           row++;
           if (row == max_items)
           {
               row = 0;
               x += 60;
               y = 2 * spacing + 20;
           }
           else
               y += 70;
       }

    if (row != 0)
       x += 60;

    // add spacing; if there are no ranged defenders, we don't do anything here
    // anyway.
    x += spacing;
    y = 2* spacing + 20;
    row = 0;

    for (it = d_defenders.begin(); it != d_defenders.end(); it++)
        if ((*it).army->getStat(Army::SHOTS) > 0)
        {
            (*it).pos.x = x + 5;
            (*it).pos.y = y + 5;
            
            row++;
            if (row == max_items)
            {
                row = 0;
                x += 60;
                y = 2 * spacing + 20;
            }
            else
                y += 70;
        }

    
    // Now the last thing to do is to place the labels; the attacker name is
    // put in the middle of the attacker side, the defender name in the middle
    // of the defender's side and the round number over the line.
    // I assume that there is always some minimum size so that the labels fit.

    // attacker name; place it in the middle of the attacking stacks.
    r.w = 100;
    r.h = 20;
    r.x = (d_line.x - spacing/2)/2 - 100/2;
    r.y = spacing;
    Player* p = (*d_attackers.begin()).army->getPlayer();

    PG_Label* l = new PG_Label(this, r, p->getName().c_str());
    l->SetFontColor(PG_Color(p->getColor()));


    // defender name; the same. It may happen that the defender's player
    // is 0 (ruin search), use neutral then
    r.x = (my_width + (d_line.x + spacing/2))/2 - 100/2;
    p = (*d_defenders.begin()).army->getPlayer();
    
    if (p)
    {
        l = new PG_Label(this, r, p->getName().c_str());
        l->SetFontColor(PG_Color(p->getColor()));
    }
    else
    {
        l = new PG_Label(this, r, "Neutral");
        l->SetFontColor(PG_Color(150, 150, 150));
    }


    // place the counter for the round numbers
    r.w = 40;
    r.x = d_line.x - 20;
    
    d_l_turn = new PG_Label(this, r, "0");

    // FINISHED!
}

void FightDialog::eventDraw(SDL_Surface* surface, const Rectangle& r)
{
    PG_ThemeWidget::eventDraw(surface, r);

    // Two items have to be drawn:

    // 1. The line that separates attackers and defenders
    DrawVLine(d_line.x, d_line.y, d_line.h, PG_Color(0,0,0));

    // 2. All army images
    std::list<DialogItem>::const_iterator it;
    for (it = d_attackers.begin(); it != d_attackers.end(); it++)
        drawArmy(surface, *it);

    for (it = d_defenders.begin(); it != d_defenders.end(); it++)
        drawArmy(surface, *it);
}

void FightDialog::drawArmy(SDL_Surface* surface, const DialogItem& unit)
{
    /* A single dialog item has dimensions (without border, which doesn't
     * interest us here) of 50x60 pixels and looks like this:
     *
     * ------------     -\
     * |          |      |
     * |          |       \    50 pixels for army image
     * |          |       /
     * |          |      |
     * ------------     -/
     *                        3 pixles spacing
     * ------------             
     * |    |     |           7 pixels for hitpoints bar
     * ------------
     */

    // currently, the army images are (yet) 40x40 pixels, so we leave some
    // additional five pixels of offset.
    SDL_Rect r;
    r.x = unit.pos.x + 5;
    r.y = unit.pos.y + 5;
    r.w = r.h = 40;
    SDL_BlitSurface(unit.army->getPixmap(), 0, surface, &r);

    // now draw the hitpoints bar, first the green part for the remaining
    // hitpoints, then the red one for the lost hp.
    r.x = unit.pos.x;
    r.y = unit.pos.y + 56;
    r.w = (unit.hp * 50) / unit.army->getStat(Army::HP);
    r.h = 6;
    DrawLine(r.x, r.y, r.x + r.w, r.y, PG_Color(0, 250, 0), r.h);

    r.x += r.w;
    r.w = 50 - r.w;
    DrawLine(r.x, r.y, r.x + r.w, r.y, PG_Color(250,0,0), r.h);
}

DialogItem* FightDialog::findArmy(Uint32 id)
{
    std::list<DialogItem>::iterator it;

    for (it = d_attackers.begin(); it != d_attackers.end(); it++)
        if ((*it).army->getId() == id)
            return &(*it);

    for (it = d_defenders.begin(); it != d_defenders.end(); it++)
        if ((*it).army->getId() == id)
            return &(*it);

    return 0;
}

// End of file
