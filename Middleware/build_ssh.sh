set -e


ROOT_DIR=$(pwd)
BASE_DIR=${ROOT_DIR}

SSH_DIR=${BASE_DIR}/openssh-portable

cd ${SSH_DIR}

autoreconf
./configure --without-zlib-version-check
make