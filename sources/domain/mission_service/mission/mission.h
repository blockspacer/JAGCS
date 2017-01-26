#ifndef MISSION_H
#define MISSION_H

// Internal
#include "mission_item.h"

namespace domain
{
    class MissionVehicle;
    class Vehicle;

    class Mission: public QObject
    {
        Q_OBJECT

    public:
        explicit Mission(QObject* parent = nullptr);
        ~Mission() override;

        int count() const;

        MissionItem* item(int seq) const;
        const QList<MissionItem*>& items() const;
        int sequence(MissionItem* item) const;

        MissionItem* take(int seq);

        MissionVehicle* assignment() const;
        Vehicle* assignedVehicle() const;

    public slots:
        void setCount(int count);
        void setMissionItem(int seq, MissionItem* item);

        void addNewMissionItem();
        void removeMissionItem(MissionItem* item);

        void exchange(int first, int last);

        void unassignVehicle();
        void assignVehicle(Vehicle* vehicle);

    signals:
        void missionItemsChanged(const QList<MissionItem*>& items);

    private:
        QList<MissionItem*> m_items;
        MissionVehicle* const m_assignment;
    };
}

#endif // MISSION_H
