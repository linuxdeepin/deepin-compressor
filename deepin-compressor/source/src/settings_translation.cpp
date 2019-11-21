#include <DSettings>

void GenerateSettingTranslate()
{
    auto setting_Text = QObject::tr("Setting");  //设置
    auto decompress = QObject::tr("Extraction"); //解压
    auto create_folder = QObject::tr("Create folder automatically");  //自动创建文件夹
    auto open_folder = QObject::tr("Corresponding folder is automatically opened");  //当解压完成后自动打开对应的文件夹
    auto file_association = QObject::tr("File association");  //关联文件
    auto file_type = QObject::tr("File type");  //文件类型
}
