
/**
 * @brief define the const id or string
 */

#ifndef EXTRACTRES_H
#define EXTRACTRES_H

#include <QMap>
#include <QString>

#define IDS_IS7Z 1001           //the archive is 7Z
#define IDS_ISZIP 1002          //the archive is zip
#define IDS_ISRAR 1003          //the archive is rar
#define IDS_PHYSICSIZE 1102
#define IDS_WRONGPSDERROR 1103  //the extract password is wrong
#define IDS_EVERYTHINGISOK 1104 //the extract password is right

#define IDS_RAR_ERRORPSD 1105
#define IDS_RAR_WRONGPSD 1106
#define IDS_RAR_ALLOK 1107

//#define IDS_REPLACETIP 1108     //Would you like to replace the existing file

#define QS(cstr) QString(cstr)

static const QMap<unsigned int,QString> GlobalStrTable = {
    { IDS_IS7Z,QS("Type = 7z")},
    { IDS_ISZIP,QS("Type = zip")},
    { IDS_ISRAR,QS("UNRAR")},

    { IDS_PHYSICSIZE,QS("Physical Size")},
    { IDS_WRONGPSDERROR ,QS("Wrong password") },
    { IDS_EVERYTHINGISOK,QS("Everything is Ok") },
    { IDS_RAR_ERRORPSD, QS("password is incorrect")},
    { IDS_RAR_WRONGPSD, QS("wrong password")},
    { IDS_RAR_ALLOK, QS("All OK")},
//    { IDS_REPLACETIP, QS("Would you like to replace the existing file")}
};


/**
  * the const IDstr for extract tool name
  * */
#define EXTRACT_TOOL_7Z "tool7z"
#define EXTRACT_TOOL_RAR "toolRar"
#define EXTRACT_TOOL_ZIP "toolZip"


#define EXTRACT_REPLACE_TIP "Would you like to replace the existing file"


#endif // EXTRACTRES_H
