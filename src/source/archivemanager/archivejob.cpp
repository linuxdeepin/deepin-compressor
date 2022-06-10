/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "archivejob.h"

ArchiveJob::ArchiveJob(QObject *parent)
    : QObject(parent)
{

}

ArchiveJob::~ArchiveJob()
{

}

void ArchiveJob::kill()
{
    if (doKill()) {
        // 设置结束标志
        m_eFinishedType = PFT_Cancel;
        m_eErrorType = ET_UserCancelOpertion;
        finishJob();
    }
}

bool ArchiveJob::status()
{
    return true;
}

void ArchiveJob::finishJob()
{
    emit signalJobFinshed();
    deleteLater();
}

bool ArchiveJob::doKill()
{
    return false;
}
