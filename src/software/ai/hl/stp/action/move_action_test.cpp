#include "software/ai/hl/stp/action/move_action.h"

#include <gtest/gtest.h>

#include "software/ai/intent/move_intent.h"

TEST(MoveActionTest, getDestination)
{
    Robot robot       = Robot(13, Point(1, 2), Vector(3, 4), Angle::fromDegrees(5),
                        AngularVelocity::fromDegrees(6), Timestamp::fromSeconds(7));
    MoveAction action = MoveAction(0.05, Angle(), false);

    action.updateControlParams(robot, Point(11, 12), Angle::quarter(), 1.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);

    EXPECT_EQ(Point(11, 12), action.getDestination());
}

TEST(MoveActionTest, getFinalOrientation)
{
    Robot robot       = Robot(13, Point(1, 0), Vector(3, 4), Angle::fromDegrees(5),
                        AngularVelocity::fromDegrees(6), Timestamp::fromSeconds(7));
    MoveAction action = MoveAction(0.05, Angle(), false);

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 1.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);

    EXPECT_EQ(Angle::quarter(), action.getFinalOrientation());
}

TEST(MoveActionTest, getFinalSpeed)
{
    Robot robot       = Robot(13, Point(1, 2), Vector(3, 4), Angle::fromDegrees(5),
                        AngularVelocity::fromDegrees(6), Timestamp::fromSeconds(7));
    MoveAction action = MoveAction(0.05, Angle(), false);

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 99.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);

    EXPECT_EQ(99, action.getFinalSpeed());
}

TEST(MoveActionTest, getAutoKickType)
{
    Robot robot       = Robot(13, Point(1, 2), Vector(3, 4), Angle::fromDegrees(5),
                        AngularVelocity::fromDegrees(6), Timestamp::fromSeconds(7));
    MoveAction action = MoveAction();

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 99.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);

    EXPECT_EQ(AutokickType::NONE, action.getAutoKickType());

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 99.0,
                               DribblerEnable::OFF, MoveType::NORMAL,
                               AutokickType::AUTOCHIP, BallCollisionType::AVOID);

    EXPECT_EQ(AutokickType::AUTOCHIP, action.getAutoKickType());
}

TEST(MoveActionTest, getDribblerEnabled)
{
    Robot robot       = Robot(13, Point(1, 2), Vector(3, 4), Angle::fromDegrees(5),
                        AngularVelocity::fromDegrees(6), Timestamp::fromSeconds(7));
    MoveAction action = MoveAction();

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 99.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);

    EXPECT_EQ(DribblerEnable::OFF, action.getDribblerEnabled());

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 99.0,
                               DribblerEnable::ON, MoveType::NORMAL,
                               AutokickType::AUTOCHIP, BallCollisionType::AVOID);

    EXPECT_EQ(DribblerEnable::ON, action.getDribblerEnabled());
}

TEST(MoveActionTest, robot_far_from_destination)
{
    Robot robot = Robot(0, Point(), Vector(), Angle::zero(), AngularVelocity::zero(),
                        Timestamp::fromSeconds(0));
    MoveAction action = MoveAction(0.05, Angle(), false);

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 1.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);
    auto intent_ptr = action.getNextIntent();

    // Check an intent was returned (the pointer is not null)
    EXPECT_TRUE(intent_ptr);
    EXPECT_FALSE(action.done());

    MoveIntent move_intent = dynamic_cast<MoveIntent &>(*intent_ptr);
    EXPECT_EQ(0, move_intent.getRobotId());
    EXPECT_EQ(Point(1, 0), move_intent.getDestination());
    EXPECT_EQ(Angle::quarter(), move_intent.getFinalAngle());
    EXPECT_EQ(1.0, move_intent.getFinalSpeed());
    EXPECT_FALSE(move_intent.getDribblerEnable() == DribblerEnable::ON);
    EXPECT_EQ(move_intent.getAutoKickType(), NONE);
}

TEST(MoveActionTest, robot_at_destination)
{
    Robot robot = Robot(0, Point(), Vector(0, 0), Angle::zero(), AngularVelocity::zero(),
                        Timestamp::fromSeconds(0));
    MoveAction action = MoveAction(0.02, Angle(), false);

    // We call the action twice. The first time the Intent will always be returned to
    // ensure the Robot is doing the right thing. In all future calls, the action will be
    // done and so will return a null pointer
    action.updateControlParams(robot, Point(0, 0), Angle::zero(), 0.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);
    action.getNextIntent();
    action.getNextIntent();

    EXPECT_TRUE(action.done());
}

TEST(MoveActionTest, test_action_does_not_prematurely_report_done)
{
    Robot robot = Robot(0, Point(), Vector(), Angle::zero(), AngularVelocity::zero(),
                        Timestamp::fromSeconds(0));
    MoveAction action = MoveAction(0.05, Angle(), false);

    // Run the Action several times
    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 1.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);
    for (int i = 0; i < 10; i++)
    {
        action.getNextIntent();
    }
    auto intent_ptr = action.getNextIntent();

    // Check an intent was returned (the pointer is not null)
    EXPECT_TRUE(intent_ptr);
    EXPECT_FALSE(action.done());
}

TEST(MoveActionTest, test_action_does_not_prematurely_report_done_angle_threshold)
{
    Robot robot = Robot(0, Point(), Vector(), Angle::zero(), AngularVelocity::zero(),
                        Timestamp::fromSeconds(0));
    MoveAction action = MoveAction(0.05, Angle::fromDegrees(0.01), false);

    // Run the Action several times
    action.updateControlParams(robot, Point(0, 0), Angle::quarter(), 1.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);
    for (int i = 0; i < 10; i++)
    {
        action.getNextIntent();
    }
    auto intent_ptr = action.getNextIntent();

    // Check an intent was returned (the pointer is not null)
    EXPECT_TRUE(intent_ptr);
    EXPECT_FALSE(action.done());
}
TEST(MoveActionTest, test_action_finishes_within_orientation_threshold)
{
    Robot robot = Robot(0, Point(0, 0), Vector(), Angle::zero(), AngularVelocity::zero(),
                        Timestamp::fromSeconds(0));
    MoveAction action = MoveAction(0.05, Angle::fromDegrees(359), false);

    // Run the Action several times
    action.updateControlParams(robot, Point(0, 0), Angle::quarter(), 1.0,
                               DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);
    for (int i = 0; i < 10; i++)
    {
        action.getNextIntent();
    }
    auto intent_ptr = action.getNextIntent();

    // Check an intent was returned (the pointer is not null)
    EXPECT_FALSE(intent_ptr);
    EXPECT_TRUE(action.done());
}

TEST(MoveActionTest, robot_far_from_destination_autokick_turned_on)
{
    Robot robot = Robot(0, Point(), Vector(), Angle::zero(), AngularVelocity::zero(),
                        Timestamp::fromSeconds(0));
    MoveAction action = MoveAction(0.05, Angle(), false);

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 1.0,
                               DribblerEnable::OFF, MoveType::NORMAL,
                               AutokickType::AUTOKICK, BallCollisionType::AVOID);
    auto intent_ptr = action.getNextIntent();

    // Check an intent was returned (the pointer is not null)
    EXPECT_TRUE(intent_ptr);
    EXPECT_FALSE(action.done());

    MoveIntent move_intent = dynamic_cast<MoveIntent &>(*intent_ptr);
    EXPECT_EQ(0, move_intent.getRobotId());
    EXPECT_EQ(Point(1, 0), move_intent.getDestination());
    EXPECT_EQ(Angle::quarter(), move_intent.getFinalAngle());
    EXPECT_EQ(1.0, move_intent.getFinalSpeed());
    EXPECT_FALSE(move_intent.getDribblerEnable());
    EXPECT_EQ(move_intent.getAutoKickType(), AUTOKICK);
}

TEST(MoveActionTest, robot_far_from_destination_dribble_turned_on)
{
    Robot robot = Robot(0, Point(), Vector(), Angle::zero(), AngularVelocity::zero(),
                        Timestamp::fromSeconds(0));
    MoveAction action = MoveAction(0.05, Angle(), false);

    action.updateControlParams(robot, Point(1, 0), Angle::quarter(), 1.0,
                               DribblerEnable::ON, MoveType::NORMAL, AutokickType::NONE,
                               BallCollisionType::AVOID);
    auto intent_ptr = action.getNextIntent();

    // Check an intent was returned (the pointer is not null)
    EXPECT_TRUE(intent_ptr);
    EXPECT_FALSE(action.done());

    MoveIntent move_intent = dynamic_cast<MoveIntent &>(*intent_ptr);
    EXPECT_EQ(0, move_intent.getRobotId());
    EXPECT_EQ(Point(1, 0), move_intent.getDestination());
    EXPECT_EQ(Angle::quarter(), move_intent.getFinalAngle());
    EXPECT_EQ(1.0, move_intent.getFinalSpeed());
    EXPECT_TRUE(move_intent.getDribblerEnable() == DribblerEnable::ON);
    EXPECT_EQ(move_intent.getAutoKickType(), NONE);
}
