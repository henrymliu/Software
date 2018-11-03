#pragma once

#include "backend_output/backend.h"
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

    ~MrfBackend();
private:
    MRFDongle& dongle;
};
