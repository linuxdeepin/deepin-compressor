/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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

#ifndef KPROCESS_P_H
#define KPROCESS_P_H

#include "kprocess.h"

class KProcessPrivate
{
    Q_DECLARE_PUBLIC(KProcess)
protected:
    KProcessPrivate(KProcess *q) :
        openMode(QIODevice::ReadWrite),
        q_ptr(q)
    {
    }

    QString prog;
    QStringList args;
    QIODevice::OpenMode openMode;

    KProcess *q_ptr;
};

#endif
