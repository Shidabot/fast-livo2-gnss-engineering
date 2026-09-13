# FAST-LIVO2-GNSS Odometry & Mapping

**English** | [中文](README.zh-CN.md)

**An engineering development fork derived from [FAST-LIVO2](https://github.com/hku-mars/FAST-LIVO2), with GNSS/RTK-constrained offline trajectory optimization and mapping.**



This personal project is under active engineering debugging. Its focus is sensor integration, timestamp and coordinate consistency, initialization, runtime reliability, configuration validation and repeatable testing.

This repository is engineering work, not an academic contribution or a new SLAM method. It is not an official FAST-LIVO2 release. The LiDAR–visual–inertial frontend originates from FAST-LIVO2.

> **Status:** development and debugging in progress. 

## System overview

| Sensor | Role |
|---|---|
| LiDAR | Geometric observations and point-cloud mapping |
| Camera | Direct visual observations in the FAST-LIVO2 frontend |
| IMU | State propagation, gravity initialization and motion compensation |
| GNSS / RTK | Global position observations for offline alignment and batch optimization |

The online frontend performs LiDAR–inertial and visual updates. After collection, the offline backend aligns GNSS observations with the trajectory, builds a GTSAM factor graph and exports an optimized trajectory and global point cloud.

**This project is an odometry and mapping system with GNSS-constrained offline optimization.** The frontend estimates local motion and builds a point-cloud map; the backend corrects the collected trajectory using global position observations.

Mapping can be part of a SLAM system, and loop closure is not a prerequisite for every use of the term SLAM. However, this working tree does not implement loop-closure detection or loop-closure constraints. Its documented scope is therefore **Odometry & Mapping**, rather than a complete SLAM stack.

**GNSS currently constrains the offline backend. Online RTK state correction is disabled.**

![System workflow](pics/method_overview_en.png)

## Data synchronization

![Timestamp-based data synchronization](pics/data_synchronization_en.png)

In LIVO mode, the corrected image timestamp defines the frontend update time. LiDAR points and IMU samples are grouped for that time, followed by an LIO update and a VIO update at the same state time. GNSS observations are aligned separately in the offline backend.

- Timestamp corrections are configured under `time_offset`.
- State, TF, path and point-cloud outputs use measurement state time.
- The IMU-rate prediction uses its corrected IMU timestamp.
- Local world coordinates use `camera_init`; IMU/body coordinates use `aft_mapped`.
- GNSS ENU positions and globally aligned results use `map`. A live global-to-local transform is not currently provided.

The figures are conceptual diagrams. Source code and configuration define actual runtime behavior.

## Engineering changes

| Area | Changes in the current working tree |
|---|---|
| GNSS / RTK | Observation validation, accuracy-to-variance conversion, gap-aware constraints, antenna-offset handling and backend lifecycle fixes |
| Visual processing | Projection and patch bounds checks, image validation, robust residual weighting, low-texture rejection and guarded state/covariance updates |
| IMU | Explicit propagation-time initialization, reset cleanup, stationary gyro-bias initialization and invalid-interval rejection |
| Interfaces | Measurement timestamps, corrected body-cloud frame labels and body-frame odometry velocity |
| Configuration | Startup checks for extrinsics, rotations, noise, preprocessing, visual parameters and map layers |
| Verification | CTest registration, Debug/Release utility CI and an output-bag contract checker |

These implementation changes are awaiting complete integration validation. They are not benchmark claims.

See [RTK changes](RTK_FIXES.md), [visual changes](VISUAL_FIXES.md) and [P0 engineering changes](P0_FIXES.md). The RTK and visual records describe earlier stages; P0 documents subsequent IMU changes.

## Dependencies and build

This is a **ROS1 catkin / C++17** project. A complete validated environment matrix is still being established.

| Dependency | Notes |
|---|---|
| CMake | 3.10 or newer |
| ROS1 / catkin | ROS C++, TF, image transport, CV bridge, PCL conversions and message filters |
| PCL, Eigen, OpenCV | Inherited baseline: PCL ≥ 1.8, Eigen ≥ 3.3.4, OpenCV ≥ 4.2 |
| OpenMP | Required by estimator timing and parallel code |
| Sophus | Legacy non-templated version; inherited setup uses commit `a621ff` |
| [rpg_vikit](https://github.com/xuankuzcr/rpg_vikit) | Camera models and utilities |
| [livox_ros_driver](https://github.com/Livox-SDK/livox_ros_driver) | ROS1 Livox message definitions |
| [gnss_comm](https://github.com/HKUST-Aerial-Robotics/gnss_comm) | GNSS PVT messages |
| [GTSAM](https://github.com/borglab/gtsam) | Offline factor-graph optimization |
| [GeographicLib](https://geographiclib.sourceforge.io/) | Geographic-to-ENU conversion |

Place this repository and its ROS package dependencies in a catkin workspace. Install the remaining native libraries and source your ROS environment, then:

```bash
cd /absolute/catkin_ws
catkin_make -DCMAKE_BUILD_TYPE=Debug -DCATKIN_ENABLE_TESTING=ON -DBUILD_TESTING=ON
source devel/setup.bash
```

Use Release for performance measurements after correctness checks. Record dependency revisions and compiler versions for each run. Dependency locking and a full reproducible container remain engineering tasks.

## Configuration

The inherited examples are `launch/HH.launch` with `config/HH-LVGO.yaml`, and `launch/AGV.launch` with `config/AGV-LVGO.yaml`. Use calibration matching your actual camera and sensor mounting.

| Group | Purpose |
|---|---|
| `common` | Topics and frontend mode |
| `extrin_calib` | LiDAR–IMU and LiDAR–camera extrinsics |
| `time_offset` | Sensor timestamp corrections |
| `imu`, `lio`, `vio` | Estimator settings |
| `gps` | GNSS topic, antenna offset, time offset and backend enable flag |
| `opt` | Offline replay, factor variances and GNSS gap limit |
| `pcd_save`, `publish` | Saving and visualization |

`gps/debug_mode` enables frontend debug recording. `opt/load_data` selects replay from debug recordings. The legacy `opt/debug_mode` alias is deprecated; conflicting values fail startup.

Set `common/img_en: 0` for LiDAR–inertial operation without a camera model. IMU initialization assumes a stationary sensor.

## Run with a ROS bag

Use a bag containing the configured sensors and matching calibration. For an HH-compatible dataset, run these in separate sourced terminals:

```bash
# Terminal 1
roscore
```

```bash
# Terminal 2
rosparam set /use_sim_time true
roslaunch fast_livo HH.launch rviz:=false outputfilepath:=/absolute/new_run/output
```

```bash
# Terminal 3
rosbag play --clock /absolute/input.bag
```

After playback, press **Enter** in the launch terminal to trigger offline optimization. The backend waits for the keyframe stream to become idle before freezing the collected data. Keep the process running until completion.

Use a fresh output directory for each run. Some inherited frontend logs and PCD/COLMAP resources still use source-tree paths; output-directory handling is not yet unified.

## Outputs

| Topic / directory | Meaning |
|---|---|
| `/odometry/fast_livo2` | Local frontend odometry |
| `/synced_cloud` | Body-frame cloud paired with odometry by timestamp |
| `/cloud_registered` | Registered point cloud in local world coordinates |
| `/aft_mapped_to_init` | Local odometry and associated TF |
| `/path` | Local trajectory visualization |
| `/LIVO2/imu_propagate` | Optional IMU-rate prediction |
| `outputfilepath/TUM/` | Backend trajectory exports |
| `outputfilepath/vel/` | Backend velocity exports |
| `outputfilepath/global_pcd/` | Global point-cloud exports |
| `outputfilepath/debug/` | Debug recordings when enabled |

Backend files depend on successful alignment and optimization. Global results are offline outputs.

## Tests and CI

Dependency-free C++ tests:

```bash
cmake -S . -B build-utils -DFAST_LIVO_UTILS_ONLY=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-utils --parallel 2
cd build-utils
ctest --output-on-failure
```

Full ROS build and CTest execution:

```bash
bash /absolute/path/to/this_repo/scripts/verify_p0.sh /absolute/catkin_ws
```

Record `/odometry/fast_livo2` and `/synced_cloud` during replay, stop recording cleanly, then:

```bash
python3 scripts/check_replay.py /absolute/output-check.bag
```

The checker validates finite odometry, quaternion normalization, frame labels, nondecreasing timestamps and cloud/odometry timestamp pairing. It does not measure trajectory accuracy or prove geometric frame correctness.

GitHub Actions runs dependency-free C++ tests in Debug/Release and tests the Python checker. It does not currently build the full ROS estimator or perform dataset replay. No deployment pipeline is implemented.

**Verification:** GitHub Actions passed the dependency-free C++ tests in Debug/Release and the Python checker tests. Local static consistency checks also passed. Full ROS estimator compilation, the dependency-based IMU integration test and real-data replay remain unverified.

## Engineering roadmap

- Complete ROS compilation and regression replay on a documented environment.
- Bound sensor queues, trajectory history and point-cloud memory.
- Add sensor timeout diagnostics, controlled degradation and recovery.
- Replace terminal-triggered optimization with an explicit session interface.
- Make recording and map export asynchronous, failure-aware and recoverable.
- Separate synchronization, estimation, storage, ROS adapters and offline optimization.

## Contributors

| Contributor | Role |
|---|---|
| [shida / Shidabot](https://github.com/Shidabot) | Project owner, engineering maintainer and local modification author; responsible for integration and validation decisions |
| Codex (OpenAI AI coding assistant) | Assisted with code review CI configuration and bilingual documentation |

Codex is credited as an AI assistant, not a human maintainer or a GitHub account. AI-assisted changes require engineering validation. See [contribution credits](CONTRIBUTORS.md). Original FAST-LIVO2 contributors are acknowledged separately below.

## Provenance, authorship and license

**Engineering maintainer and local modification author:** shida <shida.86@outlook.com>.


The repository includes the [GNU GPL version 2 license text](LICENSE). Third-party components retain their respective licenses and notices.

For this fork's engineering issues, contact [shida](mailto:shida.86@outlook.com) or use the repository issue tracker. Refer to [FAST-LIVO2](https://github.com/hku-mars/FAST-LIVO2) for the upstream implementation and original work.
