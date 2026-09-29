#!/usr/bin/env python3
from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    #fetch the urdf, srdf, and kinematics from your setup package
    moveit_config = MoveItConfigsBuilder("moveit_robot_setup", package_name="moveit_robot_setup").to_moveit_configs()

    #run your C++ node and inject the required parameters
   
    move_node = Node(
            package='robot_move',
            executable='robot_move_execute',
            output="screen",
            parameters=[
                        moveit_config.robot_description,
                        moveit_config.robot_description_semantic,
                        moveit_config.robot_description_kinematics,
                        {"use_sim_time": True}
                    ]
        )

    return LaunchDescription([move_node])