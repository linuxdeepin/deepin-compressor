/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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

#ifndef COMPRESSORAPPLICATION_H
#define COMPRESSORAPPLICATION_H

#include <DApplication>

DWIDGET_USE_NAMESPACE

// 应用程序
class CompressorApplication: public DApplication
{
    Q_OBJECT
public:
    CompressorApplication(int &argc, char **argv);
    bool notify(QObject *watched, QEvent *event) override;
};

#endif // COMPRESSORAPPLICATION_H
