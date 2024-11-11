set -e

BASE_DIR=$(pwd)
PROJECT_DIR=${BASE_DIR}/..

BUILD_DIR=${BASE_DIR}/build
OUTPUT_DIR=${BASE_DIR}/output

mkdir -p ${BUILD_DIR}
mkdir -p ${OUTPUT_DIR}

CODEGEN_DIR=${BASE_DIR}/codegen
GDBUS_DIR=${BASE_DIR}/gdbus/
cd ${GDBUS_DIR}
${CODEGEN_DIR}/generate --generate-c-code=interface --interface-prefix=com.example --c-generate-object-manager interface.xml
${CODEGEN_DIR}/generate --generate-c-code=video --interface-prefix=com.example video.xml
${CODEGEN_DIR}/generate --generate-c-code=photo --interface-prefix=com.example photo.xml

cp ${BASE_DIR}/rpc/rpc_middleware.h ${OUTPUT_DIR}/include

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${OUTPUT_DIR}/lib/x86_64-linux-gnu
cd ${BUILD_DIR}
cmake ..
make
make install

cp ${BUILD_DIR}/middleware ${OUTPUT_DIR}/
# cp ${BUILD_DIR}/gdbus/client ${OUTPUT_DIR}/
# cp ${BUILD_DIR}/gdbus/server ${OUTPUT_DIR}/

cp ${BUILD_DIR}/gdbus/video ${OUTPUT_DIR}/
cp ${BUILD_DIR}/gdbus/photo ${OUTPUT_DIR}/

rm -rf ${OUTPUT_DIR}/config_mgmt.db

cd ${OUTPUT_DIR}
echo
echo
./middleware

# ./video
# ./client &
# sleep 10
# killall server