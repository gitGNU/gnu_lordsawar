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
#include <pgrichedit.h>
#include "playerlist.h"
#include "QuestsManager.h"
#include "questsreport.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

//first comes the implementation of QuestItem. A QuestItem is a bar of 420x100
//pixels which displays infos about a single stack.
class QuestItem : public PG_Widget
{
    public:
        QuestItem(PG_Widget* parent, Rectangle rect);
        ~QuestItem();

        void fillItem(Quest* quest, Hero *hero);

        sigc::signal<void, Quest*> sclicked;

    private:
        bool eventMouseButtonDown(const SDL_MouseButtonEvent * event);

        PG_Label* d_l_hero;
        PG_RichEdit* d_l_quest;
        PG_Label* d_l_status;
        Quest* d_quest;
};

QuestItem::QuestItem(PG_Widget* parent, Rectangle rect)
                    :PG_Widget(parent, rect)
{
    d_l_hero = new PG_Label(this, Rectangle(10, 10, 150, 15), "");
    d_l_quest = new PG_RichEdit(this, Rectangle(10, 30, 410, 40), "");
    d_l_quest->SetTransparency(255);
    d_l_status = new PG_Label(this, Rectangle(10, 75, 410, 15), "");
}

QuestItem::~QuestItem()
{
    delete d_l_hero;
    delete d_l_quest;
    delete d_l_status;
}

void QuestItem::fillItem(Quest* quest, Hero *hero)
{
    d_quest = quest;
    Player *player = Playerlist::getActiveplayer();

    SDL_Color color = player->getColor();
    d_l_hero->SetFontColor(PG_Color(color.r, color.g, color.b));

    std::stringstream s;
    s << hero->getName() << "(" << hero->getId() << ")";
    d_l_hero->SetText(s.str().c_str());

    d_l_quest->SetText(quest->getDescription());
    d_l_status->SetText(quest->getProgress().c_str());
}

bool QuestItem::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{
    if (!IsVisible())
        return true;
    sclicked.emit(d_quest);
    return true;
}


QuestsReport::QuestsReport(PG_Widget* parent, Rectangle rect)
    :PG_Window(parent, rect, _("Quests report"), PG_Window::MODAL), d_index(0)
{
    //I assume 500x400 pixels here as well
    d_b_ok = new PG_Button(this, Rectangle(435, 370, 90, 25), _("OK"),2);
    d_b_ok->sigClick.connect(slot(*this, &QuestsReport::b_okClicked));

    d_b_up = new PG_Button(this, Rectangle(435, 150, 90, 25), _("Prev."),0);
    d_b_up->sigClick.connect(slot(*this, &QuestsReport::b_upClicked));

    d_b_down = new PG_Button(this, Rectangle(435, 250, 90, 25), _("Next"),1);
    d_b_down->sigClick.connect(slot(*this, &QuestsReport::b_downClicked));

    d_l_number = new PG_Label(this, Rectangle(435, 35, 60, 20), "");

    d_items[0] = new QuestItem(this, Rectangle(10, 30, 420, 100));
    d_items[1] = new QuestItem(this, Rectangle(10, 140, 420, 100));
    d_items[2] = new QuestItem(this, Rectangle(10, 250, 420, 100));

    d_l_noquests = new PG_Label(this, Rectangle(10, 35, 480, 40), "");

    Player *player = Playerlist::getActiveplayer();
    // read quests for this player
    QuestsManager *q_mgr = QuestsManager::getInstance();
    q_mgr->getPlayerQuests(player, d_questlist, d_heroes);
    d_index = 0;
    fillQuestItems();
}

QuestsReport::~QuestsReport()
{
    delete d_l_noquests;

    delete d_items[0];
    delete d_items[1];
    delete d_items[2];

    delete d_l_number;
    delete d_b_down;
    delete d_b_up;
    delete d_b_ok;
}

bool QuestsReport::b_okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool QuestsReport::b_upClicked(PG_Button* btn)
{
    d_index--;
    if(d_index < 0)
        d_index = 0;
    return true;
}

bool QuestsReport::b_downClicked(PG_Button* btn)
{
    d_index++;
    if ((d_index+3) > (int) d_questlist.size())
        d_index = d_questlist.size() - 3;
    fillQuestItems();
    return true;
}

void QuestsReport::fillQuestItems()
{
    debug("fillQuestItems: count = " << d_heroes.size());
    if (d_heroes.size() > 0)
    {
        unsigned int max = d_index + 3;
        if (max > d_heroes.size())
            max = d_heroes.size();
        d_l_number->SetTextFormat("%i - %i", d_index+1, max);
    }
    else
    {
        Player *player = Playerlist::getActiveplayer();
        d_l_noquests->SetTextFormat(_("No quests for %s"), 
                                    player->getName().c_str());
    }
    for (unsigned int i = 0; i < 3; i++)
    {
        unsigned int idx = d_index+i;
        if (idx >= d_heroes.size())
            return;
        debug("fillItem: i = " << i << ", idx " << idx);
        d_items[i]->fillItem(d_questlist[idx], d_heroes[idx]);
        d_items[i]->Redraw();
    }
}

bool QuestsReport::eventKeyDown(const SDL_KeyboardEvent* key)
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
