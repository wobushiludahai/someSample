
set -e

ROOT_DIR=$(pwd)
BASE_DIR=${ROOT_DIR}

LOGROTATE_DIR=${BASE_DIR}/logrotate
POPT_DIR=${BASE_DIR}/POPT



TEST_DIR=${BASE_DIR}/tests
OUTPUT_DIR=${TEST_DIR}/output

rm -rf ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}

cd ${POPT_DIR}
./autogen.sh
./configure --prefix=${OUTPUT_DIR}
make -j
make install

cd ${LOGROTATE_DIR}
./autogen.sh
./configure --prefix=${OUTPUT_DIR} LDFLAGS="-L${OUTPUT_DIR}/lib" CPPFLAGS="-I${OUTPUT_DIR}/include"
make -j
make install
