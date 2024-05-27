
set -e

ROOT_DIR=$(pwd)
BASE_DIR=${ROOT_DIR}

GLIB_DIR=${BASE_DIR}/glib
SQLITE_DIR=${BASE_DIR}/sqlite
CJSON_DIR=${BASE_DIR}/cJSON

cd ${GLIB_DIR}
git reset --hard
git clean -dfx
rm -rf _build
meson _build
time ninja -C _build

cd ${SQLITE_DIR}
git reset --hard
git clean -dfx
rm -rf ${SQLITE_DIR}/build
mkdir -p ${SQLITE_DIR}/build
cd ${SQLITE_DIR}/build
../configure prefix=${SQLITE_DIR}/
time make -j$(nproc)

cd ${CJSON_DIR}
git reset --hard
git clean -dfx
mkdir -p ${CJSON_DIR}/build
cd ${CJSON_DIR}/build
cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=Off
time make -j$(nproc)
