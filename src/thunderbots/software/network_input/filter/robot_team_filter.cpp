#include "robot_team_filter.h"

#include <algorithm>
#include <vector>

#include "robot_filter.h"

RobotTeamFilter::RobotTeamFilter() {}

std::vector<FilteredRobotData> RobotTeamFilter::getFilteredData(
    const std::vector<SSLRobotData> &new_team_data)
{
    std::vector<FilteredRobotData> result;

    for(auto data : new_team_data) {
        FilteredRobotData filtered_data;
        filtered_data.position = data.position;
        filtered_data.velocity = Vector();
        filtered_data.id = data.id;
        filtered_data.orientation = data.orientation;
        filtered_data.angular_velocity = AngularVelocity::zero();
        filtered_data.timestamp = data.timestamp;

        result.push_back(filtered_data);
    }

    return result;
}
