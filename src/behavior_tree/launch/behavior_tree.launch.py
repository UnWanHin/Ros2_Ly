import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import SetEnvironmentVariable

def generate_launch_description():
    
    # [可選] 如果你非常堅持要指定 Log 目錄，可以在這裡設置環境變數
    # 但通常 ROS 2 會自動管理 Log (默認在 ~/.ros/log)
    # set_log_dir = SetEnvironmentVariable('ROS_LOG_DIR', os.path.expanduser('~/workspace/logs/behavior_tree/log'))

    return LaunchDescription([
        # set_log_dir, # 如果上面解開註釋，這裡也要解開

        Node(
            package='behavior_tree',      # [注意] 請確認 package.xml 裡的包名 (通常是 rm_behavior_tree)
            executable='behavior_tree_node', # [注意] 請確認 CMakeLists.txt 裡 add_executable 的名字
            name='behavior_tree',            # 節點名稱
            output='screen',                 # 'screen' 會把日誌印在終端機，方便調試；'log' 則只存檔
            emulate_tty=True,                # 讓日誌有顏色
            parameters=[
                # 如果之後有 config.yaml 可以在這裡加載
                # os.path.join(get_package_share_directory('behavior_tree'), 'config', 'config.yaml')
            ]
        )
    ])