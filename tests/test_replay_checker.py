# Engineering fork maintainer: shida <shida.86@outlook.com>
# Maintenance notice added: 2026-09-13
import importlib.util
from pathlib import Path
from types import SimpleNamespace as NS
import unittest

spec = importlib.util.spec_from_file_location("check_replay", Path(__file__).parents[1] / "scripts/check_replay.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

def message(stamp, cloud=False):
    vector = NS(x=0.0, y=0.0, z=0.0)
    return NS(header=NS(stamp=NS(to_nsec=lambda: stamp), frame_id="aft_mapped" if cloud else "camera_init"),
              child_frame_id="aft_mapped",
              pose=NS(pose=NS(position=vector, orientation=NS(x=0., y=0., z=0., w=1.)), covariance=[0.]*36),
              twist=NS(twist=NS(linear=vector), covariance=[0.]*36))

class ReplayCheckerTest(unittest.TestCase):
    def test_valid_pair(self):
        c = module.OutputChecker()
        c.consume(module.CLOUD, message(100, True))
        c.consume(module.ODOM, message(100))
        self.assertEqual(c.finish()[module.CLOUD], 1)

    def test_mismatched_pair(self):
        c = module.OutputChecker()
        c.consume(module.CLOUD, message(101, True))
        c.consume(module.ODOM, message(100))
        with self.assertRaises(ValueError): c.finish()

    def test_bad_frame(self):
        c = module.OutputChecker()
        with self.assertRaises(ValueError): c.consume(module.CLOUD, message(100))

    def test_time_regression(self):
        c = module.OutputChecker()
        c.consume(module.ODOM, message(100))
        with self.assertRaises(ValueError): c.consume(module.ODOM, message(99))

    def test_nonfinite_state(self):
        c = module.OutputChecker()
        msg = message(100)
        msg.pose.pose.position.x = float("nan")
        with self.assertRaises(ValueError): c.consume(module.ODOM, msg)

    def test_missing_output(self):
        with self.assertRaises(ValueError): module.OutputChecker().finish()

if __name__ == "__main__": unittest.main()
