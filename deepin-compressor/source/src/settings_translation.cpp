#include <DSettings>

void GenerateSettingTranslate()
{
    auto setting_Text = QObject::tr("General");  //设置
    auto decompress = QObject::tr("Extraction"); //解压
    auto create_folder = QObject::tr("Auto create a folder for multiple extracted files");  //自动创建文件夹
    auto open_folder = QObject::tr("Show extracted files when complete");  //当解压完成后自动打开对应的文件夹
    auto file_association = QObject::tr("Files Associated");  //关联文件
    auto file_type = QObject::tr("File Type");  //文件类型
}
