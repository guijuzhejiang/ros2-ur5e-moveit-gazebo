#!/usr/bin/env python3
"""
Gazebo仿真启动文件 - 用于启动机械臂仿真环境
"""

import os
from launch import LaunchDescription
from launch.actions import (
    IncludeLaunchDescription,
    ExecuteProcess,
    SetEnvironmentVariable,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    # 使用UR5作为临时替代方案
    ur_desc_share = get_package_share_directory("ur_description")
    gripper_share = get_package_share_directory("robot_gripper")

    ur_sim_path = os.path.dirname(ur_desc_share)
    gripper_sim_path = os.path.dirname(gripper_share)

    existing_path = os.environ.get("GZ_SIM_RESOURCE_PATH", "")
    gz_resource_path = f"{ur_sim_path}:{gripper_sim_path}"
    if existing_path:
        gz_resource_path += f":{existing_path}"

    set_gz_resource_path = SetEnvironmentVariable(
        "GZ_SIM_RESOURCE_PATH", gz_resource_path
    )

    # Moveit configuration package we created
    moveit_config = MoveItConfigsBuilder(
        "moveit_robot_setup", package_name="moveit_robot_setup"
    ).to_moveit_configs()
    world_file = os.path.join(
        get_package_share_directory("robot_move"), "worlds", "empty.sdf"
    )
    rviz_config_file = os.path.join(
        get_package_share_directory("moveit_robot_setup"), "config", "moveit.rviz"
    )
    pathGazebo = os.path.join(
        get_package_share_directory("ros_gz_sim"), "launch", "gz_sim.launch.py"
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(pathGazebo),
        launch_arguments={"gz_args": f"-r {world_file}"}.items(),
    )

    # empty.sdf 的世界名是 robot_world，Gazebo 把时钟发在 /world/robot_world/clock。
    # ROS 的 use_sim_time 只认 /clock，所以在这里改名，并使用 Clock QoS。
    clock_bridge = Node(
        package="ros_gz_bridge",
        executable="bridge_node",
        output="screen",
        parameters=[
            {
                "use_sim_time": False,
                "bridge_names": ["clock"],
                "bridges": {
                    "clock": {
                        "ros_topic_name": "/clock",
                        "gz_topic_name": "/world/robot_world/clock",
                        "ros_type_name": "rosgraph_msgs/msg/Clock",
                        "gz_type_name": "gz.msgs.Clock",
                        "direction": "GZ_TO_ROS",
                        "qos_profile": "CLOCK",
                    }
                },
            }
        ],
    )

    ros_gz_sim_node = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=["-topic", "robot_description", "-name", "robot_gripper"],
        output="screen",
    )

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[
            moveit_config.robot_description,
            {"use_sim_time": True},
        ],
    )

    joint_state_br = ExecuteProcess(
        cmd=[
            "ros2",
            "control",
            "load_controller",
            "--set-state",
            "active",
            "joint_state_broadcaster",
        ]
    )
    arm_control = ExecuteProcess(
        cmd=[
            "ros2",
            "control",
            "load_controller",
            "--set-state",
            "active",
            "arm_controller",
        ]
    )
    grip_control = ExecuteProcess(
        cmd=[
            "ros2",
            "control",
            "load_controller",
            "--set-state",
            "active",
            "gripper_controller",
        ]
    )

    move_group_node = Node(
        package="moveit_ros_move_group",
        executable="move_group",
        output="screen",
        parameters=[
            moveit_config.to_dict(),
            {"use_sim_time": True},
        ],
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", rviz_config_file],
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            {"use_sim_time": True},
        ],
    )

    return LaunchDescription(
        [
            set_gz_resource_path,
            gazebo,
            clock_bridge,
            robot_state_publisher_node,
            ros_gz_sim_node,
            joint_state_br,
            arm_control,
            grip_control,
            move_group_node,
            rviz_node,
        ]
    )
