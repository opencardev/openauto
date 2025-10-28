#!/usr/bin/env bash
set -euo pipefail

# Compute distro-specific Debian revision suffix for CPack DEB packages.
# Prints the suffix to stdout, e.g.:
#  - Debian 12 (bookworm): 1+deb12u1
#  - Debian 13 (trixie):   1+deb13u1
#  - Ubuntu 22.04:         0ubuntu1~22.04
#  - Ubuntu 24.04:         0ubuntu1~24.04
#  - Raspberry Pi OS:      1+rpi12u1 (example)
#  - Fallback:             1~<id><version>-<codename>

if [[ -r /etc/os-release ]]; then
  # shellcheck disable=SC1091
  . /etc/os-release
else
  echo "1~unknown" && exit 0
fi

id_lc=$(echo "${ID:-unknown}" | tr '[:upper:]' '[:lower:]')
suite=${VERSION_CODENAME:-unknown}
verid=${VERSION_ID:-0}

case "$id_lc" in
  debian)
    case "$suite" in
      bookworm) deb_num=12 ;;
      trixie)   deb_num=13 ;;
      *)        deb_num="${verid%%.*}" ;;
    esac
    if [[ -n "${deb_num}" ]]; then
      echo "1+deb${deb_num}u1"
    else
      echo "1~debian${suite}1"
    fi
    ;;
  ubuntu)
    series="${verid}"
    echo "0ubuntu1~${series}"
    ;;
  raspbian|raspios)
    series="${verid%%.*}"
    echo "1+rpi${series}u1"
    ;;
  *)
    echo "1~${id_lc}${verid}-${suite}"
    ;;
esac
