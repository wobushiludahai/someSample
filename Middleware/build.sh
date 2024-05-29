
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

cd ${GLIB_DIR}
git reset --hard
git clean -dfx
rm -rf _build
meson _build
time ninja -C _build
GLIB_INC=${TEST_INCLUDE_DIR}/glib-2.0
mkdir -p ${GLIB_INC}
cp ${GLIB_DIR}/gmodule/gmodule.h ${GLIB_INC}/

mkdir -p ${GLIB_INC}/gio
cp ${GLIB_DIR}/gio/*.h ${GLIB_INC}/gio

mkdir -p ${GLIB_INC}/gobject
cp ${GLIB_DIR}/gobject/*.h ${GLIB_INC}/gobject

mkdir -p ${GLIB_INC}/glib
mkdir -p ${GLIB_INC}/glib/deprecated
cp ${GLIB_DIR}/glib/*.h ${GLIB_INC}/glib
cp ${GLIB_DIR}/glib/deprecated/*.h ${GLIB_INC}/glib/deprecated

mv ${GLIB_INC}/glib/glib.h ${GLIB_INC}/
mv ${GLIB_INC}/glib/glib-object.h ${GLIB_INC}/
mv ${GLIB_INC}/glib/glib-unix.h ${GLIB_INC}/

cp ${GLIB_DIR}/_build/glib/libglib-2.0.so ${RUN_DIR}
cp ${GLIB_DIR}/_build/glib/libglib-2.0.so.0 ${RUN_DIR}
cp ${GLIB_DIR}/_build/glib/libglib-2.0.so.0.7200.3 ${RUN_DIR}

cp -d ${GLIB_DIR}/_build/gio/libgio-2.0.so ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gio/libgio-2.0.so.0 ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gio/libgio-2.0.so.0.7200.3 ${RUN_DIR}

cp -d ${GLIB_DIR}/_build/gmodule/libgmodule-2.0.so ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gmodule/libgmodule-2.0.so.0 ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gmodule/libgmodule-2.0.so.0.7200.3 ${RUN_DIR}

cp -d ${GLIB_DIR}/_build/gobject/libgobject-2.0.so ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gobject/libgobject-2.0.so.0 ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gobject/libgobject-2.0.so.0.7200.3 ${RUN_DIR}

cp -d ${GLIB_DIR}/_build/gthread/libgthread-2.0.so ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gthread/libgthread-2.0.so.0 ${RUN_DIR}
cp -d ${GLIB_DIR}/_build/gthread/libgthread-2.0.so.0.7200.3 ${RUN_DIR}

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
rm -rf ${SQLITE_DIR}/build

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
