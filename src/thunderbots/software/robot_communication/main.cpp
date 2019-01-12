#include <ros/ros.h>
#include <ros/time.h>
#include <thunderbots_msgs/Ball.h>
#include <thunderbots_msgs/Primitive.h>
#include <thunderbots_msgs/PrimitiveArray.h>
#include <thunderbots_msgs/Team.h>

#include <iostream>
#include <random>
#include <thread>

#include "ai/primitive/move_primitive.h"
#include "ai/primitive/primitive.h"
#include "geom/point.h"
#include "mrf/mrf_backend.h"
#include "robot_communication/grsim/grsim_backend.h"
#include "util/constants.h"
#include "util/logger/init.h"
#include "util/ros_messages.h"

// Constants
const std::string NETWORK_ADDRESS       = "127.0.0.1";
static constexpr short NETWORK_PORT     = 20011;
static constexpr unsigned int TICK_RATE = 30;

// Member variables we need to maintain state

// A vector of primitives. It is cleared each tick, populated by the callbacks
// that receive primitive commands, and is processed by the backend to send
// the primitives to the system we have chosen (such as grSim, our radio, etc.)
std::vector<std::unique_ptr<Primitive>> primitives;
MRFDongle dongle   = MRFDongle();
MrfBackend backend = MrfBackend(dongle);

namespace
{
    Team friendly_team = Team(std::chrono::milliseconds(1000));
}

// Callbacks
void primitiveUpdateCallback(const thunderbots_msgs::PrimitiveArray::ConstPtr& msg)
{
    thunderbots_msgs::PrimitiveArray prim_array_msg = *msg;
    for (const thunderbots_msgs::Primitive& prim_msg : prim_array_msg.primitives)
    {
        primitives.emplace_back(Primitive::createPrimitive(prim_msg));
    }
}

void ballUpdateCallback(const thunderbots_msgs::Ball::ConstPtr& msg)
{
    thunderbots_msgs::Ball ball_msg = *msg;

    Ball ball = Util::ROSMessages::createBallFromROSMessage(ball_msg);
    backend.update_ball(ball);

    // Send vision packet
    // TODO test this; I have a feeling that it may be
    // better to have a combined ball + team callback
    backend.send_vision_packet();
}

void friendlyTeamUpdateCallback(const thunderbots_msgs::Team::ConstPtr& msg)
{
    thunderbots_msgs::Team friendly_team_msg = *msg;

    friendly_team = Util::ROSMessages::createTeamFromROSMessage(friendly_team_msg);

    std::vector<std::tuple<uint8_t, Point, Angle>> detbots;
    for (const Robot& r : friendly_team.getAllRobots())
    {
        detbots.push_back(std::make_tuple(r.id(), r.position(), r.orientation()));
    }
    backend.update_detbots(detbots);

    // Send vision packet
    backend.send_vision_packet();
}

int main(int argc, char** argv)
{
    // Init ROS node
    ros::init(argc, argv, "robot_communication");
    ros::NodeHandle node_handle;

    // Create subscribers to topics we care about
    ros::Subscriber prim_array_sub = node_handle.subscribe(
        Util::Constants::AI_PRIMITIVES_TOPIC, 1, primitiveUpdateCallback);
    ros::Subscriber friendly_team_sub =
        node_handle.subscribe(Util::Constants::NETWORK_INPUT_FRIENDLY_TEAM_TOPIC, 1,
                              friendlyTeamUpdateCallback);
    ros::Subscriber ball_sub = node_handle.subscribe(
        Util::Constants::NETWORK_INPUT_BALL_TOPIC, 1, ballUpdateCallback);

    // Initialize the logger
    Util::Logger::LoggerSingleton::initializeLogger();

    // Initialize variables
    primitives = std::vector<std::unique_ptr<Primitive>>();
    // GrSimBackend backend = GrSimBackend(NETWORK_ADDRESS, NETWORK_PORT);

    // We loop at a set rate so that we don't overload the network with too many packets
    ros::Rate tick_rate(TICK_RATE);

    // Main loop
    while (ros::ok())
    {
        // Clear all primitives each tick
        primitives.clear();
        primitives.emplace_back(
            std::make_unique<MovePrimitive>(1, Point(std::rand() % 4, std::rand() % 4),
                                            Angle::ofDegrees(std::rand() % 360), 3));

        // Send primitives
        backend.sendPrimitives(primitives);

        // Handle libusb events for the dongle
        dongle.handle_libusb_events();

        // Spin once to let all necessary callbacks run
        // The callbacks will populate the primitives vector
        ros::spinOnce();

        // backend.updateBackendTeam(friendly_team);

        backend.sendPrimitives(primitives);

        tick_rate.sleep();
    }

    return 0;
}
