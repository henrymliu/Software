#include "mrf_backend.h"

#include <chrono>

MrfBackend::MrfBackend(MRFDongle& dongle)
    : dongle(dongle), ball(Ball(Point(0, 0), Vector(0, 0)))
{
}

MrfBackend::~MrfBackend() {}

void MrfBackend::sendPrimitives(const std::vector<std::unique_ptr<Primitive>>& primitives)
{
    dongle.send_drive_packet(primitives);
}

void MrfBackend::update_detbots(std::vector<std::tuple<uint8_t, Point, Angle>> ft)
{
    detbots = ft;
}

void MrfBackend::update_ball(Ball b)
{
    ball = b;
}

void MrfBackend::send_vision_packet()
{
    /* TODO: Change handling of timestamp depending on age of team vs ball */
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                             ball.lastUpdateTimestamp().time_since_epoch())
                             .count();
    std::cout << "Calling dongle.send_camera_packet with: ";
    for (std::size_t i = 0; i < detbots.size(); ++i)
    {
        std::cout << "bot number = " << unsigned(std::get<0>(detbots[i])) << ", ";
        std::cout << "x = " << (std::get<1>(detbots[i])).x() << ", ";
        std::cout << "y = " << (std::get<1>(detbots[i])).y() << ", ";
        std::cout << "time capture = " << timestamp << ", ";
        std::cout << "theta = " << (std::get<2>(detbots[i])).toDegrees() << std::endl;
    }

    dongle.send_camera_packet(detbots, ball.position() * 1000, timestamp);
}
