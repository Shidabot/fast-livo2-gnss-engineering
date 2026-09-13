# P0 engineering changes

## Changes

- IMU propagation time now has an explicit invalid initial value. Initialization anchors the state at the final initialization sample; reset clears time, statistics and pending points.
- IMU initialization estimates stationary gyro bias when bias estimation is enabled. Initialization still assumes a stationary sensor; this change does not implement motion-aware initialization.
- `Process2` and `processImu` return whether an update occurred. Initialization, malformed IMU batches, missing propagation data and backward propagation do not enter mapping.
- Propagation intervals are clipped to the requested state interval, including the final short extrapolation. Automatic recovery after a clock reset is not implemented: restart the session.
- Published state/cloud/TF/path timestamps use measurement state time. The IMU-rate output keeps its corrected IMU timestamp. Accumulated maps are stamped with the latest contributing state time.
- Body clouds use `aft_mapped`; local world outputs use `camera_init`. Existing frame names are retained for launch/RViz compatibility. Odometry names its child frame and expresses linear velocity in that frame.
- Raw GNSS ENU positions use `map`, consistent with the backend GNSS output. No live map-to-local transform is invented; global/local alignment remains an offline result.
- Startup validates extrinsic sizes, finite values, proper rotation matrices, preprocessing, visual settings, noise and map layers. LIO does not require a camera model. Errors are reported with a nonzero process exit status.
- `gps/debug_mode` remains frontend recording. `opt/load_data` selects offline replay. Legacy `opt/debug_mode` is accepted with a warning, but conflicting values fail startup.
- Debug build selection is respected. Unsafe fast-math flags are removed. OpenMP is an explicit linked requirement because estimator code calls its timing API unconditionally.
- Existing RTK/visual tests and new configuration/IMU tests are registered with CTest. Assert-based tests remain active in Release. GitHub Actions runs dependency-free C++ tests in Debug/Release and Python checker tests.

## Verification status

The six Python replay-checker tests were executed successfully on Windows. Source/configuration and patch consistency checks were also executed. This machine has no usable C++ compiler, ROS, Linux distribution or sensor bag. C++ tests, estimator compilation and real data replay have **not** been executed. GitHub Actions has been authored but has not run here.

The utility CI job is not a complete ROS estimator validation. No trajectory accuracy claim is made.

## Dependency-free tests (C++17 compiler and CMake >= 3.10)

```bash
cmake -S . -B build-utils -DFAST_LIVO_UTILS_ONLY=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-utils --parallel 2
cd build-utils && ctest --output-on-failure
```

Repeat with Release. Run checker tests from the repository root:

```bash
python3 -m unittest discover -s tests -p 'test_*.py'
```

## Full ROS build and regression tests

Install the dependencies documented in README into a ROS1 catkin workspace, source ROS, then:

```bash
bash /absolute/path/to/fast_livo/scripts/verify_p0.sh /absolute/catkin_ws
```

The IMU integration test covers initialization, first propagation of a stationary biased IMU, rejected backward/missing/non-finite data and reset to a new time epoch. It requires the real estimator dependencies.

## Real replay gate (must still be performed)

Use an existing dataset with matching camera calibration and all enabled sensors. In separate sourced terminals:

```bash
roscore
rosparam set /use_sim_time true
roslaunch fast_livo HH.launch rviz:=false outputfilepath:=/absolute/new/output
rosbag record -O /absolute/output-check.bag /odometry/fast_livo2 /synced_cloud /aft_mapped_to_init /path /tf /cloud_registered
rosbag play --clock /absolute/input.bag
```

After playback, stop recording cleanly, then:

```bash
python3 /absolute/path/to/fast_livo/scripts/check_replay.py /absolute/output-check.bag
```

The checker requires nonempty odometry and synced cloud outputs, finite odometry/covariance, a unit quaternion, nondecreasing timestamps, correct frame labels and matching odometry timestamps for each synced cloud. It does not prove that point coordinates match their labels, check ground-truth accuracy, or compare output timestamps to input acquisition timestamps. Inspect TF/pointcloud alignment and compare trajectory error against a known baseline before accepting this version.

Also replay with `common/img_en=0` and no camera configuration, and verify malformed extrinsics fail startup. A sustained run and degraded-mode recovery belong to P1.
