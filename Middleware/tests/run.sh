set -e

BASE_DIR=$(pwd)
PROJECT_DIR=${BASE_DIR}/..

BUILD_DIR=${BASE_DIR}/build
OUTPUT_DIR=${BASE_DIR}/output

mkdir -p ${BUILD_DIR}
mkdir -p ${OUTPUT_DIR}

GDB_FILE_NAME=interface
./codegen/generate --generate-c-code=gdbus/${GDB_FILE_NAME} --interface-prefix=com.example --c-generate-object-manager ${GDB_FILE_NAME}.xml
cp ${BASE_DIR}/rpc/rpc_middleware.h ${OUTPUT_DIR}/include

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${OUTPUT_DIR}/lib/x86_64-linux-gnu

cd ${BUILD_DIR}
cmake ..
make
make install

cp ${BUILD_DIR}/middleware ${OUTPUT_DIR}/
cp ${BUILD_DIR}/gdbus/client ${OUTPUT_DIR}/
cp ${BUILD_DIR}/gdbus/server ${OUTPUT_DIR}/

rm -rf ${OUTPUT_DIR}/test.db

cd ${OUTPUT_DIR}
echo
echo
echo "Run the dbus_test:"

# ./middleware

./server
# ./client &
# sleep 10
# killall server