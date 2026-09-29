from moveit_configs_utils import MoveItConfigsBuilder
from moveit_configs_utils.launches import generate_moveit_rviz_launch


def generate_launch_description():
    moveit_config = MoveItConfigsBuilder("robot_gripper", package_name="moveit_robot_setup").to_moveit_configs()
    return generate_moveit_rviz_launch(moveit_config)
