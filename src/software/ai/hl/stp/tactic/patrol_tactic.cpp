#include "software/ai/hl/stp/tactic/patrol_tactic.h"

#include <g3log/g3log.hpp>

#include "software/ai/hl/stp/action/move_action.h"
#include "software/ai/hl/stp/action/stop_action.h"
#include "software/ai/hl/stp/tactic/tactic_visitor.h"

PatrolTactic::PatrolTactic(const std::vector<Point> &points,
                           double at_patrol_point_tolerance,
                           Angle orientation_at_patrol_points,
                           double linear_speed_at_patrol_points)
    : Tactic(true),
      patrol_points(points),
      at_patrol_point_tolerance(at_patrol_point_tolerance),
      orientation_at_patrol_points(orientation_at_patrol_points),
      linear_speed_at_patrol_points(linear_speed_at_patrol_points),
      patrol_point_index(0)
{
}

std::string PatrolTactic::getName() const
{
    return "Patrol Tactic";
}

double PatrolTactic::calculateRobotCost(const Robot &robot, const World &world)
{
    if (patrol_points.empty())
    {
        return 0.0;
    }
    else if (this->robot)
    {
        // If we already assigned a robot to this tactic, prefer reassigning
        // that robot
        if (this->robot.value() == robot)
        {
            return 0.0;
        }
        else
        {
            return 1.0;
        }
    }
    else
    {
        // Prefer robots that are close to the current patrol point
        double dist = (robot.position() - patrol_points.at(patrol_point_index)).length();
        double cost = dist / world.field().totalXLength();
        return std::clamp<double>(cost, 0, 1);
    }
}

void PatrolTactic::calculateNextAction(ActionCoroutine::push_type &yield)
{
    if (patrol_points.empty())
    {
        auto stop_action = std::make_shared<StopAction>(false, true);
        do
        {
            LOG(WARNING) << "Running a Patrol Tactic with no patrol points" << std::endl;
            stop_action->updateControlParams(*robot, false);
            yield(stop_action);
        } while (true);
    }

    auto move_action =
        std::make_shared<MoveAction>(this->at_patrol_point_tolerance, Angle(), false);
    do
    {
        move_action->updateControlParams(
            *robot, patrol_points.at(patrol_point_index), orientation_at_patrol_points,
            linear_speed_at_patrol_points, DribblerEnable::OFF, MoveType::NORMAL,
            AutokickType::NONE, BallCollisionType::AVOID);
        if (move_action->done())
        {
            patrol_point_index = (patrol_point_index + 1) % patrol_points.size();
            move_action->updateControlParams(
                *robot, patrol_points.at(patrol_point_index),
                orientation_at_patrol_points, linear_speed_at_patrol_points,
                DribblerEnable::OFF, MoveType::NORMAL, AutokickType::NONE,
                BallCollisionType::AVOID);
        }

        yield(move_action);
    } while (true);
}

void PatrolTactic::accept(TacticVisitor &visitor) const
{
    visitor.visit(*this);
}
