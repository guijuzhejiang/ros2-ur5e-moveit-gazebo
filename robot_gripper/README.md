# robot_gripper

UR5e 与两指夹爪的机器人描述。本包安装 URDF/Xacro，供 MoveIt 和 Gazebo 使用。手臂模型来自 `ur_description`。

底座相对 `world` 抬高 0.8 m，与桌面齐平。夹爪经 `gripper_adapter_link` 固定在 `tool0` 上。

## 文件

| 文件 | 内容 |
| --- | --- |
| `urdf/robot_gripper.urdf.xacro` | 整机入口：UR5e、夹爪、Gazebo 接口 |
| `urdf/gripper_macro.xacro` | 夹爪几何与 `left_finger_joint`、`right_finger_joint` |
| `urdf/gazebo_env.xacro` | `gz_ros2_control` 硬件接口，控制器参数指向 `moveit_robot_setup` |
| `urdf/robot_gripper_final.urdf` | 展开结果。`moveit_robot_setup` 直接包含这个文件 |

两指是移动关节，轴向朝中间合拢，行程上限 0.02555 m。搬运程序里张开为 `0.005`，夹紧为 `0.015`。

## 修改模型

改 Xacro 后重新展开，再只编译本包。否则 `install` 里仍是旧的 `robot_gripper_final.urdf`。展开前工作空间中需已有 `ur_description` 和本包。

```bash
source install/setup.bash
xacro src/robot_gripper/urdf/robot_gripper.urdf.xacro \
  -o src/robot_gripper/urdf/robot_gripper_final.urdf
colcon build --packages-select robot_gripper
source install/setup.bash
```
