#!/usr/bin/env python3
"""
behavior_tree 单包启动入口。

用途：
- 仅启动 behavior_tree_node，适合独立调试决策流程。
- 不负责拉起 detector / gimbal_driver 等下游节点。
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo, Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    output = LaunchConfiguration("output")
    competition_profile = LaunchConfiguration("competition_profile")
    bt_config_file = LaunchConfiguration("bt_config_file")
    bt_tree_file = LaunchConfiguration("bt_tree_file")

    launch_args = [
        DeclareLaunchArgument(
            "output",
            default_value="screen",
            description="ROS node output mode: screen or log.",
        ),
        DeclareLaunchArgument(
            "competition_profile",
            default_value="",
            description="behavior_tree profile override: regional or league. Empty keeps config/default.",
        ),
        DeclareLaunchArgument(
            "bt_config_file",
            default_value="",
            description="Optional behavior_tree JSON config path. Relative paths resolve under behavior_tree share dir.",
        ),
        DeclareLaunchArgument(
            "bt_tree_file",
            default_value="",
            description="Optional behavior_tree XML path. Relative paths resolve under behavior_tree share dir.",
        ),
    ]

    info_logs = [
        LogInfo(msg=["[behavior_tree] output: ", output]),
        LogInfo(msg=["[behavior_tree] competition_profile: ", competition_profile]),
        LogInfo(msg=["[behavior_tree] bt_config_file: ", bt_config_file]),
        LogInfo(msg=["[behavior_tree] bt_tree_file: ", bt_tree_file]),
    ]

    nodes = [
        Node(
            package="behavior_tree",
            executable="behavior_tree_node",
            name="behavior_tree",
            output=output,
            parameters=[
                {
                    "competition_profile": competition_profile,
                    "bt_config_file": bt_config_file,
                    "bt_tree_file": bt_tree_file,
                }
            ],
            on_exit=Shutdown(reason="behavior_tree exited"),
        )
    ]

    return LaunchDescription(launch_args + info_logs + nodes)
