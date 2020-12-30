/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <gaoxiang@uniontech.com>
*
* Maintainer: chendu <gaoxiang@uniontech.com>
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

#include "libminizipplugin.h"
#include "gtest/src/stub.h"
#include "queries.h"
#include "datamanager.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QDebug>

Q_DECLARE_METATYPE(KPluginMetaData)
