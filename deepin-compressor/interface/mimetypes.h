#ifndef MIMETYPES_H
#define MIMETYPES_H

#include <QMimeType>


    /**
     * @param filename Absolute path of a file.
     * @return The mimetype of the given file.
     */
     QMimeType determineMimeType(const QString& filename);

#endif // MIMETYPES_H
