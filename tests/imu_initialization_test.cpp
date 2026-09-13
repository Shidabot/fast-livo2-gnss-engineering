/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#include "IMU_Processing.h"
#include <cassert>
#include <limits>

sensor_msgs::ImuConstPtr sample(double time, double acc = 9.81) {
    sensor_msgs::ImuPtr imu(new sensor_msgs::Imu());
    imu->header.stamp.fromSec(time);
    imu->linear_acceleration.z = acc;
    imu->angular_velocity.z = 0.01;
    return imu;
}
int main(int argc, char **argv) {
    ros::init(argc, argv, "imu_initialization_test", ros::init_options::AnonymousName);
    ImuProcess imu;
    imu.lidar_type = AVIA;
    imu.first_lidar_time = 1.0;
    imu.set_imu_init_frame_num(3);
    StatesGroup state;
    LidarMeasureGroup batch;
    batch.lio_vio_flg = LIO;
    PointCloudXYZI::Ptr output(new PointCloudXYZI());
    MeasureGroup init;
    init.lio_time = 1.03;
    for (int i = 0; i < 4; ++i) init.imu.push_back(sample(1.0 + i * 0.01));
    batch.measures.push_back(init);
    assert(!imu.Process2(batch, state, output, Sophus::SE3()));
    assert(!imu.imu_need_init);
    assert(output->empty());
    assert(std::abs(state.bias_g.z() - 0.01) < 1e-9);
    PointType point{};
    point.x = 1;
    point.curvature = 10;
    batch.pcl_proc_cur->push_back(point);
    batch.lidar_frame_beg_time = 1.03;
    MeasureGroup next;
    next.lio_time = 1.05;
    next.imu.push_back(sample(1.04));
    next.imu.push_back(sample(1.05));
    batch.measures.clear();
    batch.measures.push_back(next);
    assert(imu.Process2(batch, state, output, Sophus::SE3()));
    assert(state.pos_end.allFinite() && state.cov.allFinite());
    assert(state.pos_end.norm() < 1e-8);
    assert((state.rot_end - M3D::Identity()).norm() < 1e-8);
    const StatesGroup before = state;
    batch.measures.back().lio_time = 1.02;
    assert(!imu.Process2(batch, state, output, Sophus::SE3()));
    assert((state.cov - before.cov).norm() == 0);
    batch.measures.back().lio_time = 1.06;
    batch.measures.back().imu.clear();
    assert(!imu.Process2(batch, state, output, Sophus::SE3()));
    batch.measures.back().imu.push_back(sample(1.06, std::numeric_limits<double>::quiet_NaN()));
    assert(!imu.Process2(batch, state, output, Sophus::SE3()));
    imu.Reset();
    assert(imu.imu_need_init && !imu.imu_time_init);
    // A new epoch must initialize again without reusing old statistics or time.
    batch.measures.clear();
    init.imu.clear();
    init.lio_time = 0.23;
    for (int i = 0; i < 4; ++i) init.imu.push_back(sample(0.2 + i * 0.01));
    batch.measures.push_back(init);
    assert(!imu.Process2(batch, state, output, Sophus::SE3()));
    assert(!imu.imu_need_init);
}
