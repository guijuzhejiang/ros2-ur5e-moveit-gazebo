// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <moveit_msgs/msg/attached_collision_object.hpp>
#include <shape_msgs/msg/solid_primitive.hpp>

// C++ includes
#include <chrono>
#include <memory>
#include <thread>
#include <vector>
#include <string>

// Decleare this to reduce verbosity
using MoveGroup = moveit::planning_interface::MoveGroupInterface;
using PlanningScene = moveit::planning_interface::PlanningSceneInterface;
using Pose = geometry_msgs::msg::Pose;

//define the cylinder x,y,z coordinates of the origin, and dimensions - look at empty.sdf
double xCyl=0.5;
double yCyl=0.4;
double zCyl=0.85;
//radius and height
double rCyl=0.016;
double hCyl=0.1;
//define the table x,y,z of origin (center) and dimensions - look at empty.sdf
double xTab=0.0;
double yTab=0.0;
//here, we slightly decrease z compared to empty.sdf
// double zTab=(0.8-0.1)/2;
double zTab=0.399;
//width , length and height
double wTab=2.0;
double lTab=2.0;
//slightly decrease the heigh
//double hTab=0.79;
double hTab=0.798;
// Create a pose from (x,y,z) and Euler angles
Pose createPose(double x, double y, double z, double roll, double pitch, double yaw)
{
  Pose pose;
  pose.position.x = x;
  pose.position.y = y;
  pose.position.z = z;

  tf2::Quaternion quat;
  quat.setRPY(roll, pitch, yaw);
  pose.orientation.x = quat.x();
  pose.orientation.y = quat.y();
  pose.orientation.z = quat.z();
  pose.orientation.w = quat.w();

  return pose;
}

bool planExecute(MoveGroup& group,
                 MoveGroup::Plan& plan,
                 const std::shared_ptr<rclcpp::Node>& node,
                const std::string& stepName)
{
  //create a plan;
  auto planIndicator = group.plan(plan);
  if (planIndicator == moveit::core::MoveItErrorCode::SUCCESS)
  {
    //if successfull, execute
    RCLCPP_INFO(node->get_logger(), "Execute the step: %s", stepName.c_str());
    //execute the plan
    group.execute(plan);
    return true;
  } else
  {
    RCLCPP_ERROR(node->get_logger(), "Failed the step: %s", stepName.c_str());
    return false;
  }
}

void attachCylinder(PlanningScene& planScene,
                    double x,
                    double y,
                    double z,
                    double rC,
                    double hC,
                    const std::shared_ptr<rclcpp::Node>& node)
{
  moveit_msgs::msg::AttachedCollisionObject aco;

  aco.link_name = "tool0";

  aco.object.header.frame_id = "world";

  aco.object.id = "cylinder1";

  shape_msgs::msg::SolidPrimitive prim;
  prim.type = prim.CYLINDER;
  prim.dimensions ={hC, rC};

  Pose pose;
  pose.position.x = x;
  pose.position.y = y;
  pose.position.z = z;

  pose.orientation.w = 1.0;

  aco.object.primitives.push_back(prim);
  aco.object.primitive_poses.push_back(pose);
  aco.object.operation = aco.object.ADD;

  aco.touch_links = {
    "left_finger",
    "right_finger",
    "gripper_base_link",
    "tool0"
  };

  planScene.applyAttachedCollisionObjects({aco});
  RCLCPP_INFO(node->get_logger(), "Cylinder added as the collision object!");
  rclcpp::sleep_for(std::chrono::milliseconds(500));
}

void detachCylinder(PlanningScene& psi, const std::shared_ptr<rclcpp::Node>& node)
{
  moveit_msgs::msg::AttachedCollisionObject aco;
  aco.link_name = "tool0";
  aco.object.id = "cylinder1";

  aco.object.operation = moveit_msgs::msg::CollisionObject::REMOVE;
  psi.applyAttachedCollisionObjects({aco});

  psi.removeCollisionObjects({"cylinder1"});
  RCLCPP_INFO(node->get_logger(), "Cylinder removed as the collision object!");
  rclcpp::sleep_for(std::chrono::milliseconds(500));
}

void runExecutor(rclcpp::executors::SingleThreadedExecutor* executor)
{
  executor->spin();
}


int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("control_node");
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spinner(runExecutor, &executor);

  MoveGroup arm(node, "arm");
  MoveGroup gripper(node, "gripper");
  PlanningScene planning_scene;

  arm.setPoseReferenceFrame("base_link");
  arm.setPlanningTime(15.0);
  arm.setMaxVelocityScalingFactor(0.2);
  arm.setMaxAccelerationScalingFactor(0.2);
  arm.setGoalPositionTolerance(0.01);
  arm.setGoalOrientationTolerance(0.05);
  gripper.setGoalJointTolerance(0.002);

  std::vector<double> upright_pose = {0.0, -1.047, 1.57, 0.0, 0.0, 0.0};

  Pose pAabove = createPose(xCyl, yCyl, 0.35, 0.0, M_PI, 0.0);
  Pose pApre = createPose(xCyl, yCyl, 0.20, 0.0, M_PI, 0.0);
  Pose pAgrasp = createPose(xCyl, yCyl, 0.13, 0.0, M_PI, 0.0);

  Pose pBabove = createPose(-xCyl, -yCyl, 0.35, 0.0, M_PI, M_PI);
  Pose pBpre = createPose(-xCyl, -yCyl, 0.20, 0.0, M_PI, M_PI);
  Pose pBgrasp = createPose(-xCyl, -yCyl, 0.13, 0.0, M_PI, M_PI);

  std::vector<double> gripper_open = {0.005, 0.005};
  std::vector<double> gripper_close = {0.015, 0.015};

  MoveGroup::Plan plan;

  std::vector<Pose> cartesianWaypoints;
  moveit_msgs::msg::RobotTrajectory trajectory;
  const double stepSize = 0.005;
  double fraction = 0.0;
  std::vector<double> currentJoints;

  moveit_msgs::msg::CollisionObject table;
  table.header.frame_id = "world";
  table.id = "table";
  shape_msgs::msg::SolidPrimitive tableBox;
  tableBox.type = tableBox.BOX;
  tableBox.dimensions = {wTab, lTab, hTab};

  Pose tablePose;
  tablePose.position.x = xTab;
  tablePose.position.y = yTab;
  tablePose.position.z = zTab;
  tablePose.orientation.w = 1.0;

  table.primitives.push_back(tableBox);
  table.primitive_poses.push_back(tablePose);
  table.operation = table.ADD;

  planning_scene.applyCollisionObjects({table});
  RCLCPP_INFO(node->get_logger(), "Table added as the collision object!");
  rclcpp::sleep_for(std::chrono::milliseconds(500));

  // Sequence:(0)initial->(1)home ->(2)gripper open ->(3)pAabove
  // ->(4)pApre ->(5)pAgrasp ->(6)close gripper in A
  // ->(7)pApre ->(8)pAabove ->(9)rotate above B ->(10)pBpre
  // (11)pBgrasp->(12)open gripper ->f13)pBpre ->(14) above B
  // (15)home
  arm.setNamedTarget("home");
  if (!planExecute(arm, plan, node, "Home position")) goto codeEnd;

  gripper.setJointValueTarget(gripper_open);
  if (!planExecute(gripper, plan, node, "Open the gripper.")) goto codeEnd;
  rclcpp::sleep_for(std::chrono::milliseconds(1000));

  arm.setJointValueTarget(upright_pose);
  if (!planExecute(arm, plan, node, "Move to upright shoulder pose")) goto codeEnd;

  arm.setPoseTarget(pAabove);
  if (!planExecute(arm, plan, node, "Move above A")) goto codeEnd;

  arm.setPoseTarget(pApre);
  if (!planExecute(arm, plan, node, "Move to pre-grasp A")) goto codeEnd;

  cartesianWaypoints.clear();
  cartesianWaypoints.push_back(pAgrasp);
  fraction = arm.computeCartesianPath(cartesianWaypoints, stepSize, trajectory);

  if (fraction >= 0.95)
  {
    RCLCPP_INFO(node->get_logger(), "Straight line to grasp in A position");
    arm.execute(trajectory);
  } else
  {
    RCLCPP_INFO(node->get_logger(), "PLANNING FAILED: Failed to approach A!");
    goto codeEnd;
  }

  gripper.setJointValueTarget(gripper_close);
  if (!planExecute(gripper, plan, node, "Close the gripper to pick A.")) goto codeEnd;
  rclcpp::sleep_for(std::chrono::milliseconds(1000));

  attachCylinder(planning_scene, xCyl, yCyl, zCyl, rCyl, hCyl, node);
  rclcpp::sleep_for(std::chrono::milliseconds(500));

  arm.setPoseTarget(pApre);
  if (!planExecute(arm, plan, node, "Go straight up to pApre A")) goto codeEnd;

  arm.setPoseTarget(pAabove);
  if (!planExecute(arm, plan, node, "Go to pAabove")) goto codeEnd;

  arm.getCurrentState()->copyJointGroupPositions(arm.getCurrentState()->getRobotModel()->getJointModelGroup("arm"), currentJoints);
  currentJoints[0] += M_PI;
  
  arm.setJointValueTarget(currentJoints);
  if (!planExecute(arm, plan, node, "Rotate to above B")) goto codeEnd;

  arm.setPoseTarget(pBpre);
  if (!planExecute(arm, plan, node, "move to pBpre")) goto codeEnd;

  arm.setPoseTarget(pBgrasp);
  if (!planExecute(arm, plan, node, "Approach to pBgrasp")) goto codeEnd;

  gripper.setJointValueTarget(gripper_open);
  if (!planExecute(gripper, plan, node, "Open the gripper at B")) goto codeEnd;
  rclcpp::sleep_for(std::chrono::milliseconds(1000));

  detachCylinder(planning_scene, node);
  rclcpp::sleep_for(std::chrono::milliseconds(500));

  cartesianWaypoints.clear();
  cartesianWaypoints.push_back(pBpre);

  fraction = arm.computeCartesianPath(cartesianWaypoints, stepSize, trajectory);

  if (fraction >= 0.95) {
    RCLCPP_INFO(node->get_logger(), "Go to Bpre");
    arm.execute(trajectory);
  } else {
    RCLCPP_ERROR(node->get_logger(), "PLANNING FAILED: Go to Bpre");
    goto codeEnd;
  }

  arm.setPoseTarget(pBabove);
  if (!planExecute(arm, plan, node, "Go above B")) goto codeEnd;

  arm.setNamedTarget("home");
  if (!planExecute(arm, plan, node, "Back to home")) goto codeEnd;

  RCLCPP_INFO(node->get_logger(), "Completed");

codeEnd:
  // Shut down
  rclcpp::shutdown();
  spinner.join(); 
  return 0;
}
