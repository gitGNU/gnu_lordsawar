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

#include "stackreport.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "army.h"
#include <stdio.h>
#include <pgbutton.h>
#include <pglabel.h>

//first comes the implementation of StackItem. A StackItem is a bar of 420x80
//pixels which displays infos about a single stack.
class StackItem : public PG_Widget
{
    public:
        StackItem(PG_Widget* parent, Rectangle rect);
        ~StackItem();

        void readData(Stack* stack);

        sigc::signal<void, Stack*> sclicked;

    private:
        bool eventMouseButtonDown(const SDL_MouseButtonEvent * event);

        PG_Button* d_b_army[8];
        PG_Label* d_l_owner;
        PG_Label* d_l_xpos;
        PG_Label* d_l_ypos;
        Stack* d_stack;
};

StackItem::StackItem(PG_Widget* parent, Rectangle rect)
    :PG_Widget(parent, rect)
{
    d_l_owner = new PG_Label(this, Rectangle(10, 10, 150, 15), "");
    d_l_xpos = new PG_Label(this, Rectangle(200, 10, 45, 15), "");
    d_l_ypos = new PG_Label(this, Rectangle(250, 10, 45, 15), "");

    for (int i = 0; i < 8; i++)
    {
        d_b_army[i] = new PG_Button(this, Rectangle(10 + i*50, 30, 40, 40), "",i);
        d_b_army[i]->EnableReceiver(false);
        d_b_army[i]->Hide();
    }
}

StackItem::~StackItem()
{
}

void StackItem::readData(Stack* stack)
{
    d_stack = stack;

    if (!d_stack)
    {
        d_l_owner->SetText("");
        d_l_xpos->SetText("");
        d_l_ypos->SetText("");
        for (int i = 0; i < 8; i++)
            d_b_army[i]->Hide();
        return;
    }

    SDL_Color color = stack->getPlayer()->getColor();
    d_l_owner->SetFontColor(PG_Color(color.r, color.g, color.b));
    d_l_owner->SetText(stack->getPlayer()->getName().c_str());

    d_l_xpos->SetTextFormat("x: %i", stack->getPos().x);
    d_l_ypos->SetTextFormat("y: %i", stack->getPos().y);

    //display all armies in the stack
    Stack::iterator it = d_stack->begin();
    for (int i = 0; i < 8; i++)
    {
        d_b_army[i]->SetIcon((SDL_Surface*)0, 0, 0);
        if (it != d_stack->end())
        {
            d_b_army[i]->SetIcon((*it)->getPixmap(), 0, 0);
            d_b_army[i]->Show();
            it++;
            continue;
        }
        //it == stack->end()
        d_b_army[i]->Hide();
    }
}

bool StackItem::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{
    if (!IsVisible())
        return true;

    if (d_stack)
        sclicked.emit(d_stack);
    return true;
}


//now comes the StackReport class
StackReport::StackReport(PG_Widget* parent, Rectangle rect)
    :PG_Window(parent, rect, _("stack report"), PG_Window::MODAL), d_index(0)
{
    //I assume 500x400 pixels here as well
    d_b_ok = new PG_Button(this, Rectangle(430, 365, 90, 25), _("OK"),2);
    d_b_ok->sigClick.connect(slot(*this, &StackReport::b_okClicked));

    d_b_up = new PG_Button(this, Rectangle(430, 150, 90, 25), _("Prev."),0);
    d_b_up->sigClick.connect(slot(*this, &StackReport::b_upClicked));

    d_b_down = new PG_Button(this, Rectangle(430, 250, 90, 25), _("Next"),1);
    d_b_down->sigClick.connect(slot(*this, &StackReport::b_downClicked));

    d_l_number = new PG_Label(this, Rectangle(430, 35, 60, 20), "");

    d_items[0] = new StackItem(this, Rectangle(10, 30, 420, 80));
    d_items[1] = new StackItem(this, Rectangle(10, 120, 420, 80));
    d_items[2] = new StackItem(this, Rectangle(10, 210, 420, 80));
    d_items[3] = new StackItem(this, Rectangle(10, 300, 420, 80));

    for (int i = 0; i < 4; i++)
        d_items[i]->sclicked.connect(sigc::slot(*this, &StackReport::stackSelected));

    //now get all stacks sorted by player with the activeplayer first and
    //the stacks of the neutral player last
    Playerlist::iterator pit;
    Stacklist::iterator sit;

    d_stacklist.resize(Stacklist::getNoOfStacks());
    //first, stacks of the active player
    Stacklist* mylist = Playerlist::getActiveplayer()->getStacklist();
    for (sit = mylist->begin(); sit != mylist->end(); sit++)
    {
        d_stacklist[d_index] = (*sit);
        d_index++;
    }

    //now the other non-neutral players
    for (pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        if (((*pit) == Playerlist::getActiveplayer())
            || ((*pit) == Playerlist::getInstance()->getNeutral()))
        {
            continue;
        }

        mylist = (*pit)->getStacklist();
        for (sit = mylist->begin(); sit != mylist->end(); sit++)
        {
            d_stacklist[d_index] = (*sit);
            d_index++;
        }
    }

    //now the neutral player
    mylist = Playerlist::getInstance()->getNeutral()->getStacklist();
    for (sit = mylist->begin(); sit != mylist->end(); sit++)
    {
        d_stacklist[d_index] = (*sit);
        d_index++;
    }

    d_index = 0;

    fillStackItems();
}

StackReport::~StackReport()
{
    d_stacklist.clear();
}

bool StackReport::b_okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool StackReport::b_upClicked(PG_Button* btn)
{
    d_index--;
    if(d_index < 0)
    d_index = 0;

    fillStackItems();

    return true;
}

bool StackReport::b_downClicked(PG_Button* btn)
{
    d_index++;
    if ((d_index+4) > (int) d_stacklist.size())
        d_index = d_stacklist.size() - 4;

    fillStackItems();

    return true;
}

void StackReport::stackSelected(Stack* s)
{
    sselectingStack.emit(s->getPos());
    Update();
}

void StackReport::fillStackItems()
{
    d_l_number->SetTextFormat("%i - %i", d_index+1, d_index + 4);

    for (int i = 0; i < 4; i++)
    {
        Stack* s = 0;
        if (d_index+i < (int)d_stacklist.size())
            s = d_stacklist[d_index+i];
        d_items[i]->readData(s);
        d_items[i]->Redraw();
    }
}

bool StackReport::eventKeyDown(const SDL_KeyboardEvent* key)
{
    switch(key->keysym.sym)
    {
        case SDLK_RETURN:
            b_okClicked(0);
            break;
        case SDLK_UP:
            b_upClicked(0);
            break;
        case SDLK_DOWN:
            b_downClicked(0);
            break;
        default:
            break;
    }
    return true;
}
