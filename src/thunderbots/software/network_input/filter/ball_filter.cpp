#include "ball_filter.h"

BallFilter::BallFilter() {}

FilteredBallData BallFilter::getFilteredData(std::vector<SSLBallData> new_ball_data)
{
    FilteredBallData data;
    data.position = new_ball_data[0].position;
    data.velocity = Vector();
    data.timestamp = new_ball_data[0].timestamp;
    return data;
}
