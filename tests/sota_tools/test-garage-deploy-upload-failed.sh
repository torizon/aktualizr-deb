#!/bin/bash
set -euo pipefail
trap 'kill %1' EXIT

TEMP_DIR=$(mktemp -d)

PORT=$(python3 -c 'import socket; s=socket.socket(); s.bind(("", 0)); print(s.getsockname()[1]); s.close()')
TREEHUB="{\
  \"ostree\": {\
    \"server\": \"http://localhost:$PORT/\"\
  }\
}"

TREEHUB_DEST="{\
  \"ostree\": {\
    \"server\": \"http://localhost:1/\"\
  }\
}"

echo "$TREEHUB" > "$TEMP_DIR"/treehub.json
echo "$TREEHUB_DEST" > "$TEMP_DIR"/treehub_dest.json

./tests/sota_tools/treehub_server.py -p"${PORT}" -c  &
until curl -I localhost:"${PORT}" 2>/dev/null; do sleep 0.5; done

$1 --commit b9ac1e45f9227df8ee191b6e51e09417bd36c6ebbeff999431e3073ac50f0563 -f "$TEMP_DIR"/treehub.json -p "$TEMP_DIR"/treehub_dest.json --name testname -h hwids
exit_code=$?
exit $exit_code
