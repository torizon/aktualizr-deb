#!/bin/bash
set -u

$3 "$1" --ref master --repo sota_tools/repo --credentials "$2" --cacert sota_tools/testcerts.crt -n 2>&1 | grep -q ca-certificates
test $? -eq 1
