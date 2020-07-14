#ifndef GLOBALARCHIVEMANAGER_H
#define GLOBALARCHIVEMANAGER_H

#include "TSSingleton.h"
#include "archive_manager.h"

class GlobalArchiveManager : public TSSingleton<GlobalArchiveManager>
{
    SINGLETON_DECLARE(GlobalArchiveManager);
public:
    GlobalArchiveManager();
    ~GlobalArchiveManager();

public:
   Archive * getCurrentArchive();
   void setCurrentArchive(Archive *archive);

   ReadOnlyArchiveInterface *getCurrentPlugin();
private:
   Archive* m_Archive;

};

#endif // GLOBALARCHIVEMANAGER_H
