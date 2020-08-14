#include "globalarchivemanager.h"

SINGLETON_IMPLEMENT(GlobalArchiveManager);

GlobalArchiveManager::GlobalArchiveManager()
    : m_Archive(nullptr)
{
}

GlobalArchiveManager::~GlobalArchiveManager()
{
}

Archive *GlobalArchiveManager::getCurrentArchive()
{
    return m_Archive;
}

void GlobalArchiveManager::setCurrentArchive(Archive *archive)
{
    m_Archive = archive;
}

ReadOnlyArchiveInterface *GlobalArchiveManager::getCurrentPlugin()
{
    if (!m_Archive)
        return nullptr;
    return m_Archive->interface();
}
