#include <libtar.h>
#include <string>
#include <iostream>
#include <list>
class Tar_Helper
{
public:

    //! Constructor
    Tar_Helper(std::string file, std::ios::openmode mode, bool &broken);

    //! Destructor
    ~Tar_Helper();

    bool saveFile(std::string file, std::string destfile = "");

    std::string getFile(std::string filename, bool &broken);

    std::string getFirstFile(bool &broken);
    std::string getFirstFile(std::string extension, bool &broken);

    std::list<std::string> getFilenamesWithExtension(std::string ext);

    std::list<std::string> getFilenames();


    bool removeFile(std::string filename);
    bool replaceFile(std::string old_filename, std::string new_filename);

    bool Open(std::string file, std::ios::openmode mode);
    void Close();

    static bool is_tarfile (std::string file);

    static std::string getFile(TAR *t, std::string filename, bool &broken, std::string tmpoutdir);
    static std::list<std::string> getFilenames(TAR *t);
    static bool saveFile(TAR *t, std::string filename, std::string destfile = "");
    //copies a tar file to another place, renaming one of the files inside.
    static bool copy(std::string filename, std::string newfilename);
    static void clean_tmp_dir(std::string filename);
private:

    // DATA
    TAR *t;
    std::ios::openmode openmode;
    std::string tmpoutdir;
};
