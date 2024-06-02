
set -e

ROOT_DIR=$(pwd)
BASE_DIR=${ROOT_DIR}

GLIB_DIR=${BASE_DIR}/glib
SQLITE_DIR=${BASE_DIR}/sqlite
CJSON_DIR=${BASE_DIR}/cJSON
TEST_DIR=${BASE_DIR}/tests
OUTPUT_DIR=${TEST_DIR}/output

rm -rf ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}

cd ${GLIB_DIR}
git reset --hard
git clean -dfx
rm -rf _build
mkdir -p _build
cd _build
meson setup .. --prefix=${OUTPUT_DIR} --buildtype=release
ninja
ninja install

cd ${SQLITE_DIR}
git reset --hard
git clean -dfx
rm -rf ${SQLITE_DIR}/build
mkdir -p ${SQLITE_DIR}/build
cd ${SQLITE_DIR}/build
../configure prefix=${OUTPUT_DIR}/
time make -j$(nproc)
make install
rm -rf ${SQLITE_DIR}/build

cd ${CJSON_DIR}
git reset --hard
git clean -dfx
mkdir -p ${CJSON_DIR}/build
cd ${CJSON_DIR}/build
cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=Off -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
time make -j$(nproc)
make install

