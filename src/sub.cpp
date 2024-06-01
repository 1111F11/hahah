#include <ros/ros.h>
#include <cstdlib>
#include "can_msgs/Frame.h"
#include <geometry_msgs/Twist.h>
double robot_width=0.4,wheel_radius=0.085;
ros::Publisher feedback_pub;
void feedback_cmd_vel(const can_msgs::Frame::ConstPtr& msg)
{
    int16_t left_rpm = (msg->data[5] << 8) | msg->data[4];
    int16_t right_rpm = (msg->data[7] << 8) | msg->data[6];
    left_rpm/=10;
    right_rpm/=10;
    double left_wheel_linear_velocity = (left_rpm * 2 * M_PI * wheel_radius) / 60.0;
    double right_wheel_linear_velocity = -(right_rpm * 2 * M_PI * wheel_radius) / 60.0;
    double linear_velocity = (left_wheel_linear_velocity + right_wheel_linear_velocity) / 2.0;
    double angular_velocity = (right_wheel_linear_velocity - left_wheel_linear_velocity) / robot_width;

    // 发布/feedback_cmd_vel话题
    geometry_msgs::Twist feedback_msg;
    // feedback_msg.linear.x = -angular_velocity;
    // feedback_msg.angular.z = linear_velocity;
    feedback_msg.linear.x = linear_velocity;
feedback_msg.angular.z = angular_velocity;//2024.3.8修改了正负号
    feedback_pub.publish(feedback_msg);
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "sub");
    ros::NodeHandle nh("");
    ros::Rate loop_rate(10);
    ros::Subscriber feedback_sub = nh.subscribe("/received_messages", 10, feedback_cmd_vel);  // 订阅/received_messages话题
    feedback_pub = nh.advertise<geometry_msgs::Twist>("/feedback_cmd_vel", 10);
    ros::spin();
    return 0;
}