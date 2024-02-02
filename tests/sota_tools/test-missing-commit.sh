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

echo "$TREEHUB" > "$TEMP_DIR"/treehub.json
./tests/sota_tools/treehub_server.py --tls -p"${PORT}" -d"${TEMP_DIR}" &
sleep 1

TARGET="OSTree commit 16ef2f2629dc9263fdf3c0f032563a2d757623bbc11cf99df25c3c3f258dccbe was not found in src repository"
($1 --commit 16ef2f2629dc9263fdf3c0f032563a2d757623bbc11cf99df25c3c3f258dccbe -f "$TEMP_DIR"/treehub.json -p "$TEMP_DIR"/treehub.json --name testname -h hwids || true) | grep -x "$TARGET"
exit $?
