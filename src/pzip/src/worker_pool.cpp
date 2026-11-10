// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/worker_pool.h"
#include "pzip/file_task.h"
#include "pzip/extractor.h"

namespace pzip {

template class WorkerPool<FileTask>;
template class WorkerPool<ExtractTask>;

} // namespace pzip

