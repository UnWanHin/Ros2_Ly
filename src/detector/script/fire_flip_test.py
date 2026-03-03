#!/usr/bin/env python3
import argparse
import os
import signal
import subprocess
import time

import rclpy
from rclpy.node import Node
from std_msgs.msg import UInt8


class FireFlipTestNode(Node):
    def __init__(self, fire_hz: float):
        super().__init__('fire_flip_test')
        self.fire_pub = self.create_publisher(UInt8, '/ly/control/firecode', 10)
        self.fire_status = 0

        self.get_logger().warn(
            f'Fire flip test started: /ly/control/firecode will toggle at {fire_hz:.2f} Hz')

        # 啟動即翻轉一次，方便硬件測試不等待第一個 timer 週期
        self._on_timer()

        period = 1.0 / max(fire_hz, 0.1)
        self.timer = self.create_timer(period, self._on_timer)

    def _publish_fire(self, value: int):
        msg = UInt8()
        msg.data = value
        self.fire_pub.publish(msg)

    def _on_timer(self):
        self.fire_status = 0b11 if self.fire_status == 0 else 0b00
        self._publish_fire(self.fire_status)

    def stop_safely(self):
        # 退出前清零，避免殘留開火狀態
        self._publish_fire(0)
        self.get_logger().info('Fire flip test stopping, firecode reset to 0')


def _start_detector_process(params_file: str | None, warmup_sec: float) -> subprocess.Popen:
    cmd = ['ros2', 'run', 'detector', 'detector_node']
    if params_file:
        cmd += ['--ros-args', '--params-file', params_file]

    print('[fire_flip_test] starting detector:', ' '.join(cmd), flush=True)
    proc = subprocess.Popen(cmd, preexec_fn=os.setsid)
    time.sleep(max(warmup_sec, 0.0))
    return proc


def _stop_process_tree(proc: subprocess.Popen):
    if proc is None or proc.poll() is not None:
        return
    try:
        os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
        proc.wait(timeout=3.0)
    except Exception:
        try:
            os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
        except Exception:
            pass


def main(args=None):
    parser = argparse.ArgumentParser(
        description='實機火控翻轉測試：可選擇拉起 detector，並持續翻轉 firecode')
    parser.add_argument('--fire-hz', type=float, default=8.0,
                        help='火控翻轉頻率 (Hz), 預設 8.0')
    parser.add_argument('--start-detector',
                        type=lambda x: x.lower() in ('1', 'true', 'yes'),
                        default=False,
                        help='是否同時拉起 detector_node (預設: false)')
    parser.add_argument('--params-file', type=str, default='',
                        help='可選: detector params yaml 路徑')
    parser.add_argument('--warmup-sec', type=float, default=1.5,
                        help='拉起 detector 後等待秒數 (預設: 1.5)')
    cli = parser.parse_args()

    detector_proc = None
    if cli.start_detector:
        detector_proc = _start_detector_process(
            cli.params_file if cli.params_file else None,
            cli.warmup_sec,
        )

    rclpy.init(args=args)
    node = FireFlipTestNode(cli.fire_hz)

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.stop_safely()
        node.destroy_node()
        rclpy.shutdown()
        _stop_process_tree(detector_proc)


if __name__ == '__main__':
    main()
