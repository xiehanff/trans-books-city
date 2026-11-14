#!/usr/bin/env bash
set -euo pipefail

value="safe"
if [[ "$value" != "safe" ]]; then
    exit 1
fi

printf 'bash_smoke=%s\n' "$value"
