#include "ros/ros.h"
#include "socketcan_bridge/auto_ware.h"
#include "sensor_msgs/Joy.h"

ros::Publisher pub;
ros::Subscriber sub;
socketcan_bridge::auto_ware auto_ware_msg;
double max_accel_cmd = 0.3;
int accel_cmd_key, steer_cmd_key, add_key, reduce_key;

void chatterCallback(const sensor_msgs::Joy::ConstPtr& joy_msg) {
    // 当收到手柄加档按键信号且最大速度小于1.2m/s，最大线速度增加0.3m/s
    if (joy_msg->buttons[add_key] == true && max_accel_cmd < 1.2) {
        max_accel_cmd = max_accel_cmd + 0.3;
        ROS_INFO("max_accel_cmd=%f", max_accel_cmd);
    }

    // 当收到手柄减档按键信号且最大速度大于0.3m/s，最大线速度减小0.3m/s
    if (joy_msg->buttons[reduce_key] == true && max_accel_cmd > 0.3) {
        max_accel_cmd = max_accel_cmd - 0.3;
        ROS_INFO("max_accel_cmd=%f", max_accel_cmd);
    }

    // 最大线速度根据档位变化，最大角速度为固定值0.5rad/s
    auto_ware_msg.accel_cmd = joy_msg->axes[accel_cmd_key] * max_accel_cmd;
    auto_ware_msg.steer_cmd = joy_msg->axes[steer_cmd_key] * 0.5;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "sr_joy");
    ros::NodeHandle n;
    int frequency;

    ros::param::get("~add_key", add_key);
    ros::param::get("~reduce_key", reduce_key);
    ros::param::get("~accel_cmd_key", accel_cmd_key);
    ros::param::get("~steer_cmd_key", steer_cmd_key);
    ros::param::get("~frequency", frequency);

    pub = n.advertise<socketcan_bridge::auto_ware>("/auto_ware", 100);
    sub = n.subscribe("/joy", 1000, chatterCallback);

    ros::Rate loop(frequency);
    bool stop = false;  // 手柄数据非0标志位 false:数据为非0 true：数据为0

    while (ros::ok()) {
        // 手柄未拨动摇杆，joy_node节点持续发布数据为0的/joy话题；
        // 增加该判断可以在导航move_base节点发布cmd_vel话题的同时，sr_joy_node节点不发布cmd_vel话题
        // 当需要手柄控制底盘时，只要保证该frequency频率远大于move_base发布的cmd_vel即可
        if (stop == true && fabs(auto_ware_msg.accel_cmd) == 0.0 && fabs(auto_ware_msg.steer_cmd) == 0.0) {
            stop = false;
            loop.sleep();
            ros::spinOnce();
            continue;
        }

        if (stop == false && fabs(auto_ware_msg.accel_cmd) == 0.0 && fabs(auto_ware_msg.steer_cmd) == 0.0) {
            stop = true;
        }

        pub.publish(auto_ware_msg);
        loop.sleep();
        ros::spinOnce();
    }

    return 0;
}