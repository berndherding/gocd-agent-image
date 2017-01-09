#!/usr/bin/env bash

# shellcheck source=agent.inc
. "$(dirname "${BASH_SOURCE[0]}")/agent.inc"

# shellcheck disable=SC2119
destroy "${1:-$(getImageTag)}"
