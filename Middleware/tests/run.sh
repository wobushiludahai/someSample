set -e

BASE_DIR=$(pwd)
PROJECT_DIR=${BASE_DIR}/..

BUILD_DIR=${BASE_DIR}/build
OUTPUT_DIR=${BASE_DIR}/output

mkdir -p ${BUILD_DIR}
mkdir -p ${OUTPUT_DIR}

cd ${BUILD_DIR}
cmake ..
make

cp ${BUILD_DIR}/middleware ${OUTPUT_DIR}/
cp ${BUILD_DIR}/gdbus/client ${OUTPUT_DIR}/
cp ${BUILD_DIR}/gdbus/server ${OUTPUT_DIR}/

rm -rf ${OUTPUT_DIR}/test.db

cd ${OUTPUT_DIR}
echo
echo
echo "Run the middleware:"

./server &
./client &
sleep 10
killall server
killall client