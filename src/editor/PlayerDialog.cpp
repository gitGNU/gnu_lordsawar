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

#include <pglabel.h>
#include <pgapplication.h>
#include <pgmessagebox.h>
#include <sstream>

#include "PlayerDialog.h"
#include "../ai_fast.h"
#include "../playerlist.h"
#include "../armysetlist.h"
#include "../stacklist.h"
#include "../citylist.h"
#include "../armysetlist.h"
#include "../File.h"
#include "../defs.h"

// see end of file for an explanation of this function
void transferPlayer(Player* pold, Player* pnew);


E_PlayerDialog::E_PlayerDialog(PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("Manage players"), PG_Window::MODAL), d_player(0)
{
    // - at the top left and right: buttons for the next/previous player
    // - top middle: selection of type and armyset
    // - below: player data (id, name, color, gold)
    // - below: special selection (neutral, maniac)
    // - below: buttons (ok, add, remove)

    PG_Rect r;
    
    // First the top

    // top left: previous player, top right: next player, middle number of player
    r.SetRect(20, 50, 90, 30);
    d_prev = new PG_Button(this, r, _("prev"));
    d_prev->sigClick.connect(slot(*this, &E_PlayerDialog::playerSelected));

    r.SetRect(my_width - 110, 50, 90, 30);
    d_next = new PG_Button(this, r, _("next"));
    d_next->sigClick.connect(slot(*this, &E_PlayerDialog::playerSelected));

    r.SetRect(my_width/2 - 30, 50, 60, 30);
    d_l_count = new PG_Label(this, r);
    

    // top middle: armyset and type selection
    r.SetRect(my_width/2 - 200, 100, 170, 20);
    d_types = new PG_DropDown(this, r);
    d_types->AddItem(_("Human"));
    d_types->AddItem(_("Dummy AI"));
    d_types->AddItem(_("Fast AI"));
    d_types->AddItem(_("Smart AI"));
    d_types->SetEditable(false);
    d_types->sigSelectItem.connect(slot(*this, &E_PlayerDialog::typeChanged));

    // filling the armyset names is a rather complicated task
    r.SetRect(my_width/2 + 30, 100, 170, 20);
    d_armysets = new PG_DropDown(this, r);
    
    const Armysetlist* as = Armysetlist::getInstance();
    std::vector<Uint32> ids = as->getArmysets();
    for (unsigned int i = 0; i < ids.size(); i++)
        d_armysets->AddItem(_(as->getName(ids[i]).c_str()));
    
    d_armysets->SetEditable(false); 
    d_armysets->sigSelectItem.connect(slot(*this, &E_PlayerDialog::armysetChanged));

    // now the data section
    r.SetRect(20, 160, 40, 20);
    new PG_Label(this, r, _("ID"));
    r.x += 10 + r.w;
    d_l_id = new PG_Label(this, r);

    r.SetRect(20, 190, 50, 20);
    new PG_Label(this, r, _("Name:"));
    r.SetRect(80, 190, 100, 20);
    d_l_name = new PG_LineEdit(this, r);
    d_l_name->sigEditEnd.connect(slot(*this, &E_PlayerDialog::valueChanged));

    r.SetRect(220, 190, 50, 20);
    new PG_Label(this, r, _("Gold:"));
    r.x += 10 + r.w;
    d_l_gold = new PG_LineEdit(this, r, "LineEdit", 5);
    d_l_gold->SetValidKeys("-1234567890");
    d_l_gold->sigEditEnd.connect(slot(*this, &E_PlayerDialog::valueChanged));
    
    r.SetRect(20, 220, 50, 20);
    new PG_Label(this, r, _("Red:"));
    r.x += r.w + 10;
    d_l_red = new PG_LineEdit(this, r, "LineEdit", 3);
    d_l_red->SetValidKeys("1234567890");
    d_l_red->sigEditEnd.connect(slot(*this, &E_PlayerDialog::valueChanged));
        
    r.x += 30 + r.w;
    new PG_Label(this, r, _("Green:"));
    r.x += r.w + 10;
    d_l_green = new PG_LineEdit(this, r, "LineEdit", 3);
    d_l_green->SetValidKeys("1234567890");
    d_l_green->sigEditEnd.connect(slot(*this, &E_PlayerDialog::valueChanged));
        
    r.x += 30 + r.w;
    new PG_Label(this, r, _("Blue:"));
    r.x += r.w + 10;
    d_l_blue = new PG_LineEdit(this, r, "LineEdit", 3);
    d_l_blue->SetValidKeys("1234567890");
    d_l_blue->sigEditEnd.connect(slot(*this, &E_PlayerDialog::valueChanged));
        
    // the special buttons for neutral player and maniac ai etc.
    r.SetRect(20, 260, 150, 20);
    d_neutral = new PG_CheckButton(this, r, _("neutral player"));
    d_neutral->EnableReceiver(false);

    r.y += 30;
    d_persistent = new PG_CheckButton(this, r, _("persistent player"));
    d_persistent->sigClick.connect(slot(*this, &E_PlayerDialog::miscSelected));
    
    r.SetRect(my_width - 170, 260, 150, 20);
    d_maniac = new PG_CheckButton(this, r, _("maniac mode"));
    d_maniac->sigClick.connect(slot(*this, &E_PlayerDialog::miscSelected));

    r.y += 30;
    d_join = new PG_CheckButton(this, r, _("join armies"));
    d_join->sigClick.connect(slot(*this, &E_PlayerDialog::miscSelected));

    // Finally, the buttons at the bottom

    // the button to add a player
    r.SetRect(my_width/2-165, my_height-50, 90, 30);
    d_b_add = new PG_Button(this, r, _("Add"));
    d_b_add->sigClick.connect(slot(*this, &E_PlayerDialog::playerAdded));

    r.x += 110;
    d_b_remove = new PG_Button(this, r, _("Remove"));
    d_b_remove->sigClick.connect(slot(*this, &E_PlayerDialog::playerRemoved));

    r.x += 110;
    PG_Button* b_ok = new PG_Button(this, r, _("OK"));
    b_ok->sigClick.connect(slot(*this, &E_PlayerDialog::okClicked));

    playerSelected(0);
}

E_PlayerDialog::~E_PlayerDialog()
{
}

bool E_PlayerDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool E_PlayerDialog::playerRemoved(PG_Button* btn)
{
    // first, ask the player for sure
    PG_Rect r(my_width/2 - 150, my_height/2 - 75, 300, 130);
    PG_MessageBox mb(this, r, _("Warning"), 
                    _("Do you really want to remove this player?"),
                    PG_Rect(50, 90, 40, 20), _("Yes"),
                    PG_Rect(210, 90, 40, 20), _("No"));
    mb.Show();
    if (mb.RunModal() == 2)
        return true;
    mb.Hide();

    
    Playerlist* pl = Playerlist::getInstance();
    
    if (!d_player || d_player == pl->getNeutral())
        return true;

    // we need to remove all things which the player might possess. These are
    // especially cities and stacks. For simplicity, we just have them taken
    // over by the neutral player
    Player* neutral = pl->getNeutral();
    Stacklist* sl = d_player->getStacklist();
    while (!sl->empty())
    {
        neutral->addStack(*(sl->begin()));
        sl->erase(sl->begin());
    }

    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getPlayer() == d_player)
            (*it).conquer(neutral);

    // TODO: remove all events associated with this player lateron
    // though this is not important (the events aren't raised, then)
    
    
    // remove the player from the playerlist etc.
    for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
        if (d_player == (*it))
        {
            pl->erase(it);
            delete d_player;
            d_player = 0;
            break;
        }

    playerSelected(0);
    
    return true;
}

bool E_PlayerDialog::playerAdded(PG_Button* btn)
{
    SDL_Color c; c.r = 0; c.g = 0; c.b = 0;c.unused=0;
    Uint32 set = (Armysetlist::getInstance()->getArmysets())[0];
    Player* p = Player::create(std::string(d_l_name->GetText()), set, c, Player::AI_DUMMY);
    Playerlist* pl = Playerlist::getInstance();

    // where should the player be put
    PG_Rect r(my_width/2 - 150, my_height/2 - 100, 300, 200);
    PG_MessageBox mb(this, r, _("New Player"), _("Do you want to add the "
                "new player in front of this player or at the end of the list?"),
                PG_Rect(50, 160, 60, 30), _("Here"),
                PG_Rect(190, 160, 60, 30), _("End"));
    mb.Show();
    if (mb.RunModal() == 1)
    {
        Playerlist::iterator it;
        for (it = pl->begin(); it != pl->end(); it++)
            if (*it == d_player)
                break;
        pl->insert(it, p);
    }
    else
        pl->push_back(p);
    
    // update the stat about how many players exist
    playerSelected(0);
    
    return true;
}

bool E_PlayerDialog::playerSelected(PG_Button* btn)
{
    PG_Application::SetBulkMode(true);
   
    const Playerlist* pl = Playerlist::getInstance();
    Playerlist::const_iterator it;
    int index = 0;

    if (d_player != 0)
    {
        for (it = pl->begin(); it != pl->end(); it++)
        {
            index++;
            if (*it == d_player)
                break;
        }
    }

    // first, select the player to be displayed
    if (d_player == 0)
    {
        d_player = *(pl->begin());
        index = 1;
    }
    else if (btn == d_next)
    {
        if (it == Playerlist::getInstance()->end())
        {
            PG_Application::SetBulkMode(false);
            return true;
        }

        index++;
        it++;
        d_player = *it;
    }
    else if (btn == d_prev)
    {
        if (it == Playerlist::getInstance()->begin())
        {
            PG_Application::SetBulkMode(false);
            return true;
        }

        index--;
        it--;
        d_player = *it;
    }

    // now update the data *sigh
    // which is: some id's...
    std::stringstream s;
    s <<index <<"/" <<pl->size();
    d_l_count->SetText(s.str().c_str());

    s.str("");
    s <<d_player->getId();
    d_l_id->SetText(s.str().c_str());

    // ...the dropdown boxes...
    switch (d_player->getType())
    {
        case Player::HUMAN:
            d_types->SetText(_("Human"));
            break;
        case Player::AI_DUMMY:
            d_types->SetText(_("Dummy AI"));
            break;
        case Player::AI_FAST:
            d_types->SetText(_("Fast AI"));
            break;
        case Player::AI_SMART:
            d_types->SetText(_("Smart AI"));
    }

    s.str(Armysetlist::getInstance()->getName(d_player->getArmyset()));
    d_armysets->SetText(s.str().c_str());

    // ...the player data...
    d_l_name->SetText(d_player->getName().c_str());
    
    s.str("");
    s <<d_player->getGold();
    d_l_gold->SetText(s.str().c_str());
    
    s.str("");
    s <<static_cast<Uint32>(d_player->getColor().r);
    d_l_red->SetText(s.str().c_str());

    s.str("");
    s <<static_cast<Uint32>(d_player->getColor().g);
    d_l_green->SetText(s.str().c_str());

    s.str("");
    s <<static_cast<Uint32>(d_player->getColor().b);
    d_l_blue->SetText(s.str().c_str());
    

    // finalize
    checkButtons(); //set the check buttons
    PG_Application::SetBulkMode(false);
    Update();

    return true;
}

bool E_PlayerDialog::typeChanged(PG_ListBoxBaseItem* item)
{
    if (!d_player)
        return true;

    std::string type(item->GetText());
    Player::Type kind = Player::HUMAN;
    Playerlist* pl = Playerlist::getInstance();

    if (type == _("Fast AI"))
        kind = Player::AI_FAST;
    else if (type == _("Dummy AI"))
        kind = Player::AI_DUMMY;
    else if (type == _("Smart AI"))
        kind = Player::AI_SMART;

    // create the new player (unfortunately, you cannot simply change the
    // player type since these are different classes)
    Player* p = Player::create(d_player, kind);


    // since we change players here, we need to copy everything from one to
    // the other player.
    transferPlayer(d_player, p);
    
    // remove the old player from the playerlist and add the new one
    for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
        if ((*it) == d_player)
        {
            it = pl->insert(it, p);     // *it == p
            it++;               // old element is 1 behind the inserted one
            pl->flErase(it);
            break;
        }
    d_player = p;
    
    playerSelected(0);
    return true;
}

bool E_PlayerDialog::armysetChanged(PG_ListBoxBaseItem* item)
{
    if (!d_player)
        return true;
    
    // find the armyset id of the selection
    const Armysetlist* al = Armysetlist::getInstance();
    std::vector<Uint32> sets = al->getArmysets();

    for (unsigned int i = 0; i < sets.size(); i++)
        if (al->getName(sets[i]) == std::string(item->GetText()))
            d_player->setArmyset(sets[i]);

    return true;
}

bool E_PlayerDialog::valueChanged(PG_LineEdit* edit)
{
    if (!d_player)
        return true;
    
    char buffer[11]; buffer[10] = '\0';
    SDL_Color c = d_player->getColor();
    
    if (edit == d_l_name)
        d_player->setName(std::string(edit->GetText()));
    else if (edit == d_l_gold)
            d_player->setGold(atoi(edit->GetText()));
    else if (edit == d_l_red || edit == d_l_green || edit == d_l_blue)
    {
            c.r = atoi(d_l_red->GetText());
            c.g = atoi(d_l_green->GetText());
            c.b = atoi(d_l_blue->GetText());
            d_player->setColor(c);

            // if the user produces out of range results, we correct him here
            snprintf(buffer, 10, "%i", d_player->getColor().r);
            d_l_red->SetText(buffer);
            snprintf(buffer, 10, "%i", d_player->getColor().g);
            d_l_green->SetText(buffer);
            snprintf(buffer, 10, "%i", d_player->getColor().b);
            d_l_blue->SetText(buffer);
    }
            
    return true;
}

bool E_PlayerDialog::miscSelected(PG_RadioButton* btn, bool click)
{
    if (!d_player)
        return true;
    
    if ((btn == d_maniac || btn == d_join)
        && d_player->getType() != Player::AI_FAST)
        return true;

    if (btn == d_maniac || btn == d_join)
    {
        AI_Fast* ai = dynamic_cast<AI_Fast*>(d_player);
        ai->setManiac(d_maniac->GetPressed());
        ai->setJoin(d_join->GetPressed());
    }
    else if (btn == d_persistent)
        d_player->setMortality(!(d_persistent->GetPressed()));

    return true;
}

void E_PlayerDialog::checkButtons()
{
    // care about neutral player...
    d_b_remove->EnableReceiver(true);
    d_neutral->SetUnpressed();
    d_persistent->EnableReceiver(true);
    
    if (d_player->isImmortal())
        d_persistent->SetPressed();
    else
        d_persistent->SetUnpressed();

    if (d_player == Playerlist::getInstance()->getNeutral())
    {
        d_b_remove->EnableReceiver(false);
        d_neutral->SetPressed();
        d_persistent->EnableReceiver(false);
    }
    
    // show/hide special buttons for fast ai...
    d_maniac->Hide();
    d_join->Hide();
    
    if (d_player && d_player->getType() == Player::AI_FAST)
    {
        d_maniac->Show();
        d_join->Show();
        AI_Fast* ai = dynamic_cast<AI_Fast*>(d_player);
        
        if (ai->getManiac())
            d_maniac->SetPressed();
        else
            d_maniac->SetUnpressed();
        
        if (ai->getJoin())
            d_join->SetPressed();
        else
            d_join->SetUnpressed();
    }

    // next or prev buttons have to be dis-/enabled
    d_next->EnableReceiver(true);
    d_prev->EnableReceiver(true);
    
    if (d_player == *(Playerlist::getInstance()->begin()))
        d_prev->EnableReceiver(false);
    if (d_player == *(Playerlist::getInstance()->rbegin()))
        d_next->EnableReceiver(false);
}

void transferPlayer(Player* pold, Player* pnew)
{
    /* Some things about this function:
     * If you change a player's type, you need to actually remove and recreate
     * this player. The reason behind this is simply that different player types
     * are different classes which may save different data and such. So two
     * solutions come to my mind:
     * a) the player parent class saves and handles additional player data in
     *    some generic way, e.g. by storing it in a map<string, bool> or so.
     * b) we need to tweak everything so that the new player seems to behave
     *    like the old did, i.e. transfer all cities.
     * Everything else is not, uhm, generic and thus defined ugly. I have chosen
     * the second approach. To minimize the work, however, the new player already
     * gets the id of the old one, so events and such, that just store the player
     * id, don't need to be altered.
     *  The changed objects so far include: cities, stacks and armies. Stacks are
     *  already changed automatically in the player constructor.
     */
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getPlayer() == pold)
            (*it).setPlayer(pnew);

    // important: if we change the neutral player's type, set the new neutral player!
    if (Playerlist::getInstance()->getNeutral() == pold)
        Playerlist::getInstance()->setNeutral(pnew);
}
