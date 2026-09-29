# robot_move

Gazebo 世界与搬运节点。世界 `robot_world` 中有地面、2 m × 2 m × 0.8 m 的桌子，以及桌面上的蓝色圆柱，位置 `(0.5, 0.4)`。

节点 `robot_move_execute` 使用 MoveIt 规划组 `arm` 和 `gripper`，把圆柱规划到对称点 `(-0.5, -0.4)`。规划场景中的圆柱贴在 `tool0` 上，只用于避碰。Gazebo 里的圆柱是独立模型，停留在原位。

## 启动

两个终端都在工作空间根目录，且已 `source install/setup.bash`。

```bash
ros2 launch robot_move gazebo_move.launch.py
```

该启动文件拉起 Gazebo、`/clock` 桥、`robot_state_publisher`、模型生成、三个控制器、`move_group` 和 RViz。节点使用仿真时间。

```bash
ros2 launch robot_move run_move.launch.py
```

## 动作

源码在 `src/robot_motion_source.cpp`。规划参考系为 `base_link`，速度与加速度缩放 0.2，规划时限 15 s，末端姿态为夹爪朝下。

1. 手臂到 `home`，夹爪张开到 `0.005`。
2. 肩关节到直立准备姿态。
3. 移到圆柱上方，降到抓取高度前，再沿直线到抓取点。
4. 夹爪合到 `0.015`，把圆柱挂到 `tool0`。
5. 抬起，底座关节转约 180°。
6. 下降，张开，从规划场景移除圆柱。
7. 直线抬起，回到 `home`。

桌子同样写入规划场景，尺寸对应 `worlds/empty.sdf`，高度略收，避免与桌面模型贴死。

只改本包时：

```bash
colcon build --packages-select robot_move
source install/setup.bash
```
