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

#include "armies-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../playerlist.h"
#include "../player.h"
#include "../stacklist.h"
#include "../stack.h"
#include "../army.h"
#include "../FogMap.h"

ArmiesReportDialog::ArmiesReportDialog(Player *theplayer)
{
    player = theplayer;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/armies-report-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    armies_list = Gtk::ListStore::create(armies_columns);
    xml->get_widget("treeview", armies_treeview);
    armies_treeview->set_model(armies_list);
    armies_treeview->append_column("", armies_columns.image);
    armies_treeview->append_column("", armies_columns.player);

    armies_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &ArmiesReportDialog::on_selection_changed));
    // add the stacks, first our own
    Stacklist *sl = player->getStacklist();
    for (Stacklist::iterator i = sl->begin(), end = sl->end(); i != end; ++i)
	add_stack(*i);

    // then those we can see
    for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
	     end = Playerlist::getInstance()->end(); i != end; ++i)
    {
	Player *p = *i;
        if (p == player || p == Playerlist::getInstance()->getNeutral())
            continue;

        sl = p->getStacklist();
	for (Stacklist::iterator j = sl->begin(), jend = sl->end();
	     j != jend; ++j)
	{
	    Stack *s = *j;
	    if (player->getFogMap()->getFogTile(s->getPos()) == FogMap::OPEN)
		add_stack(s);
	}
    }
}

void ArmiesReportDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void ArmiesReportDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();

    dialog->get_size(width, height);
}

void ArmiesReportDialog::on_selection_changed()
{
    Gtk::TreeIter i = armies_treeview->get_selection()->get_selected();
    if (i)
    {
	Stack *stack = (*i)[armies_columns.stack];
	stack_selected.emit(stack);
    }
}

void ArmiesReportDialog::add_stack(Stack *stack)
{
    Gtk::TreeIter i = armies_list->append();
    (*i)[armies_columns.player] = stack->getPlayer()->getName();
    (*i)[armies_columns.stack] = stack;

    // FIXME: this is rather crude - we should probably at least show the
    // remaining movement points of the armies, and also do something to the
    // size of the imagery
    
    // now the image
    
    if (stack->empty())
	return;

    // create a surface that contains all the armies images
    const int internal_padding = 4;
    int width = 0, height = 0;
    SDL_PixelFormat *fmt = 0;
    for (Stack::iterator i = stack->begin(), end = stack->end(); i != end; ++i)
    {
	SDL_Surface *pic = (*i)->getPixmap();
	width += pic->w;
	if (pic->h > height)
	    height = pic->h;
	fmt = pic->format;
    }
    width += (stack->size() - 1) * internal_padding;
    
    SDL_Surface *surface
	= SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, fmt->BitsPerPixel,
			       fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_FillRect(surface, 0, SDL_MapRGBA(fmt, 0, 0, 0, 0));
    
    SDL_Rect r;
    r.x = r.y = 0;
    for (Stack::iterator i = stack->begin(), end = stack->end(); i != end; ++i)
    {
	SDL_Surface *pic = (*i)->getPixmap();
	// we have to save, overwrite & restore alpha settings because
	// otherwise SDL uses the destination's alpha channel
	Uint32 flags = pic->flags; 
	SDL_SetAlpha(pic, 0, 0);
	SDL_BlitSurface(pic, 0, surface, &r);
	pic->flags = flags;
	r.x += pic->w + internal_padding;
    }
    
    // and action
    (*i)[armies_columns.image] = to_pixbuf(surface);
    SDL_FreeSurface(surface);
}
