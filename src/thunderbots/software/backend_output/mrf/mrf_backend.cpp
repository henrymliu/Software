#include "mrf_backend.h"

MrfBackend::MrfBackend(MRFDongle& dongle) : dongle(dongle)
{

}

MrfBackend::~MrfBackend()
{
}

void MrfBackend::sendPrimitives(
        const std::vector<std::unique_ptr<Primitive>> &primitives)
{
	dongle.build_drive_packet(primitives);
}

