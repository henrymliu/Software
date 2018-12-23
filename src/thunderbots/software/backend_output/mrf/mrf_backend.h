#pragma once

#include "backend_output/backend.h"
#include "ai/world/ball.h"
#include "ai/world/team.h"
#include "dongle.h"
#include <limits>

class MrfBackend : public Backend
{
public:
    /**
     * Creates new mrf backend, using the dongle class.
     * Automatically connects to the dongle upon initialization.
     */
    explicit MrfBackend(MRFDongle& dongle);

    /**
     * Sends the given primitives to the backend to control the robots
     *
     * @param primitives the list of primitives to send
     */
    void sendPrimitives(
        const std::vector<std::unique_ptr<Primitive>> &primitives) override;

    void update_detbots(std::vector<std::tuple<uint8_t, Point, Angle>> ft);
    void update_ball(Ball b);

    void send_vision_packet();

    ~MrfBackend();
private:
    MRFDongle& dongle;
    std::vector<std::tuple<uint8_t, Point, Angle>> detbots;
    Ball ball;
};
