#ifndef OEA_PLANNER_ROS_H
#define OEA_PLANNER_ROS_H

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt8.h>
#include <geometry_msgs/PoseStamped.h>
#include <nav_msgs/OccupancyGrid.h> // map
#include <nav_msgs/Path.h>
#include <tf/transform_listener.h>
#include <sensor_msgs/Joy.h>
#include <sensor_msgs/PointCloud2.h>
#include <mission_ctrl_msgs/mission_ctrl_defines.h>
#include <oea_planner/isPoseValid.h>
#include <oea_planner/astar.h>


/*#define IDLE 0;
#define BUSY 1;
#define ERROR 2;*/
//#define PLANNING 1; // for server to know
//#define LOADING_MAP 3; // for server to know


class TOea_Planner
{
public:
    /*explicit*/ TOea_Planner(ros::NodeHandle &n, ros::NodeHandle &private_n, std::string logger_name);
    ~TOea_Planner();
    int exec();
    bool executeCycle(std::string &error_str, nav_msgs::Path &planned_path);
    TAstar Astar_;
    std_msgs::UInt8 planner_state;
    ros::Publisher state_pub_; // planner state publisher
    ros::ServiceServer ss_;
    bool map_received_;
    std::string logger_name_;
private:
    std::string global_frame_id, base_frame_id;

    // ** publishers:
    ros::Publisher pcd_pub_; // inflation point cloud publisher
    ros::Publisher path_pub_, visual_path_pub_; // path publisher for controller (topic) / and to view in both topic and action (visual)
    ros::Publisher arrows_pub_; // arrows publisher

    // ** subscribers:
    ros::Subscriber map_sub_; // map subscriber
    ros::Subscriber goal_sub_; // goal subscriber

    // ** Callbacks:
    void goalCB(const geometry_msgs::PoseStamped::ConstPtr& goal_msg);
    void mapCB(const nav_msgs::OccupancyGrid::ConstPtr& map_msg);

    // other functions
	// ** Services:
    bool IsPoseValid(oea_planner::isPoseValid::Request& req, oea_planner::isPoseValid::Response& res);


};

#endif // OEA_PLANNER_ROS_H
