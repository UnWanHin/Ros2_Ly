#!/usr/bin/env python3
import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    home_dir = os.environ['HOME']
    config_file = os.path.join(home_dir, 'ros2_ly_ws/src/detector/config/auto_aim_config.yaml')
    
    print(f"\033[92m[能量機關模式] 載入配置: {config_file}\033[0m")
    
    return LaunchDescription([
        Node(
            package='gimbal_driver',
            executable='gimbal_driver_node',
            name='gimbal_driver',
            output='screen',
            parameters=[config_file]
        ),
        Node(
            package='buff_hitter',
            executable='buff_hitter_node',
            name='buff_hitter',
            output='screen',
            parameters=[config_file]
        ),
        Node(
            package='detector',
            executable='simple_bridge_node',
            name='simple_bridge',
            output='screen',
            parameters=[config_file]
        ),
    ])
