#ifndef ARCHIVEVIEWSTATE_H
#define ARCHIVEVIEWSTATE_H

#include "archiveui_types.h"

class ArchiveViewState
{
public:
    static ArchiveViewState &instance()
    {
        static ArchiveViewState state;
        return state;
    }
    void resetArchiveData() { m_data.reset(); }
    ArchiveData &archiveData() { return m_data; }

private:
    ArchiveData m_data;
};

#endif
