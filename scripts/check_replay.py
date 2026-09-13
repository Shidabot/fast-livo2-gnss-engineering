#!/usr/bin/env python3
# Engineering fork maintainer: shida <shida.86@outlook.com>
# Maintenance notice added: 2026-09-13
"""Check the public output contract of a ROS1 output bag; not trajectory accuracy."""
import argparse
import math

ODOM = "/odometry/fast_livo2"
CLOUD = "/synced_cloud"

class OutputChecker:
    def __init__(self):
        self.last = {}
        self.stamps = {ODOM: set(), CLOUD: set()}
        self.counts = {ODOM: 0, CLOUD: 0}

    def consume(self, topic, msg):
        if topic not in self.stamps:
            return
        stamp = msg.header.stamp.to_nsec()
        if stamp <= 0 or stamp < self.last.get(topic, 0):
            raise ValueError(topic + ": zero or decreasing timestamp")
        self.last[topic] = stamp
        expected_frame = "camera_init" if topic == ODOM else "aft_mapped"
        if msg.header.frame_id != expected_frame:
            raise ValueError(topic + ": incorrect frame_id")
        if topic == ODOM:
            if msg.child_frame_id != "aft_mapped":
                raise ValueError(topic + ": incorrect child_frame_id")
            p, q, v = msg.pose.pose.position, msg.pose.pose.orientation, msg.twist.twist.linear
            values = [p.x, p.y, p.z, q.x, q.y, q.z, q.w, v.x, v.y, v.z]
            values += list(msg.pose.covariance) + list(msg.twist.covariance)
            if not all(math.isfinite(x) for x in values):
                raise ValueError(topic + ": non-finite state/covariance")
            if abs(sum(x*x for x in [q.x, q.y, q.z, q.w]) - 1.0) > 1e-3:
                raise ValueError(topic + ": non-unit quaternion")
        self.stamps[topic].add(stamp)
        self.counts[topic] += 1

    def finish(self):
        if not all(self.counts.values()):
            raise ValueError("Required odometry or cloud topic is missing/empty")
        if not self.stamps[CLOUD].issubset(self.stamps[ODOM]):
            raise ValueError("A synced cloud has no odometry with the same measurement timestamp")
        return self.counts

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output_bag")
    args = parser.parse_args()
    import rosbag
    checker = OutputChecker()
    with rosbag.Bag(args.output_bag) as bag:
        for topic, msg, _ in bag.read_messages(topics=[ODOM, CLOUD]):
            checker.consume(topic, msg)
    print("Output contract passed:", checker.finish())

if __name__ == "__main__":
    main()
