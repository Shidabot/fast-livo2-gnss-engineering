/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#include "optimization.h"

int main(int argc, char **argv)
{
  ros::init(argc, argv, "laserMapping");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);
  try {
  LIVMapper mapper(nh); 

  optimization opti(nh);


  mapper.initializeSubscribersAndPublishers(nh, it);
  mapper.run();
  return 0;
  } catch (const std::exception &e) {
    ROS_FATAL_STREAM("Startup/runtime failure: " << e.what());
    return 1;
  }
}