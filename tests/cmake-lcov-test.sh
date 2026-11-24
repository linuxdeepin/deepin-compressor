# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir

cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4

./bin/tests/archive_contract_test --gtest_output=xml:./report/report_archive_contract.xml
./bin/tests/deepin-compressor_test --gtest_output=xml:./report/report_deepin-compressor.xml

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

mkdir -p report
lcov -d $workdir -c -o ./report/coverage.info

#只收集src部分目录
lcov --extract ./report/coverage.info '*/src/*' -o ./report/coverage.info
lcov --remove ./report/coverage.info '*/tests/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
