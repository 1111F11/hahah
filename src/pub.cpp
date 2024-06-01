#include "ros/ros.h"
#include "can_msgs/Frame.h"
//#include "socketcan_bridge/auto_ware.h"
#include <geometry_msgs/Twist.h>
#include <ros/subscriber.h>
#include <std_msgs/Bool.h>

uint8_t some_data[8]={0x00, 0x01, 0x25, 0x25, 0x00, 0x00, 0x00, 0x00};
bool stop_robot = false;

//void process_cmd_vel(const socketcan_bridge::auto_ware::ConstPtr& msg)
void process_cmd_vel(const geometry_msgs::Twist::ConstPtr& msg)
{
    double linear_velocity = msg->linear.x; // m/s
    double angular_velocity = -msg->angular.z; // rad/s//2024.3.28增加了负号
    // double linear_velocity = msg->accel_cmd;    // 线速度 (m/s)
    // double angular_velocity = msg->steer_cmd;  // 角速度 (rad/s)
    int16_t linear_velocity_cmd = static_cast<int16_t>(linear_velocity * 100);  // 转换为cm/s
    int16_t angular_velocity_cmd = static_cast<int16_t>(angular_velocity * 10 * 180 / M_PI);  // 转换为0.1°/s
    some_data[0] = 0x00;
    some_data[1] = 0x01;
    some_data[2] = 0x25;
    some_data[3] = 0x25;
    some_data[4] = linear_velocity_cmd & 0xFF;
    some_data[5] = (linear_velocity_cmd >> 8) & 0xFF;
    some_data[6] = angular_velocity_cmd & 0xFF;
    some_data[7] = (angular_velocity_cmd >> 8) & 0xFF;
}



void process_stop(const std_msgs::Bool::ConstPtr& msg)
{
    stop_robot = msg->data;
    if (stop_robot) {
        // 如果接收到停止指令，将速度设置为0
        some_data[4] = 0;
        some_data[5] = 0;
        some_data[6] = 0;
        some_data[7] = 0;
    }
}

int main(int argc, char *argv[])
{
  ros::init(argc, argv, "pub");
  ros::NodeHandle nh("");
  ros::Subscriber sub = nh.subscribe("/cmd_vel", 10, process_cmd_vel);  // 订阅/cmd_vel话题
  ros::Subscriber stop_sub = nh.subscribe("/stop", 10, process_stop);  // 订阅/stop话题
  ros::Publisher pub = nh.advertise<can_msgs::Frame>("sent_messages", 10);  // 发布can消息
  ros::Rate loop_rate(10);  // 设置发布频率为70Hz
  // can_msgs::Frame msg;
  // msg.id = 0x0002;  // 设置can id
  // msg.is_rtr = false;
  // msg.is_extended = false;
  // msg.is_error = false;
  // msg.dlc = 8;
  // msg.data = {0x2b,0x0f,0x20,00,0x01,00,00,00};
  // pub.publish(msg);
  // ros::Duration(0.2).sleep();
  while (ros::ok())
  {
    can_msgs::Frame msg;
    msg.id = 0x0002;  // 设置can id
    msg.is_rtr = false;
    msg.is_extended = false;
    msg.is_error = false;
    msg.dlc = 8;
    ros::spinOnce();
    msg.data = {some_data[0], some_data[1], some_data[2], some_data[3], some_data[4], some_data[5], some_data[6], some_data[7]};  // 设置can数据
    pub.publish(msg);
    loop_rate.sleep();
    some_data[0] = 0x00;//命令字
    some_data[1] = 0x01;//对象索引1
    some_data[2] = 0x25;//对象索引2
    some_data[3] = 0x25;//
    some_data[4] = 0x00;
    some_data[5] = 0x00;
    some_data[6] = 0x00;
    some_data[7] = 0x00;
    //ros::Duration(0.05).sleep();
  }
  return 0;
}
