#!/bin/sh -ex

# This script will download the latest SQLCipher and create
# the amalgamated sqlite3.c and sqlite3.h files

REPO="sqlcipher/sqlcipher"
URL=$(curl -s "https://api.github.com/repos/${REPO}/releases/latest" \
  | grep '"tarball_url":' \
  | cut -d '"' -f 4)

cd `mktemp -d`
curl -L "$URL" -o source.tar.gz
SRC_DIR=$(tar tzf source.tar.gz | head -1 | cut -f1 -d"/")
tar xvzf source.tar.gz

./$SRC_DIR/configure \
  --with-tempstore=yes \
  CFLAGS="-DSQLCIPHER_CRYPTO_LIBTOMCRYPT \
          -DSQLITE_HAS_CODEC \
          -DSQLITE_EXTRA_INIT=sqlcipher_extra_init \
          -DSQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown"

make sqlite3.c

cp -v sqlite3.c sqlite3.h ${OLDPWD}/Sources/DataLiteC/sqlcipher
