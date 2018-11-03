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
	// for (auto p : primitives)
	{
	    // Encode each primitive and submit transfer through the dongle.
		// dongle->
	}
}

