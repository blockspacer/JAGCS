#ifndef HOME_POSITION_HANDLER_H
#define HOME_POSITION_HANDLER_H

#include "abstract_mavlink_handler.h"

namespace domain
{
    class VehicleService;
    class Vehicle;
    class Position;

    class HomePositionHandler: public AbstractMavLinkHandler
    {
    public:
        HomePositionHandler(VehicleService* vehicleService,
                            MavLinkCommunicator* communicator);

    public slots:
        void processMessage(const mavlink_message_t& message) override;

        void sendHomePositionRequest(Vehicle* vehicle);
        void sendHomePositionSetting(Vehicle* vehicle, const Position& position);

    private:
        VehicleService* m_vehicleService;
    };
}

#endif // HOMEPOSITIONHANDLER_H
