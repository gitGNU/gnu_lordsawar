#include "PG_FileDialog.h"
#include <iostream>
#include "defs.h"

#define WIDTH	425
#define HEIGHT	270

using namespace std;

void PG_FileDialog::Init(Type type)
{
	typemode = type;
        selectedFile = "";
	// label for the dir selector
	char* text;
	(type == PG_OPEN)?text = _("Look in:"):text=_("Save in:");
	new PG_Label(this, Rectangle(10, 35, 100, 20), text);

	// dir selector drop down
	my_dirdrop = new PG_DropDown(this, Rectangle(70, 32, WIDTH - 200, 22),PGDDID);
	my_dirdrop->SetEditable(false);
	my_dirdrop->sigSelectItem.connect(slot(*this, &PG_FileDialog::dir_select_handler));

	// file list box
	my_filelistbox = new PG_ListBox(this, Rectangle(10, 65, WIDTH-20, HEIGHT-130));
	my_filelistbox->sigSelectItem.connect(slot(*this, &PG_FileDialog::file_select_handler));

	// labels
	new PG_Label(this, Rectangle(10, HEIGHT-60, 100, 20), _("File name:"));
	new PG_Label(this, Rectangle(10, HEIGHT-30, 100, 20), _("File type:"));
	
	// edit box for filename when selected
	my_filename = new PG_LineEdit(this, Rectangle(100, HEIGHT-60, WIDTH - 230, 22), "LineEdit", 255);

	// file filter select dropdown
	my_filterdrop = new PG_DropDown(this, Rectangle(100, HEIGHT-30, WIDTH - 230, 22),PGDDID);
	my_filterdrop->SetEditable(false);

    // FIXME: to prevent strange bugs: disable the filterdrop
	//my_filterdrop->sigSelectItem.connect(slot(*this, &PG_FileDialog::filter_select_handler));

	// ok.cancle btns
	(type == PG_OPEN)?text = _("Open"):text=_("Save");
	my_btn1 = new PG_Button(this, Rectangle(310, HEIGHT-60, 100, 25), text,PG_BTN_ONE);
	my_closebtn = new PG_Button(this, Rectangle(310, HEIGHT-30, 100, 25), _("Cancel"), PG_BTN_TWO);
	
	// up a dir
	my_updirbtn = new PG_Button(this, Rectangle(320, 32, 25, 25), _("U"), PG_BTN_UP);
	//new dir
	my_newfolderbtn = new PG_Button(this, Rectangle(350, 32, 25, 25), "*", PG_BTN_NEW);

	my_btn1->sigClick.connect(slot(*this, &PG_FileDialog::my_btn1Clicked));
	my_closebtn->sigClick.connect(slot(*this, &PG_FileDialog::my_closebtnClicked));
	my_updirbtn->sigClick.connect(slot(*this, &PG_FileDialog::my_updirbtnClicked));
	my_newfolderbtn->sigClick.connect(slot(*this, &PG_FileDialog::my_newfolderbtnClicked));
}

PG_FileDialog::PG_FileDialog(PG_Widget* parent, Type type, bool modal,
                    string dir, string filter, string filtername, 
                    const char* style)
    :PG_Window(parent, Rectangle(0,0, WIDTH, HEIGHT), (type == PG_OPEN)?_("Open"):_("Save"), (modal)?PG_Window::MODAL : PG_Window::DEFAULT)
{
	// do the common initialisation
	Init(type);

    PG_FileArchive::RemoveAllArchives();

    // the dir needs a "/" at the end
    if (dir.at(dir.length()-1) != '/')
    {
        dir += "/";
    }

    // set the inital dir
    if (SetDir(dir))
    {
	my_dirdrop->AddItem(dir.c_str());
    }
    // dir seems to be invalid => use the default 
    else
    {
        char buffer[101]; buffer[100]='\0';
        snprintf (buffer, 100, _("Directory '%s' does not exist or has wrong permissions\n"), dir.c_str());
        cerr << buffer;
	//set the base dir as the inital dir in the dropdown
	my_dirdrop->AddItem(PG_FileArchive::GetBaseDir());
	//set it as the dir
	SetDir(PG_FileArchive::GetBaseDir());
    }
	
    curfilter = filter;

    // add the inital filter and set it as the default
    AddFilter(filtername, filter, true);
	
}

PG_FileDialog::~PG_FileDialog(void)
{
    //UL: Somehow I get a segfault when deleting my_filelistbox, but only(!!)
    //if I select a file, not if I enter the filename directly. Propably a bug
    //in paragui (is it fixed now?). To make sure, clear the listbox before
    //deleting it.
    my_filelistbox->DeleteAll();
	RemoveAllChilds();
}

void PG_FileDialog::AddFilter(string name, string type, bool defaultf){
	string dd_display(name);
	dd_display +=" [" + string(type) + "]";
	
	my_filterdrop->AddItem(dd_display.c_str());
	
	// if this is to be set as the default filter
	if (defaultf)
        {
	    my_filterdrop->SetText(dd_display.c_str());
	    curfilter = type;
	    eventFilterSelected(NULL);
	}
}

string PG_FileDialog::getSelectedFile()
{
    if (selectedFile.empty()) return "";
    else return (filePath + selectedFile);
}

string PG_FileDialog::getSelectedFilter()
{
	return curfilter;
}

bool PG_FileDialog::SetSelected(const char* filename)
{
	return false;
}

void PG_FileDialog::SelectAll(bool files){
}

const PG_FileList PG_FileDialog::SelectedFiles(void){
	PG_FileList i;
	return i;
}

const char* PG_FileDialog::FilePath(void)
{
	return filePath.c_str();
}

bool PG_FileDialog::SetDir(string dir){
    filePath = dir;
    my_dirdrop->SetText(dir.c_str());
    PG_FileArchive::RemoveAllArchives();
    if (!PG_FileArchive::AddArchive(dir.c_str()))
    {
        PG_LogERR(_("Cannot add directory '%s' to the archive"), dir.c_str());
        return false;
    }
    eventDirEntered(dir.c_str());
    return true;
}

void PG_FileDialog::ShowHiddenFiles(bool show){

}

void PG_FileDialog::RefreshDir(void){
	ListFilesWithFilter();
}

void PG_FileDialog::ResortDir(void){
}

void PG_FileDialog::SetSelectMode(SelectMode mode){
	my_filelistbox->SetMultiSelect(mode);
}

bool  PG_FileDialog::GetSelectMode(void){
	return my_filelistbox->GetMultiSelect();
}

void PG_FileDialog::SetViewMode(ViewMode mode){
	viewmode = mode;
}

int  PG_FileDialog::GetViewMode(void){
	return viewmode;
}

bool PG_FileDialog::eventFileSelected(const char* filename){
	PG_LogDBG("event file: %s", filename);

    if (PG_FileArchive::IsDirectory(filename)){
        //then just move to next level
        string temp = filePath + filename + "/";
        SetDir(temp.c_str());
        ListFilesWithFilter();
    }

	// if we are in multiselect mode then we need to add the files to the list.
	if (my_filelistbox->GetMultiSelect() == MULTISELECT) {
	
	}

	return false;
}

bool PG_FileDialog::eventDirEntered(const char* dirname){
	PG_LogDBG("event dir: %s", dirname);
	// need to add to the archive
	if (!PG_FileArchive::AddArchive(dirname))
		PG_LogERR(_("Cannot add directory '%s' to the archive"), dirname);
		
	return false;
}

bool PG_FileDialog::eventFilterSelected(const char* filter){
	PG_LogDBG("event filter: %s", filter);
	//list the files with the selected filter
	ListFilesWithFilter();

	return false;
}

/*
bool PG_FileDialog::eventButtonClick(int id, PG_Widget* widget) {
  //PG_Window::eventButtonClick(id, widget);
	
	switch(id){
		case PG_BTN_ONE:
            selectedFile = my_filename->GetText();
            if (!selectedFile.empty())
            {
                Hide();
                QuitModal();
                return true;
            }
		break;
		case PG_BTN_TWO:{
			Hide();
			//since this is the 'cancel' button, we will set the selected file to NULL
			//so that the user doesn't accidently use it
			selectedFile.erase();
			QuitModal();
			return true;
			}
		break;
		
		case PG_BTN_UP:{
			int size = filePath.size();
            if (size > 1) {
                filePath[size-1] = '\0';
                int index = filePath.find_last_of("/");

                filePath = filePath.substr(0, index);
                filePath += "/";

                PG_LogDBG(_("index: %d uppath: %s"), index, filePath.c_str());
                SetDir(filePath.c_str());
                ListFilesWithFilter();
            }
			return true;
			}
		break;
		
		case PG_BTN_NEW:{
			PG_LogDBG(_("not implemented yet"));
            return true;
            }
		break;
	}
	
	return false;
	
}
*/

bool PG_FileDialog::my_btn1Clicked(PG_Button* btn) {
	selectedFile = my_filename->GetText();
	if (!selectedFile.empty())
	{
		Hide();
		QuitModal();
		return true;
	}
	return true;
}

bool PG_FileDialog::my_closebtnClicked(PG_Button* btn) {
	Hide();
	//since this is the 'cancel' button, we will set the selected file to NULL
	//so that the user doesn't accidently use it
	selectedFile.erase();
	QuitModal();
	return true;
}

bool PG_FileDialog::my_updirbtnClicked(PG_Button* btn) {
	int size = filePath.size();
	if (size > 1) {
		filePath[size-1] = '\0';
		int index = filePath.find_last_of("/");

		filePath = filePath.substr(0, index);
		filePath += "/";

		PG_LogDBG("index: %d uppath: %s", index, filePath.c_str());
		SetDir(filePath.c_str());
		ListFilesWithFilter();
	}
	return true;
}

bool PG_FileDialog::my_newfolderbtnClicked(PG_Button* btn) {
	PG_LogDBG("not implemented yet");
	return true;
}

void PG_FileDialog::ListFilesWithFilter(){
	my_filelistbox->DeleteAll();
	//iterate the file list and add to the listbox
	PG_FileList* dirlist = PG_FileArchive::GetFileList("/", "*");
	PG_FileList* curdir = PG_FileArchive::GetFileList("/", curfilter.c_str());

	PG_LogDBG("files: %d", curdir->size());

	//list all of the dir first
	for (PG_FileList::iterator itr = dirlist->begin(); itr != dirlist->end(); itr++){
		if (PG_FileArchive::IsDirectory((*itr).c_str())){
		        PG_ListBoxItem *addme = new PG_ListBoxItem(my_filelistbox, 20, (*itr).c_str());
			addme->SetFontColor(PG_Color(150, 150, 255));
		}
	}
	//list only the files
	for (PG_FileList::iterator itr = curdir->begin(); itr != curdir->end(); itr++){
		if (!PG_FileArchive::IsDirectory((*itr).c_str())){
		        (new PG_ListBoxItem(my_filelistbox, 20, (*itr).c_str()));
			//PG_ListBoxItem *addme = new PG_ListBoxItem(20, (*itr).c_str());
			//my_filelistbox->AddItem(addme);
		}
	}
	Update();

        // These take a segmentation faults if not commented
	//delete dirlist;
	//delete curdir;

}

bool PG_FileDialog::eventKeyDown(const SDL_KeyboardEvent* key)
{	
	switch (key->keysym.sym)
	{		
	    case SDLK_ESCAPE:
              my_closebtnClicked(0);
	      break;
            case SDLK_RETURN:
	      my_btn1Clicked(0);
	      break;
	    default:
	      break;
	}
	
	return true;
}

bool PG_FileDialog::filter_select_handler(PG_ListBoxBaseItem *item) {
	// get the file extension
	char *temp = new char;
	strcpy(temp, item->GetText());
	strtok(temp, "[");
	curfilter = strtok(NULL, "]");
	
	eventFilterSelected(curfilter.c_str());
	delete temp;
	return true;
}

bool PG_FileDialog::file_select_handler(PG_ListBoxBaseItem *item) {
    selectedFile = (char*)item->GetText();
    //if a dir is selected, clear the filename box
    if (!PG_FileArchive::IsDirectory(selectedFile.c_str()))
    {
	my_filename->SetText(selectedFile.c_str());
    }
    else
    {
	my_filename->SetText("");
    }
		
    eventFileSelected(selectedFile.c_str());
    return true;
}

bool PG_FileDialog::dir_select_handler(PG_ListBoxBaseItem *item) {
	eventDirEntered(item->GetText());
	return true;
}


