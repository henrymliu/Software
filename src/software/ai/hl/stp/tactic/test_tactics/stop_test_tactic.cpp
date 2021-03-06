#include "software/ai/hl/stp/tactic/test_tactics/stop_test_tactic.h"

#include "software/ai/hl/stp/action/stop_action.h"

StopTestTactic::StopTestTactic(bool loop_forever) : Tactic(loop_forever) {}

std::string StopTestTactic::getName() const
{
    return "Stop Test Tactic";
}

double StopTestTactic::calculateRobotCost(const Robot &robot, const World &world)
{
    // Prefer all robots equally with a cost of 0.5
    return 0.5;
}

void StopTestTactic::calculateNextAction(ActionCoroutine::push_type &yield)
{
    do
    {
        yield(std::make_shared<StopAction>());
    } while (this->robot->velocity().length() > 0.05);
}

void StopTestTactic::accept(TacticVisitor &visitor) const
{
    // StopTestTactic is meant to be a simple test tactic and so
    // we invoke YAGNI to not implement the visitor for this tactic
    throw std::invalid_argument(
        "Error: Tactic Visitor does not implement visiting this Tactic, so this accept function does nothing");
}
