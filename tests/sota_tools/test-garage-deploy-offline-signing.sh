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

echo "#!/bin/bash" > "$TEMP_DIR/garage-sign"
echo "mkdir -p ./tuf/aktualizr" >> "$TEMP_DIR/garage-sign"
chmod +x "$TEMP_DIR/garage-sign"
export PATH=$PATH:$TEMP_DIR

echo "$TREEHUB" > "$TEMP_DIR/treehub.json"
./tests/sota_tools/treehub_server.py -p"${PORT}" -c &
until curl -I localhost:"${PORT}" 2>/dev/null; do sleep 0.5; done

cd "$TEMP_DIR"
# Currently, if credentials do not support offline signing, this will fail. If
# that ever changes, this will need to verify that the credentials do support
# offline signing.
$1 --commit b9ac1e45f9227df8ee191b6e51e09417bd36c6ebbeff999431e3073ac50f0563 -f "$TEMP_DIR/treehub.json" -p "$2" --name testname -h hwids
exit_code=$?
if [ -d "./tuf/aktualizr" ]; then
  echo "garage-sign tuf repo still present!"
  exit_code=1
fi

exit $exit_code
