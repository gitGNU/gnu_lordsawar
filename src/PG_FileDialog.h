#ifndef PG_FILESELECTOR
#define PG_FILESELECTOR

#include "pglistbox.h"
#include "pgbutton.h"
#include "pgdropdown.h"
#include "pgwidget.h"
#include "pglineedit.h"
#include "pgwindow.h"
#include "pgfilearchive.h"
#include "pgcolumnitem.h"
#include "pglog.h"
#include <list>
#include <string>

/*!	\class PG_FileDialog

	The PG_FileSelector class provides dialogs that allow users to select files or directories.
	You will be able to select one or many files from a directory.
	
	As the filesystem structure as seen by paragui is though the PhysFS library, we will be limiting
	the seen directories to only those the programmer inatiates using the PG_FileArchive::AddArchive.
	
	as such, only the writable directories will been seen by the save fileselector.
	
	There will be the ability to create a new directory in both open and save mode. If in open mode, 
	the directory will NOT be set to writable, if in save mode, it will. 
	
	Also, PhysFS allows zip and other archives to be seen as 'directories'. The will be the option
	of setting zip files to be viewed as files or directories, the default will be as directories.
	
	\bug PG_DropwDown still has a bug when running in modal mode, thus, there is a problem when
	\bug running the file selection in modal mode.

*/

/* FIXME: I have disabled the filter selection because it yields strange segfaults
 * To see them, uncomment the FIXME line in the constructor, select another filter
 * and abort the dialog.
 */

class DECLSPEC PG_FileDialog: public PG_Window
{
    public:
        enum Type{ PG_OPEN, PG_SAVE };

	//!constructor
	/*!
		@param parent the parent widget
		@param initaldir the inital directory to open the fileselector into
		@param initalfilter the inital filter to use on the dir
		@param filtername the name of the inital filter
		@param type either open or save fileselector
		@param modal is the fileselector going to be run modal
		@param style the theme style to load from the themefile
	*/
	PG_FileDialog(PG_Widget* parent = 0, Type type = PG_OPEN, bool modal = false,
            std::string dir = 0, std::string filter = "*",
            std::string filtername = "All", const char* style = "FileSelector");

	//! destructor
	~PG_FileDialog();
	
	//! add a new extension filter
	/*
		add a new file extension to sort by
		@param	name the name type of the file
		@param type extension type without the (.)
		@param defaultf set this extension to be the default; the one that is set when the dialogue is shown
	*/
	void AddFilter(std::string name, std::string type, bool defaultf=false );

	//! get the name of the selected file
	/*!
		@return the name of the currently selected file		
		
		This should be used after the dialogue is closed.
	*/
        std::string getSelectedFile();

	//! get the current selected file filter ie - *.mp3
	/*!
		@return the file extension of the currently selected filter
	*/
        std::string getSelectedFilter();
	//! sets the selected file
	/*!
		will set the selected file to \a filename in the current dir. if the file doesn't exist
		then nothing is selected
		
		@param filename the name of the file to select
		@returns false if file doesn't exist
	*/
	bool SetSelected(const char* filename);
	//! selects/de-selectes all of the files in the current dir
	/*!
		@param files if true, then select, otw de-select
	*/
	void SelectAll(bool files = true);
	//! returns a list of all the selected files
	/*!
		@return a PG_FileList of all selected files
		
		the user is responsible for the deletion of the list and everything in it.		
	*/
	const PG_FileList SelectedFiles(void);
	//! get the current filepath
	/*!
		@return the current filepath
	*/
	const char* FilePath(void);

	//! set the dir
	/*! @param dir the dir to set to
		
		set the current browsing dir
        return true on success
	*/
	bool SetDir(std::string dir);
	//! show or hide hidden file
	/*!
		@param show true if you want to see files with hidden attrib
	*/
	void ShowHiddenFiles(bool show=true);
	//! refresh the current dir
	void RefreshDir(void);
	//! resort the dir - used in conjunction with RefreshDir
	void ResortDir(void);

	/*!
		\enum ViewMode
		The method to view files 
	*/
	enum ViewMode{
		//! show name, size, attribs
		Detail,
		//! show only name
		List
	};

	/*!
		\enum SelectMode
	*/
	enum SelectMode{
		//!single file select
		SINGLESELECT = 0,
		//!multifile select
		MULTISELECT = 1
	};
	
	//! set the file select mode
	/*!
		@param mode the mode of selection
	*/
	void SetSelectMode(SelectMode mode);
	//! get the select mode
	/*!
		@return the SelectMode
	*/
	bool GetSelectMode();
	//! set the view mode
	/*!
		@param mode the view mode
	*/

	void SetViewMode(ViewMode mode);
	//! get the view mode
	/*!
		@return the view mode
	*/
	int GetViewMode(void);
	
protected:
	//! file selected event
	/*!
		@param filename name of current filename selected
	*/
	virtual bool eventFileSelected(const char* filename);
	//! dir select event
	/*!
		@param dirname name of current dirname selected
	*/
	virtual bool eventDirEntered(const char* dirname);
	//! filter select event
	/*!
		@param filter name of current filter selected
	*/
	virtual bool eventFilterSelected(const char* filter);
	
	//bool eventButtonClick(int id, PG_Widget* widget);

	bool my_btn1Clicked(PG_Button *btn);
	bool my_closebtnClicked(PG_Button *btn);
	bool my_updirbtnClicked(PG_Button *btn);
	bool my_newfolderbtnClicked(PG_Button *btn);

	bool filter_select_handler(PG_ListBoxBaseItem *item);
	bool file_select_handler(PG_ListBoxBaseItem *item);
	bool dir_select_handler(PG_ListBoxBaseItem *item);

private:
	//! constructor helper
	void Init( Type type);
	
	//! listing the files helper
	void ListFilesWithFilter(void);
	
	// Event handler 
	bool eventKeyDown(const SDL_KeyboardEvent* key);

	PG_ListBox	 	*my_filelistbox;
	PG_Button 		*my_btn1;
	PG_Button 		*my_closebtn;
	PG_Button		*my_updirbtn;
	PG_Button		*my_newfolderbtn;
	PG_DropDown		*my_dirdrop;
	PG_DropDown		*my_filterdrop;
	PG_LineEdit		*my_filename;
	
        std::string selectedFile;
        std::string filePath;
        std::string curfilter;
	ViewMode 		viewmode;
	Type			typemode;
	enum btnids{
		PG_BTN_ONE 	=1,
	 	PG_BTN_TWO 	=2,
 		PG_BTN_UP	=3,
	 	PG_BTN_NEW 	=4,
 		PGDDID		=9965
	};
};

#endif

// End of file
