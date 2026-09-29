# ROS 2 UR5e MoveIt

带两指夹爪的 UR5e，在 Gazebo 中把桌上圆柱从一侧搬到另一侧。运动规划由 MoveIt 完成，关节由 ros2_control 驱动。

目标发行版：ROS 2 Jazzy。命令在工作空间根目录执行（本目录的上一级）。

## 包

| 包 | 作用 |
| --- | --- |
| [`robot_gripper`](robot_gripper/README.md) | UR5e 与夹爪的 URDF，以及 Gazebo 硬件接口 |
| [`moveit_robot_setup`](moveit_robot_setup/README.md) | 规划组、运动学、碰撞、控制器 |
| [`robot_move`](robot_move/README.md) | 仿真世界与搬运节点 |
| `Universal_Robots_ROS2_Description` | 上游 UR 描述，安装后包名为 `ur_description` |

## 构建

```bash
rosdep install --from-paths src --ignore-src -r -y
colcon build
source install/setup.bash
```

## 运行

终端 1，先等 Gazebo 中出现机器人、控制器加载结束：

```bash
ros2 launch robot_move gazebo_move.launch.py
```

终端 2：

```bash
ros2 launch robot_move run_move.launch.py
```

只在 RViz 里规划、不进 Gazebo：

```bash
ros2 launch moveit_robot_setup demo.launch.py
```

## 改模型之后

MoveIt 读的是展开后的 `robot_gripper/urdf/robot_gripper_final.urdf`。改过 `robot_gripper.urdf.xacro` 后：

```bash
source install/setup.bash
xacro src/robot_gripper/urdf/robot_gripper.urdf.xacro \
  -o src/robot_gripper/urdf/robot_gripper_final.urdf
colcon build --packages-select robot_gripper
source install/setup.bash
```

只改 `robot_move`：

```bash
colcon build --packages-select robot_move
source install/setup.bash
```

原理与操作细节见 [docs/工作原理.md](docs/工作原理.md)、[docs/操作手册.md](docs/操作手册.md)。
