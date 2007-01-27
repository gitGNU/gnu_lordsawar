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

#include <locale.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "LangDialog.h"
#include "Configuration.h"
#include "defs.h"

LangDialog::LangDialog(PG_Widget* parent, const PG_Rect rect)
    :PG_Window(parent, rect), d_b_german(0), d_b_italian(0)
{
    // save the current locale
    d_origlang[20] = '\0';
    strncpy(d_origlang, setlocale(LC_ALL, ""), 20);

    // initiate the cosen language
    d_chosenlang[20] = '\0';
    strncpy(d_chosenlang, setlocale(LC_ALL, ""), 20);

    // Place the OK and Cancel buttons
    d_b_ok = new PG_Button(this, PG_Rect(30, my_height - 50, 80, 30), _("OK"));
    d_b_ok->sigClick.connect(slot(*this, &LangDialog::okClicked));

    d_b_cancel = new PG_Button(this, PG_Rect(my_width - 110, my_height - 50, 80, 30),
                               _("Cancel"));
    d_b_cancel->sigClick.connect(slot(*this, &LangDialog::cancelClicked));

    // English localisation is always possible
    PG_Rect r(30, 30, 200, 20);
    d_b_english = new PG_RadioButton(this, r, "English (C)");
    d_b_english->sigClick.connect(slot(*this, &LangDialog::langChosen));

    // now check if German localisation is possible, set German locale for
    // testing purpose
    if (setlocale(LC_ALL, "de_DE.utf8") &&
        (strcmp(setlocale(LC_ALL, "de_DE.utf8"), "de_DE.utf8") == 0))
    {
        r.y += 30;
        d_b_german = new PG_RadioButton(this, r, "Deutsch (de_DE.utf8)", d_b_english);
        d_b_german->sigClick.connect(slot(*this, &LangDialog::langChosen));
    }

    // the same: check for Italian localisation
    if (setlocale(LC_ALL, "it_IT.utf8") &&
        (strcmp(setlocale(LC_ALL, "it_IT.utf8"), "it_IT.utf8") == 0))
    {
        r.y += 30;
        d_b_italian = new PG_RadioButton(this, r, "Italiano (it_IT.utf8)", d_b_english);
        d_b_italian->sigClick.connect(slot(*this, &LangDialog::langChosen));
    }

    // the same: check for Polish localisation
    if (setlocale(LC_ALL, "pl_PL.utf8") &&
        (strcmp(setlocale(LC_ALL, "pl_PL.utf8"), "pl_PL.utf8") == 0))
    {
        r.y += 30;
        d_b_polish = new PG_RadioButton(this, r, "Polish (pl_PL.utf8)", d_b_english);
        d_b_polish->sigClick.connect(slot(*this, &LangDialog::langChosen));
    }

    // for special cases: add a possibility to specify the localisation manually
    r.y += 30;
    d_b_other = new PG_RadioButton(this, r, "Other", d_b_english);
    d_b_other->sigClick.connect(slot(*this, &LangDialog::langChosen));

    r.y += 30;
    d_edit = new PG_LineEdit(this, r, "LineEdit", 20);
    d_edit->sigEditEnd.connect(slot(*this, &LangDialog::localeEdit));
    d_edit->SetText(d_origlang);
    d_edit->SetEditable(false);


    // enable the correct button
    if (strcmp(d_origlang, "C") == 0)
        d_b_english->SetPressed();
    else if (strcmp(d_origlang, "de_DE.utf8") == 0)
        d_b_german->SetPressed();
    else if (strcmp(d_origlang, "it_IT.utf8") == 0)
        d_b_italian->SetPressed();
    else if (strcmp(d_origlang, "pl_PL.utf8") == 0)
        d_b_polish->SetPressed();
    else
    {
        d_b_other->SetPressed();
        d_edit->SetEditable(true);
    }
}

bool LangDialog::langChosen(PG_RadioButton* btn, bool edit)
{
    if (btn == d_b_english)
    {
        strncpy(d_chosenlang, "C", 20);
        setlocale(LC_ALL, "C");
    }
    else if (btn == d_b_german)
    {
        strncpy(d_chosenlang, "de_DE.utf8", 20);
        setlocale(LC_ALL, "de_DE.utf8");
    }
    else if (btn == d_b_italian)
    {
        strncpy(d_chosenlang, "it_IT.utf8", 20);
        setlocale(LC_ALL, "it_IT.utf8");
    }
    else if (btn == d_b_polish)
    {
        strncpy(d_chosenlang, "pl_PL.utf8", 20);
        setlocale(LC_ALL, "pl_PL.utf8");
    }
    else // btn == d_b_other
    {
        d_edit->SetEditable(true);
        setlocale(LC_ALL, d_edit->GetText());
    }

    // update the button texts and such
    d_b_ok->SetText(_("OK"));
    d_b_cancel->SetText(_("Cancel"));

    return true;
}

bool LangDialog::localeEdit()
{
    if (strcmp(setlocale(LC_ALL, d_edit->GetText()), d_edit->GetText()) == 0)
    {
        d_b_ok->SetText(_("OK"));
        d_b_cancel->SetText(_("Cancel"));
    }

    return true;
}

bool LangDialog::okClicked()
{
    QuitModal();

    Configuration::s_lang = std::string(d_chosenlang);

    // this is bad behaviour: save config without having been asked to do so.
    // However, since language changes ar enot _that_ often, the effect should
    // be minimal.
#ifndef __WIN32__
    char* home = getenv("HOME");
    Configuration::saveConfigurationFile(std::string(home) + "/.lordsawarrc");
#else
    Configuration::saveConfigurationFile("lordsawarrc");
#endif
    
    return true;
}

bool LangDialog::cancelClicked()
{
    setlocale(LC_ALL, d_origlang);
    QuitModal();
    
    return true;
}
