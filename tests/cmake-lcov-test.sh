utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16

./bin/tests/bz2plugin_test
./bin/tests/cli7zplugin_test
./bin/tests/clirarplugin_test
./bin/tests/deepin-compressor_test
./bin/tests/gzplugin_test
./bin/tests/interface_test
./bin/tests/libarchive_test
./bin/tests/libminizipplugin_test
./bin/tests/libzipplugin_test
./bin/tests/readonlylibarchiveplugin_test
./bin/tests/readwritelibarchiveplugin_test
./bin/tests/singlefile_test
./bin/tests/xzplugin_test

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

mkdir -p report
lcov -d $workdir -c -o ./report/coverage.info

#只收集src、3rdparty部分目录
lcov --extract ./report/coverage.info '*/src/*' '*/3rdparty/*' -o ./report/coverage.info
#过滤 tests 3rdparty/ChardetDetector目录
lcov --remove ./report/coverage.info '*/tests/*' '*/ChardetDetector/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
