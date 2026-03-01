#!/usr/bin/env python3
import argparse
import rclpy
from rclpy.node import Node
from auto_aim_common.msg import Target
from gimbal_driver.msg import GimbalAngles
from std_msgs.msg    import Bool, UInt8, Header

class TargetToGimbalNode(Node):
   def __init__(self, is_red=True, target_id=6, enable_fire=True, auto_fire=True):
       super().__init__('target_to_gimbal_mapper')
       
       # 存储参数
       self.is_red = is_red
       self.target_id = target_id
       self.enable_fire = enable_fire
       self.auto_fire = auto_fire
       
       # 火控状态
       self.fire_status = 0  # 0b00 或 0b11
       self.last_fire_command = False
       
       # 订阅目标话题
       self.target_sub = self.create_subscription(
           Target,
           '/ly/predictor/target',
           self.target_callback,
           10
       )
       
       # 发布云台角度
       self.gimbal_pub = self.create_publisher(GimbalAngles, '/ly/control/angles', 10)
       
       # 发布火控命令
       self.firecode_pub = self.create_publisher(UInt8, '/ly/control/firecode', 10)
       
       # 发布队伍颜色
       self.team_pub = self.create_publisher(Bool, '/ly/me/is_team_red', 10)
       
       # 发布目标ID
       self.bt_target_pub = self.create_publisher(UInt8, '/ly/bt/target', 10)
       
       # 创建定时器持续发布静态话题（10Hz）
       self.timer = self.create_timer(0.1, self.timer_callback)
       
       # 初始化时发布一次火控命令（确保下位机收到初始状态）
       if self.enable_fire:
           fire_msg = UInt8()
           fire_msg.data = self.fire_status
           self.firecode_pub.publish(fire_msg)
           self.get_logger().info(f'初始火控状态: 0b{self.fire_status:08b}={self.fire_status}')
       
       self.get_logger().info(
           f'节点启动: 队伍={"红方" if is_red else "蓝方"}, '
           f'target_id={target_id}, '
           f'火控={"启用" if enable_fire else "禁用"}, '
           f'自动开火={"是" if auto_fire else "否"}'
       )

   def target_callback(self, msg: Target):
       # 映射 Target -> GimbalAngles，保证 yaw/pitch 对应
       gimbal_msg = GimbalAngles()
       gimbal_msg.header = msg.header  # 复用时间戳
       gimbal_msg.yaw = msg.yaw
       gimbal_msg.pitch = msg.pitch
       
       self.gimbal_pub.publish(gimbal_msg)
       self.get_logger().debug(f'映射云台角度: yaw={msg.yaw:.3f}, pitch={msg.pitch:.3f}')
       
       # 更新目标状态（用于定时器判断是否持续开火）
       if self.enable_fire and self.auto_fire:
           self.last_fire_command = msg.status
           if msg.status:
               self.get_logger().debug(f'检测到目标: yaw={msg.yaw:.3f}, pitch={msg.pitch:.3f}')
           else:
               self.get_logger().debug('失去目标')

   def timer_callback(self):
       # 持续发布队伍颜色
       team_msg = Bool()
       team_msg.data = self.is_red
       self.team_pub.publish(team_msg)
       
       # 持续发布目标ID
       target_msg = UInt8()
       target_msg.data = self.target_id
       self.bt_target_pub.publish(target_msg)
       
       # 火控逻辑：有目标时持续翻转状态触发连续开火
       if self.enable_fire and self.auto_fire:
           if self.last_fire_command:  # 有合理目标
               # 每次定时器触发都翻转状态
               self.fire_status = 0b11 if self.fire_status == 0 else 0b00
               fire_msg = UInt8()
               fire_msg.data = self.fire_status
               self.firecode_pub.publish(fire_msg)
               self.get_logger().debug(f'持续开火: 0b{self.fire_status:08b}={self.fire_status}')
           else:  # 无目标
               # 保持当前状态，持续发布
               fire_msg = UInt8()
               fire_msg.data = self.fire_status
               self.firecode_pub.publish(fire_msg)
       elif self.enable_fire:
           # 仅启用火控但不自动开火，持续发布当前状态
           fire_msg = UInt8()
           fire_msg.data = self.fire_status
           self.firecode_pub.publish(fire_msg)

def main(args=None):
   parser = argparse.ArgumentParser(description='Target到云台角度映射节点')
   parser.add_argument(
       '--red', 
       type=lambda x: x.lower() in ('true', '1', 'yes'),
       default=True,
       help='是否为红方 (默认: true)'
   )
   parser.add_argument(
       '--target-id', 
       type=int, 
       default=6,
       help='BT目标ID (默认: 6)'
   )
   parser.add_argument(
       '--enable-fire',
       type=lambda x: x.lower() in ('true', '1', 'yes'),
       default=True,
       help='是否启用火控功能 (默认: true)'
   )
   parser.add_argument(
       '--auto-fire',
       type=lambda x: x.lower() in ('true', '1', 'yes'),
       default=True,
       help='是否自动开火 (默认: true)'
   )
   cli_args = parser.parse_args()
   
   rclpy.init(args=args)
   node = TargetToGimbalNode(
       is_red=cli_args.red,
       target_id=cli_args.target_id,
       enable_fire=cli_args.enable_fire,
       auto_fire=cli_args.auto_fire
   )
   
   try:
       rclpy.spin(node)
   except KeyboardInterrupt:
       node.get_logger().info('收到Ctrl+C，节点退出')
   finally:
       node.destroy_node()
       rclpy.shutdown()

if __name__ == '__main__':
   main()