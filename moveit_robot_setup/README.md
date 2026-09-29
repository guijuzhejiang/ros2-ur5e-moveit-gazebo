# moveit_robot_setup

`robot_gripper` 的 MoveIt 配置，由 MoveIt Setup Assistant 生成。语义机器人名为 `robot_gripper`。

## 规划

| 项 | 内容 |
| --- | --- |
| `arm` | 链 `base_link` → `tool0`，运动学求解器 KDL |
| `gripper` | `left_finger_joint`、`right_finger_joint` |
| 末端执行器 | `gripper1`，父连杆 `tool0` |
| `home` | 手臂命名姿态，见 `config/robot_gripper.srdf` |
| 虚拟关节 | `world` 到 `base_link`，固定 |

## 控制器

`config/ros2_controllers.yaml` 给 ros2_control，`config/moveit_controllers.yaml` 给 MoveIt。两者管理同一组关节：

| 控制器 | 关节 | 接口 |
| --- | --- | --- |
| `arm_controller` | 肩、肘、腕共 6 轴 | `FollowJointTrajectory` |
| `gripper_controller` | 两根手指 | `FollowJointTrajectory` |
| `joint_state_broadcaster` | 全部关节状态 | — |

Gazebo 中的硬件插件在 `robot_gripper` 的 `gazebo_env.xacro` 里加载上述 ros2_control 配置。`config/robot_gripper.ros2_control.xacro` 中的 `mock_components/GenericSystem` 只供下面的演示启动使用。

## 启动

```bash
ros2 launch moveit_robot_setup demo.launch.py
```

假硬件带动关节，在 RViz 里规划。带桌子、圆柱和 Gazebo 的仿真在 `robot_move` 的 `gazebo_move.launch.py`。

模型重新展开后，编译本包，使 `config/robot_gripper.urdf.xacro` 引用的 URDF 进入 `install`。
