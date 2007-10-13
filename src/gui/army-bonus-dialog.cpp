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

#include <config.h>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "army-bonus-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../army.h"
#include "../armysetlist.h"
#include "../player.h"
#include "../GraphicsCache.h"

ArmyBonusDialog::ArmyBonusDialog(Player *p)
{
    d_player = p;
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/army-bonus-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    armies_list = Gtk::ListStore::create(armies_columns);
    xml->get_widget("treeview", armies_treeview);
    armies_treeview->set_model(armies_list);
    armies_treeview->append_column("", armies_columns.image);
    armies_treeview->append_column("", armies_columns.name);
    armies_treeview->append_column("Str", armies_columns.str);
    armies_treeview->append_column("Move", armies_columns.move);
    armies_treeview->append_column("", armies_columns.move_image);
    armies_treeview->append_column("Bonus", armies_columns.bonus);
    armies_treeview->set_headers_visible(true);

    Uint32 max = Armysetlist::getInstance()->getSize(d_player->getArmyset());
    for (unsigned int i = 0; i < max; i++)
      addArmyType(i);
}

void ArmyBonusDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void ArmyBonusDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();

    dialog->get_size(width, height);

}

void ArmyBonusDialog::addArmyType(Uint32 army_type)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    Player *p = d_player;
    const Army *a;
    a = Armysetlist::getInstance()->getArmy(p->getArmyset(), army_type);
    if (a->isHero())
      return; //we don't want to show heroes in this list
    Gtk::TreeIter i = armies_list->append();
    (*i)[armies_columns.name] = a->getName();
    (*i)[armies_columns.image] = to_pixbuf(gc->getArmyPic(p->getArmyset(),
                                           army_type,
                                           p, NULL));
    (*i)[armies_columns.str] = a->getStat(Army::STRENGTH, false);
    (*i)[armies_columns.move] = a->getStat(Army::MOVES, false);
    Uint32 b = a->getStat(Army::MOVE_BONUS, false);
    (*i)[armies_columns.move_image] = to_pixbuf(gc->getMoveBonusPic(b, false));
    (*i)[armies_columns.bonus] = "-";
    Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
    Glib::ustring s = "";
    if (bonus & Army::ADD1STRINOPEN)
      s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
                            _("+1 str in open"));
    if (bonus & Army::ADD2STRINOPEN)
      s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
                            _("+2 str in open"));
    if (bonus & Army::ADD1STRINFOREST)
      s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
                            _("+1 str in woods"));
    if (bonus & Army::ADD1STRINHILLS)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("+1 str in hills"));
    if (bonus & Army::ADD1STRINCITY)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("+1 str in city"));
    if (bonus & Army::ADD2STRINCITY)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("+2 str in city"));
    if (bonus & Army::ADD1STACKINHILLS)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("+1 stack in hills"));
    if (bonus & Army::SUBALLCITYBONUS)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("Cancel city bonus"));
    if (bonus & Army::SUB1ENEMYSTACK)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("-1 enemy stack"));
    if (bonus & Army::ADD1STACK)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", _("+1 stack"));
    if (bonus & Army::ADD2STACK)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", _("+2 stack"));
    if (bonus & Army::SUBALLNONHEROBONUS)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("cancel non-hero"));
    if (bonus & Army::SUBALLHEROBONUS)
      s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
                            _("cancel hero"));
    if (s == "")
      (*i)[armies_columns.bonus] = "-";
    else
      (*i)[armies_columns.bonus] = s;
}
