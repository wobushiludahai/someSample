
set -e

ROOT_DIR=$(pwd)
BASE_DIR=${ROOT_DIR}

GLIB_DIR=${BASE_DIR}/glib
SQLITE_DIR=${BASE_DIR}/sqlite
CJSON_DIR=${BASE_DIR}/cJSON

cd ${GLIB_DIR}
meson _build
time ninja -C _build

CD