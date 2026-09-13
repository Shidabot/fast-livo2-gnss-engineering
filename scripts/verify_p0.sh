#!/usr/bin/env bash
# Engineering fork maintainer: shida <shida.86@outlook.com>
# Maintenance notice added: 2026-09-13
# Run inside the configured catkin workspace with ROS and dependencies installed.
set -euo pipefail
workspace="${1:?Usage: verify_p0.sh /absolute/catkin_workspace}"
cd "$workspace"
catkin_make -DCMAKE_BUILD_TYPE=Debug -DCATKIN_ENABLE_TESTING=ON -DBUILD_TESTING=ON
cd "$workspace/build"
ctest --output-on-failure
