#!/bin/bash
set -eu

$1 --ref master --repo sota_tools/repo --credentials "$2" -n --loglevel 0 2>&1 | grep -q "Content-Type"
