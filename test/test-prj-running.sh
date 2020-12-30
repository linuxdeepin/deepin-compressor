#执行测试程序
#!/bin/bash

project_path=$(cd `dirname $0`; pwd)    #当前Shell程序的目录
cd $project_path/test_output
executable1=./bz2plugin_test #可执行程序的文件名
executable2=./cli7zplugin_test
executable3=./clirarplugin_test
executable4=./deepin-compressor_test
executable5=./gzplugin_test
executable6=./interface_test
executable7=./libarchive_test
executable8=./libminizipplugin_test
executable9=./libzipplugin_test
executable10=./readonlylibarchiveplugin_test
executable11=./readwritelibarchiveplugin_test
executable12=./singlefile_test
executable13=./xzplugin_test

$executable1
$executable2
$executable3
$executable4
$executable5
$executable6
$executable7
$executable8
$executable9
$executable10
$executable11
$executable12
$executable13

exit 0
