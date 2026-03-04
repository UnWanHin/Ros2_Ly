#!/usr/bin/env python3
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # 獲取套件路徑
    detector_share = get_package_share_directory('detector')
    
    # 參數檔案路徑
    config_file = os.path.join(detector_share, 'config', 'auto_aim_config.yaml')
    
    return LaunchDescription([
        # gimbal_driver 節點
        Node(
            package='gimbal_driver',
            executable='gimbal_driver_node',
            name='gimbal_driver',
            output='log',
            parameters=[config_file]
        ),
        
        # shooting_table_calib 節點
        Node(
            package='shooting_table_calib',
            executable='shooting_table_calib_node',
            name='shooting_table_calib',
            output='screen',
            parameters=[
                config_file,
                {
                    # detector 配置路徑 (保留原始接口)
                    'detector_config.classifier_path': os.path.join(detector_share, 'Extras', 'classifier.xml'),
                    'detector_config.detector_path': os.path.join(detector_share, 'Extras', 'armor_detector_model.xml'),
                    'detector_config.car_model_path': os.path.join(detector_share, 'Extras', 'car_detector_model.xml'),
                    
                    # 隊伍顏色 (保留原始接口)
                    'team_red': True,
                    
                    # 顯示設置 (保留原始接口)
                    'web_show': True,
                    'draw_image': True,
                }
            ]
        ),
    ])
