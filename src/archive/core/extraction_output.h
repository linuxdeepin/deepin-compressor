// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTRACTION_OUTPUT_H
#define EXTRACTION_OUTPUT_H

#include "archive_types.h"

#include <QByteArray>

#include <sys/types.h>

namespace dcarchive {

// Publishes one extracted regular file atomically without forcing disk writeback.
class ExtractionOutput
{
public:
    ExtractionOutput() = default;
    ~ExtractionOutput();
    Q_DISABLE_COPY(ExtractionOutput)

    bool open(const QString &path);
    bool write(const char *data, qint64 size);
    bool commit();
    bool keepIncompleteFile();
    ArchiveError error() const { return m_error; }

private:
    bool fail(int error);

    int m_parent = -1;
    int m_file = -1;
    QByteArray m_name;
    QByteArray m_temporary;
    mode_t m_mode = 0;
    bool m_preserveMode = false;
    ArchiveError m_error = ArchiveError::None;
};

} // namespace dcarchive

#endif // EXTRACTION_OUTPUT_H
