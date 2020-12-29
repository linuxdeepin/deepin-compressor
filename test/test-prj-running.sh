#执行测试程序
#!/bin/bash

project_path=$(cd `dirname $0`; pwd)
executable1=${project_path}/test_output/bz2plugin_test #可执行程序的文件名
executable2=${project_path}/test_output/cli7zplugin_test
executable3=${project_path}/test_output/clirarplugin_test
executable4=${project_path}/test_output/deepin-compressor_test
executable5=${project_path}/test_output/gzplugin_test
executable6=${project_path}/test_output/interface_test
executable7=${project_path}/test_output/libarchive_test
executable8=${project_path}/test_output/libminizipplugin_test
executable9=${project_path}/test_output/libzipplugin_test
executable10=${project_path}/test_output/readonlylibarchiveplugin_test
executable11=${project_path}/test_output/readwritelibarchiveplugin_test
executable12=${project_path}/test_output/singlefile_test
executable13=${project_path}/test_output/xzplugin_test

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
