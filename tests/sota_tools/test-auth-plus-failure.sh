#!/bin/bash
set -uo pipefail

CREDS_FILE=$(mktemp)
trap 'rm -f $CREDS_FILE' EXIT
cat > "$CREDS_FILE" <<EOF
{
  "oauth2": {
    "client_id":"c169e6d6-d2de-4c56-ac8c-8b7671097e0c",
    "client_secret":"badsecret",
    "server":"https://auth-plus.atsgarage.com"
  },
  "ostree":{
    "server":"https://treehub.atsgarage.com/api/v3"
  }
}
EOF
$1 --ref master --repo sota_tools/repo --credentials "$CREDS_FILE" -n
test $? -eq 1
