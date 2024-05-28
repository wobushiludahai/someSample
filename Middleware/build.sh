
set -e

ROOT_DIR=$(pwd)
BASE_DIR=${ROOT_DIR}

GLIB_DIR=${BASE_DIR}/glib
SQLITE_DIR=${BASE_DIR}/sqlite
CJSON_DIR=${BASE_DIR}/cJSON
TEST_DIR=${BASE_DIR}/tests
TEST_INCLUDE_DIR=${TEST_DIR}/test_include
RUN_DIR=${TEST_DIR}/output/lib

mkdir -p ${RUN_DIR}
mkdir -p ${TEST_INCLUDE_DIR}

# cd ${GLIB_DIR}
# git reset --hard
# git clean -dfx
# rm -rf _build
# meson _build
# time ninja -C _build

cd ${SQLITE_DIR}
git reset --hard
git clean -dfx
rm -rf ${SQLITE_DIR}/build
mkdir -p ${SQLITE_DIR}/build
cd ${SQLITE_DIR}/build
../configure prefix=${SQLITE_DIR}/
time make -j$(nproc)
SQLITE_INC=${TEST_INCLUDE_DIR}/sqlite
mkdir -p ${SQLITE_INC}
cp ${SQLITE_DIR}/build/*.h ${SQLITE_INC}
cp -d ${SQLITE_DIR}/build/.libs/libsqlite3.so* ${RUN_DIR}
# rm -rf ${SQLITE_DIR}/build

cd ${CJSON_DIR}
git reset --hard
git clean -dfx
mkdir -p ${CJSON_DIR}/build
cd ${CJSON_DIR}/build
cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=Off
time make -j$(nproc)
CJSON_INC=${TEST_INCLUDE_DIR}/cJSON
mkdir -p ${CJSON_INC}
cp ${CJSON_DIR}/*.h ${CJSON_INC}
cp -d ${CJSON_DIR}/build/libcjson.so* ${RUN_DIR}
rm -rf ${CJSON_DIR}/build
