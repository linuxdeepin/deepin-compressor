#执行测试程序
#!/bin/bash

project_path=$(cd `dirname $0`; pwd)
executable1=${project_path}/test_output/cli7zplugin_test #可执行程序的文件名
executable2=${project_path}/test_output/clirarplugin_test
executable3=${project_path}/test_output/cliunarchiverplugin_test
executable4=${project_path}/test_output/lbarchive_test
executable5=${project_path}/test_output/lreadwritelibarchiveplugin_test
executable6=${project_path}/test_output/readonlylibarchiveplugin_test
executable7=${project_path}/test_output/libzipplugin_test
executable8=${project_path}/test_output/common_test
executable9=${project_path}/test_output/interface_test
executable10=${project_path}/test_output/deepin-compressor_test

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

exit 0
