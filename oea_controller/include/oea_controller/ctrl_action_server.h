#ifndef CTRL_ACTION_SERVER_H
#define CTRL_ACTION_SERVER_H

#include <actionlib/server/simple_action_server.h>
#include "oea_controller/oea_controller.h"
#include <oea_controller/controlPlatformAction.h>

#include <ros/ros.h>


class ControlAction
{
protected:
	oea_controller::TOEAController controller_;
    oea_controller::controlPlatformResult ctrl_action_result_;
public:
    actionlib::SimpleActionServer<oea_controller::controlPlatformAction> as_;
    std::string action_name_;
    bool control_finished_;
    std::string logger_name_;
private:
    ros::Timer FActionTimer;
public:
    ControlAction(std::string name, ros::NodeHandle &n) : controller_(n),
        as_(n, name, false),
        //as_(n, name, boost::bind(&ControlAction::executeCB, this, _1), false),
        action_name_(name)
    {

        logger_name_ = "control";
        // no need to set the level because was already set on oea_controller.cpp

        ROS_DEBUG_STREAM_NAMED(logger_name_, "Starting Control Action Server: " << name);

        as_.registerGoalCallback(boost::bind(&ControlAction::goalCB, this));
        as_.registerPreemptCallback(boost::bind(&ControlAction::preemptCB, this));

        as_.start();
        ROS_DEBUG_STREAM_NAMED(logger_name_, "Control Server Started: " << name);
        control_finished_ = false;

        FActionTimer=n.createTimer(ros::Duration(0.5), &ControlAction::poll_timerCB, this);
        FActionTimer.start();

    }

    ~ControlAction(void){}

    //poll controller to know if it finished (and not block it)
    void poll_timerCB(const ros::TimerEvent& event)
    {

        if(controller_.control_done_)
        {
            if (as_.isActive()) // only set if using action goals
            {
                controller_.control_done_ = false;

                ctrl_action_result_.result_state = controller_.result_state_;
                ctrl_action_result_.error_string = controller_.result_str_;

                if (ctrl_action_result_.result_state)
                {
                    ROS_INFO_STREAM_NAMED(logger_name_, "Action finished OK: " << ctrl_action_result_.error_string);
                    as_.setSucceeded(ctrl_action_result_);
                }
                else
                {
                    ROS_ERROR_STREAM_NAMED(logger_name_, "Action finished NOK: " << ctrl_action_result_.error_string);
                    as_.setAborted(ctrl_action_result_);
                }
                return;
            }
        }
    }


    // Callback for new goals received
    void goalCB()
    {
        ROS_DEBUG_STREAM_NAMED(logger_name_, "Controller Received a New goal");

        controller_.stop_robot("a new goal was received");

        std::string error_str;
        oea_msgs::Oea_path planned_path;
        planned_path = as_.acceptNewGoal()->plan_goal; //TODO (mudar action file (tirar .path)

        // just in case we received an invalid path
        if (planned_path.path.poses.size()<1)
        {
            error_str = "Path received has 0 poses!";
            ctrl_action_result_.result_state = false;
            ctrl_action_result_.error_string = error_str;
            ROS_WARN_STREAM_NAMED(logger_name_, error_str);
            as_.setAborted(ctrl_action_result_);
            return;
        }
        else
        {
            if (as_.isPreemptRequested() ||!ros::ok())
            {
                controller_.stop_robot("goal was preempted");
                ctrl_action_result_.result_state = false;
                ctrl_action_result_.error_string = "Preempt Requested after sending goal";
                as_.setPreempted();
                ROS_WARN_STREAM_NAMED(logger_name_, ctrl_action_result_.error_string);
            }

            controller_.processActionPlan(planned_path);
            return;
        }
    }

    // Callback for preemption requests
    void preemptCB()
    {
        controller_.stop_robot("goal was preempted");
        std::string error_str = "Platform Preempt Request";
        ROS_WARN_STREAM_NAMED(logger_name_, error_str);

        ctrl_action_result_.result_state = false;
        ctrl_action_result_.error_string = error_str;
        as_.setPreempted(ctrl_action_result_);
        return;
    }
};


#endif //CTRL_ACTION_SERVER_H

