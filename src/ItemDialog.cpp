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
#include <pgbutton.h>
#include <sstream>

#include "ItemDialog.h"
#include "GameMap.h"
#include "defs.h"
#include "Configuration.h"

//for getting the position
#include "player.h"
#include "stacklist.h"
#include "stack.h"
#include "File.h"
#include "Tile.h"

const int ground_id = 100;      //minimum id for buttons depicting stuff on the ground
const int back_id = 200;        // same for backpack stuff
// the hero's items are labelled using Item::Type as id

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)


ItemDialog::ItemDialog(Hero* hero, PG_Widget* parent, Rectangle rect)
    :PG_Window(parent, rect, hero->getName().c_str(), PG_Window::MODAL),
    d_hero(hero), d_ground(0), d_back(0), d_nground(0), d_nback(0),
    d_showdesc(false), d_location(Action_Equip::NONE), d_index(0)
{
    // Load the layout from a file. We assume that paragui has
    // physfs-paths pointing to our theme directory anyway, so we
    // skip this check.

    // Hack: We need to use physfs a bit more intelligently
    PG_Application::AddArchive(Configuration::s_dataPath.c_str());
    if (!LoadLayout("theme/generic/ItemDialog.xml"))
    {
        //notify somehow the dialog is broken...
        return;
    }

    // this widget is used later on with actual
    d_description = new PG_RichEdit(this, Rectangle(0,0,0,0), true, 250, 0);
    // bug in PG_RichEdit
    d_description->SetText("test");
    d_description->Show();
    d_description->Hide();

    // This is neccessary due to a bug in paragui which would have problems with
    // resetting the icon to nothing
    d_tempsurf = File::getItemPicture("nobutton");
    

    // misc setup
    PG_Button* btn = (PG_Button*)FindChild("b_ok");
    btn->sigClick.connect(slot(*this, &ItemDialog::okClicked));

    ((PG_Button*)FindChild("army"))->SetIcon(d_hero->getPixmap(), 0, 0);

    ((PG_Button*)FindChild(11))->sigClick.connect(slot(*this, &ItemDialog::scrollClicked));
    ((PG_Button*)FindChild(12))->sigClick.connect(slot(*this, &ItemDialog::scrollClicked));
    ((PG_Button*)FindChild(21))->sigClick.connect(slot(*this, &ItemDialog::scrollClicked));
    ((PG_Button*)FindChild(22))->sigClick.connect(slot(*this, &ItemDialog::scrollClicked));
    
    // find out how many ground and backpack buttons we got and disable their event
    // processing
    PG_Widget* w;
    for (int id = ground_id; (w = FindChild(id)); id++, d_nground++)
        w->EnableReceiver(false);
    for (int id = back_id; (w = FindChild(id)); id++, d_nback++)
        w->EnableReceiver(false);

    // disable the equipment buttons as well
    for (int i = 0; i < 4; i++)
        FindChild(i)->EnableReceiver(false);

    debug("Dialog has "<<d_nground <<" ground items, "<<d_nback<<" backpack slots")

    drawItems();
}

ItemDialog::~ItemDialog()
{
    // not much to clean up
}

bool ItemDialog::okClicked(PG_Button* btn)
{
    debug("OK clicked")

    QuitModal();
    return true;
}

bool ItemDialog::scrollClicked(PG_Button* btn)
{
    debug("Click on "<<btn->GetID())

    // if the player clicked the scroll buttons, change the appropriate counters
    switch(btn->GetID())
    {
        case 11:            // previous ground
            d_ground--;
            break;
        case 12:            // next ground
            d_ground++;
            break;
        case 21:            // previous backpack
            d_back--;
            break;
        case 22:            // next backpack
            d_back++;
            break;
    }


    // if the data is out of bounds, adjust it now.
    Vector<int> pos = Stacklist::getPosition(d_hero->getId());
    int size = GameMap::getInstance()->getTile(pos.x,pos.y)->getItems().size();
    if (d_ground + d_nground > size + 1 && d_nground <= size)
        d_ground--;

    size = d_hero->getBackpack().size();
    if (d_back + d_nback > size + 1 && d_nback <= size)
        d_back--;

    if (d_ground < 0)
        d_ground = 0;

    if (d_back < 0)
        d_back = 0;

    drawItems();
    return true;
}

void ItemDialog::drawItems()
{
    std::list<Item*> ilist;
    std::list<Item*>::const_iterator iit;

    // first, the hero's equipment
    ilist = d_hero->getEquipment();
    for (int i = 0; i < 4; i++)
        ((PG_Button*)FindChild(i))->SetIcon(d_tempsurf);

    for (iit = ilist.begin(); iit != ilist.end(); iit++)
    {
        debug("Drawing "<<(*iit)->getName()<<" on body")

        PG_Button* btn = (PG_Button*)FindChild((*iit)->getType());
        btn->SetIcon((*iit)->getPic());
    }

    // the hero's backpack
    ilist = d_hero->getBackpack();
    int count = 0;
    for (iit = ilist.begin(); count < d_back; count++, iit++);
    for (; count < d_back + d_nback; count++)
    {
        debug("drawing " <<(iit != ilist.end()? (*iit)->getName():"nothing")
              <<" in backpack " <<count-d_back)

        PG_Button* btn = (PG_Button*)FindChild(back_id + count - d_back);
        if (!btn)
            continue;
        if (iit != ilist.end())
            btn->SetIcon((*iit)->getPic());
        else
            btn->SetIcon(d_tempsurf);

        if (iit != ilist.end())
            iit++;
    }

    // and the items on the ground
    Vector<int> pos = Stacklist::getPosition(d_hero->getId());
    ilist = GameMap::getInstance()->getTile(pos.x, pos.y)->getItems();
    count = 0;
    for (iit = ilist.begin(); count < d_ground; count++, iit++);
    for (; count < d_ground + d_nground; count++)
    {
        debug("drawing " <<(iit != ilist.end()? (*iit)->getName():"nothing")
              <<" on ground " <<count-d_ground)

        PG_Button* btn = (PG_Button*)FindChild(ground_id + count - d_ground);
        if (!btn)
            continue;
        if (iit != ilist.end())
            btn->SetIcon((*iit)->getPic());
        else
            btn->SetIcon(d_tempsurf);

        if (iit != ilist.end())
            iit++;
    }
    Redraw();
}

void ItemDialog::createDescription(Item* item)
{
    std::stringstream ss;

    // first line: name of the item
    ss <<item->getName() <<std::endl;

    // other lines: status changes. Long if-conditions; add an explicit
    // "+" if the value is positive
    if (item->getBonus(Army::STRENGTH))
        ss <<(item->getValue(Army::STRENGTH) > 0? "+":"")
           <<item->getValue(Army::STRENGTH) <<"  " <<_("Strength") <<std::endl;

    if (item->getBonus(Army::DEFENSE))
        ss <<(item->getValue(Army::DEFENSE) > 0? "+":"")
           <<item->getValue(Army::DEFENSE) <<"  " <<_("Defense") <<std::endl;

    if (item->getBonus(Army::VITALITY))
        ss <<(item->getValue(Army::VITALITY) > 0? "+":"")
           <<item->getValue(Army::VITALITY) <<"  " <<_("Vitality") <<std::endl;

    if (item->getBonus(Army::HP))
        ss <<(item->getValue(Army::HP) > 0? "+":"")
           <<item->getValue(Army::HP) <<"  " <<_("Hitpoints") <<std::endl;

    if (item->getBonus(Army::MOVES))
        ss <<(item->getValue(Army::MOVES) > 0? "+":"")
           <<item->getValue(Army::MOVES) <<"  " <<_("Movement") <<std::endl;

    if (item->getBonus(Army::SIGHT))
        ss <<(item->getValue(Army::SIGHT) > 0? "+":"")
           <<item->getValue(Army::SIGHT) <<"  " <<_("Sight") <<std::endl;

    if (item->getBonus(Army::RANGED))
        ss <<(item->getValue(Army::RANGED) > 0? "+":"")
           <<item->getValue(Army::RANGED) <<"  " <<_("Ranged") <<std::endl;

    if (item->getBonus(Army::SHOTS))
        ss <<(item->getValue(Army::SHOTS)? "+":"")
           <<item->getValue(Army::SHOTS) <<"  " <<_("Shots") <<std::endl;

    // the movement and army boni have to be treated differently
    if (item->getBonus(Army::MOVE_BONUS))
    {
        ss <<_("bonus for ");
        Uint32 bonus = item->getValue(Army::MOVE_BONUS);
        if (bonus & Tile::WATER)
            ss <<_("water ");
        if (bonus & Tile::FOREST)
            ss <<_("forest ");
        if (bonus & Tile::HILLS)
            ss <<_("hills ");
        if (bonus & Tile::MOUNTAIN)
            ss <<_("mountain ");
        if (bonus & Tile::SWAMP)
            ss <<_("swamp");
        ss <<std::endl;
    }

    // here we restrict ourselves to bonuses that actually make sense
    if (item->getBonus(Army::ARMY_BONUS))
    {
        Uint32 bonus = item->getValue(Army::ARMY_BONUS);
        if (bonus & Army::LEADER)
            ss <<_("grants leadership") <<std::endl;
        if (bonus & Army::ANTICAVALRY)
            ss <<_("good against cavalry") <<std::endl;
        if (bonus & Army::REGENERATE)
            ss <<_("provides regeneration") <<std::endl;
        if (bonus & Army::CRITICAL)
            ss <<_("score critical hits") <<std::endl;
    }

    // finally, assign this text to the description widget and resize it
    d_description->SetText(ss.str().c_str());
}

Item* ItemDialog::getItem(Action_Equip::Slot* location, int* index)
{
    // look if the mouse cursor is over an item on the ground
    for (int id = ground_id; id < ground_id + d_nground; id++)
        if (FindChild(id)->IsMouseInside())
        {
            if (location)
                *location = Action_Equip::GROUND;
            if (index)
                *index = d_ground + (id-ground_id);

            // return the item (mmh, much code for little result)
            Vector<int> pos = Stacklist::getPosition(d_hero->getId());
            std::list<Item*> il = GameMap::getInstance()->getTile(pos.x, pos.y)->getItems();
            std::list<Item*>::iterator it;
            int count = 0;
            for (it = il.begin(); count < d_ground+(id-ground_id); count++)
                if (it != il.end())
                    it++;

            if (it != il.end())
                return (*it);
            return 0;
        }

    // or in the backpack
    for (int id = back_id; id < back_id + d_nback; id++)
        if (FindChild(id)->IsMouseInside())
        {
            if (location)
                *location = Action_Equip::BACKPACK;
            if (index)
                *index = d_back + (id-back_id);

            std::list<Item*> il = d_hero->getBackpack();
            std::list<Item*>::iterator it;
            int count = 0;
            for (it = il.begin(); count < d_back+(id-back_id); count++)
                if (it != il.end())
                    it++;

            if (it != il.end())
                return (*it);
            return 0;
        }

    // or on the hero's body (id 0 - 3)
    for (unsigned int id = 0; id < 4; id++)
        if (FindChild(id)->IsMouseInside())
        {
            if (location)
                *location = Action_Equip::BODY;
            if (index)
                *index = id;

            // find (or do not) the item in the hero's equipment
            std::list<Item*> il = d_hero->getEquipment();
            std::list<Item*>::iterator it;
            for (it = il.begin(); it != il.end(); it++)
                if ((*it)->getType() == id)
                {
                    return (*it);
                }

            return 0;
        }

    return 0;
}

Item* ItemDialog::getItem(Action_Equip::Slot location, int index)
{
    Vector<int> pos = Stacklist::getPosition(d_hero->getId());
    std::list<Item*> il;
    std::list<Item*>::iterator it;

    switch(location)
    {
        case Action_Equip::GROUND:
            // loop through the items on the ground, take the index-th element
            il = GameMap::getInstance()->getTile(pos.x, pos.y)->getItems();

            for (it = il.begin(); index > 0; it++, index--);

            if (it == il.end())
                return 0;
            return (*it);
            break;

        case Action_Equip::BACKPACK:
            // same for the backpack
            il = d_hero->getBackpack();

            for (it = il.begin(); index > 0; it++, index--);

            if (it == il.end())
                return 0;
            return (*it);
            break;

        case Action_Equip::BODY:
            // search the hero's equipped items; here, index is the item type!
            il = d_hero->getEquipment();

            for (it = il.begin(); it != il.end(); it++)
                if ((int)(*it)->getType() == index)
                    return (*it);
            break;
        case Action_Equip::NONE:
	    break;
    }

    return 0;
}

bool ItemDialog::removeItem(Action_Equip::Slot location, int index)
{
    Vector<int> pos = Stacklist::getPosition(d_hero->getId());
    Item* item = getItem(location, index);
    debug("removing item " <<item->getName() <<" from " <<location)

    switch(location)
    {
        case Action_Equip::GROUND:
            GameMap::getInstance()->getTile(pos.x,pos.y)->removeItem(item);
            break;
        case Action_Equip::BACKPACK:
            d_hero->removeFromBackpack(item);
            break;
        case Action_Equip::BODY:
            d_hero->removeFromEquipment(item);
            break;
        default:
            return false;
    }

    return true;
}

bool ItemDialog::addItem(Action_Equip::Slot location, int index, Item* item)
{
    Vector<int> pos = Stacklist::getPosition(d_hero->getId());
    debug("adding item " <<item->getName() <<" to " <<location)

    switch(location)
    {
        case Action_Equip::GROUND:
            GameMap::getInstance()->getTile(pos.x,pos.y)->addItem(item, index);
            break;
        case Action_Equip::BACKPACK:
            d_hero->addToBackpack(item, index);
            break;
        case Action_Equip::BODY:
            d_hero->addToEquipment(item);
            break;
        default:
            return false;
    }

    return true;
}

bool ItemDialog::eventMouseButtonDown(const SDL_MouseButtonEvent* ev)
{
    Item* item = getItem();
    if (item == 0)
        return false;

    if (ev->button == SDL_BUTTON_RIGHT)
    {
        debug("requesting description of " <<item->getName())
        // We want to show some description of the item, so
        
        // a) fill the description text
        createDescription(item);

        // b) move it to the mouse cursor
        int xpos = ev->x - my_xpos;
        if (xpos + d_description->my_width > my_width)
            xpos = my_width - d_description->my_width;

        int ypos = ev->y - my_ypos;
        if (ypos + d_description->my_height > my_height)
            ypos = my_height - d_description->my_height;

        d_description->MoveWidget(xpos, ypos);

        // c) and show it
        d_description->Show();
        d_showdesc = true;
        return true;
    }
    if (ev->button == SDL_BUTTON_LEFT)
    {
        debug("Selecting " <<item->getName())
        // The items shall be assigned by drag'n drop, so if the user presses
        // down the button, memorize the item he has clicked and morph the mouse
        // cursor to the current item pic.
        getItem(&d_location, &d_index);

        // This leads to severe graphics problems
        //PG_Application::ShowCursor(PG_Application::SOFTWARE);
        //PG_Application::SetCursor(item->getPic());

        return true;
    }

    return false;
}

bool ItemDialog::eventMouseButtonUp(const SDL_MouseButtonEvent* ev)
{
    if (ev->button == SDL_BUTTON_RIGHT && d_showdesc)
    {
        d_description->Hide();
        d_showdesc = false;
        return true;
    }
    if (ev->button == SDL_BUTTON_LEFT && d_location != Action_Equip::NONE)
    {
        //PG_Application::ShowCursor(PG_Application::HARDWARE);
        Action_Equip::Slot location = Action_Equip::NONE;
        int index;

        getItem(&location, &index);

        // mouse click was released _somewhere_ => ignore it
        if (location == Action_Equip::NONE)
        {
            d_location = Action_Equip::NONE;
            return true;
        }

        // special sanity checks:
        // a) IF we are moving some item on an
        // equipment slot AND the item is of the wrong type (e.g.
        // picking up a shield from the ground to the weapon slot),
        // we abort here.
        // b) if both locations are the same, there is little use
        // in doing _anything_
        if ((location == Action_Equip::BODY 
            && (int)getItem(d_location,d_index)->getType() != index)
            || (location == d_location))
        {
            d_location = Action_Equip::NONE;
            return true;
        }

        // if there is no item at the destination slot, this is just a simple matter
        // of moving th eitem from d_location/index to location/index.
        if (!getItem(location, index))
        {
            Item* item = getItem(d_location, d_index);
            removeItem(d_location, d_index);
            addItem(location, index, item);

            // fill in the equipment action
            Action_Equip* a = new Action_Equip();
            a->fillData(d_hero->getId(), item->getId(), location, index);
            d_hero->getPlayer()->getActionlist()->push_back(a);

            d_location = Action_Equip::NONE;
            drawItems();
            return true;
        }

        // else swap the items. The procedure is very simple: Remove item a
        // at position (loc1,index1), add it to (loc2), then the other way round.
        // To make things more complicated, we need to 
        // a) ensure that _always_ the equipped item is removed first!
        // b) fill in the actions for the item shuffling.

        // =>(a) Thus, as a first step, we find an order. We first move from source
        // to target, then the other way round.
        Action_Equip::Slot s_loc, t_loc;
        int s_index, t_index;

        s_loc = d_location;
        s_index = d_index;
        t_loc = location;
        t_index = index;

        if (t_loc == Action_Equip::BODY)
        {
            // swap
            s_loc = location;       s_index = index;
            t_loc = d_location;     t_index = d_index;
        }

        // first move from source to target...
        Item* item = getItem(s_loc, s_index);
        if (item)
        {
            removeItem(s_loc, s_index);
            addItem(t_loc, t_index, item);

            // fill in the equipment action; a bit ugly to do so outside of the
            // player class. Consider this as a hack :)
            Action_Equip* a = new Action_Equip();
            a->fillData(d_hero->getId(), item->getId(), t_loc, t_index);
            d_hero->getPlayer()->getActionlist()->push_back(a);
        }

        // now the other way round, substitute s_* by t_* and note that the
        // second item has moved back one position.
        item = getItem(t_loc, t_index+1);
        if (item)
        {
            removeItem(t_loc, t_index+1);
            addItem(s_loc, s_index, item);

            // fill in the equipment action
            Action_Equip* a = new Action_Equip();
            a->fillData(d_hero->getId(), item->getId(), s_loc, s_index);
            d_hero->getPlayer()->getActionlist()->push_back(a);
        }

        d_location = Action_Equip::NONE;
        drawItems();
        return true;
    }

    return false;
}

