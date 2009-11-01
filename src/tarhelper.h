#include <libtar.h>
#include <string>
#include <iostream>
#include <list>
class Tar_Helper
{
public:

    //! Constructor
    Tar_Helper(std::string file, std::ios::openmode mode);

    //! Destructor
    ~Tar_Helper();

    bool saveFile(std::string file, std::string destfile = "");

    std::string getFile(std::string filename, bool &broken);

    std::string getFirstFile(bool &broken);

    std::list<std::string> getFilenamesWithExtension(std::string ext);

    std::list<std::string> getFilenames();

    void Close();
private:

    // DATA
    TAR *t;
    std::ios::openmode openmode;
    std::string tmpoutdir;
};
