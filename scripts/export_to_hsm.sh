#! /bin/bash
set -euo pipefail

TEST_PKCS11_MODULE_PATH=${TEST_PKCS11_MODULE_PATH:-/usr/lib/softhsm/libsofthsm2.so}
TOKEN_DIR=${TOKEN_DIR:-/var/lib/softhsm/tokens}
SOFTHSM2_CONF=${SOFTHSM2_CONF:-/etc/softhsm/softhsm2.conf}

sed -i "s:^directories\\.tokendir = .*$:directories.tokendir = ${TOKEN_DIR}:" "${SOFTHSM2_CONF}"
mkdir -p "${TOKEN_DIR}"

softhsm2-util --init-token --slot 0 --label "Virtual token" --pin 1234 --so-pin 1234
SLOT=$(softhsm2-util --show-slots | grep -m 1 -oP 'Slot \K[0-9]+')
echo "Initialized token in slot: $SLOT"

openssl x509 -outform der -in "${CERTS_DIR}/client.pem" -out "${CERTS_DIR}/client.der"
pkcs11-tool --module="${TEST_PKCS11_MODULE_PATH}" --id 1 --write-object "${CERTS_DIR}/client.der" --type cert --login --pin 1234

openssl pkcs8 -topk8 -inform PEM -outform PEM -nocrypt -in "${CERTS_DIR}/pkey.pem" -out "${CERTS_DIR}/priv.p8"
softhsm2-util --import "${CERTS_DIR}/priv.p8" --label "uptane" --id 02 --slot "$SLOT" --pin 1234
