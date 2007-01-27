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

#include <string>
#include <pglistboxitem.h>
#include "File.h"
#include "Configuration.h"
#include "ScenariosDialog.h"
#include "MainWindow.h"
#include "w_edit.h"
#include "defs.h"
#include "GameMap.h"
#include "vectormap.h"
#include "citylist.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

ScenariosDialog::ScenariosDialog(PG_Widget* parent, PG_Rect rect,string * fname)
  :PG_Window(parent, rect, _("Load Scenario"),PG_Window::MODAL)
{
    debug("ScenariosDialog()");
    // The Scenarios dialog consists of two parts:
    // - a list of game maps (left) that takes a large portion, where you select
    //   the map filename
    // - some labels for the map description and in future pheraps some an image

    // First part: place the buttons and maps listbox
    d_name="";
    d_comment="";
    filename=fname;

    l_name = new PG_Label(this, PG_Rect(160, 32, 80, 25), _("Name:"));
    l_desc = new PG_Label(this, PG_Rect(160, 62, 90, 25), _("Description:"));

    l_scname = new PG_Label(this, PG_Rect(160+l_name->GetTextWidth()+5, 35, 80, 25), "");
    //l_scdesc = new PG_Label(this, PG_Rect(160+l_desc->GetTextWidth()+5, 65, 90, 25), "");
	

    //l_scdesc=new PG_MultiLineEdit(this,PG_Rect(160+l_desc->GetTextWidth()+5, 65, 230, 150));
    l_scdesc=new PG_MultiLineEdit(this,PG_Rect(160, 65, 335, 150));
    l_scdesc->SetText("Once upon a time, in a galaxy far far away...\n...\n...\n...\n...\n...\n...\nand they lived happily till once upon a time, in a galaxy far far away...\n[INSERT STORY HERE]");
    l_scdesc->Show();
    l_scdesc->SetEditable(false);

    d_background = File::getMiscPicture("about_screen.jpg", false);
    PG_ThemeWidget* d_back = new PG_ThemeWidget(this, PG_Rect(160, 225, 335, 225));
    d_back->SetBackground(d_background, BKMODE_STRETCH);



    PG_Rect myrect(245, Height() - 40, 0, 30);
    d_b_ok = new PG_Button(this, myrect, _("OK"));
    d_b_ok->SizeWidget(d_b_ok->GetTextWidth()+40, 30, true);

    myrect.SetRect(245+d_b_ok->GetTextWidth()+50, Height() - 40, 0, 30);
    d_b_canc = new PG_Button(this, myrect, _("Cancel"));
    d_b_canc->SizeWidget(d_b_canc->GetTextWidth()+40, 30, true);

    myrect.x = 8;
    myrect.y = 32;

    myrect.w = 140;
    myrect.h = my_height - 40;
    d_maps = new PG_ListBox(this, myrect);
    d_maps->SetMultiSelect(false);

    list<string> lm = File::scanMaps();
    
    // and fill it
    for (list<string>::iterator it = lm.begin(); it != lm.end(); it++)
    {
        //Andrea: here i must find a better solution to store the filenames 
        PG_ListBoxItem* item = new PG_ListBoxItem(d_maps, 20, (*it).c_str(),
                         0, static_cast<void*> (&(*it)));
        item->Select();
    }

    d_maps->SelectFirstItem();
    mapSelected(d_maps->GetSelectedItem());

    l_scname->SetText(_(d_name.c_str()));
    l_scname->SetSizeByText();  	

    l_scdesc->SetText(_(d_comment.c_str()));

    // last stuff
    d_b_ok->sigClick.connect(slot(*this, &ScenariosDialog::okClicked));
    d_b_canc->sigClick.connect(slot(*this, &ScenariosDialog::cancelClicked));
    d_maps->sigSelectItem.connect(slot(*this, &ScenariosDialog::mapSelected));
}

ScenariosDialog::~ScenariosDialog()
{
    delete d_b_ok;
    delete d_b_canc;
    delete d_maps;
    delete l_name;
    delete l_desc;
    delete l_scname;
    SDL_FreeSurface(d_background);
    delete l_scdesc;
}

bool ScenariosDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool ScenariosDialog::cancelClicked(PG_Button* btn) 
{
    *filename="";    
    QuitModal();
    return true;
}


bool ScenariosDialog::mapSelected(PG_ListBoxBaseItem* item)
{

    //Andrea: here i must find a better solution to get the filenames 
    //    filename =  static_cast<string*> (item->GetUserData());
    debug(cerr << "Value= " << item->GetText() << " ----------- ")
    *filename= string(item->GetText());

    string savegame = Configuration::s_dataPath+string("/map/")+(*filename)+string(".map");
    XML_Helper helper(savegame, ios::in, Configuration::s_zipfiles);

    helper.registerTag("scenario", SigC::slot((*this), &ScenariosDialog::scan));

    //now parse the document and close the file afterwards
    if (!helper.parse())
    {
        cerr << "Error: Could not parse " << savegame << " ! Exiting!" << endl;
        exit(-1);
    }

    helper.close();

    cerr << "Scenario name= " << d_name << endl;
    cerr << "Scenario comment= " << d_comment << endl;

    l_scname->SetText(_(d_name.c_str()));
    l_scname->SetSizeByText();

    l_scdesc->SetText(_(d_comment.c_str()));

    Update(); 
    
    return true;
}

bool ScenariosDialog::scan(std::string tag, XML_Helper* helper)
{
    if (tag == "scenario")
    {
        if (helper->getVersion() != FREELORDS_SAVEGAME_VERSION)
        {
            cerr <<_("savefile has wrong version, we want ");
            std::cerr <<FREELORDS_SAVEGAME_VERSION <<",\n";
            cerr <<_("savefile offers ") << helper->getVersion() <<".\n";
            return false;
        }
    
        debug("loading scenario")
        helper->getData(d_name, "name");
        helper->getData(d_comment, "comment");
    }

    return true;
}

// End of file
