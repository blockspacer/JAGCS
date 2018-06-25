#include "bluetooth_service.h"

// Qt
#include <QBluetoothLocalDevice>
#include <QPointer>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QMap>
#include <QDebug>

// Internal
#include "notification_bus.h"

using namespace domain;

class BluetoothService::Impl
{
public:
    QBluetoothLocalDevice* localDevice;
    QPointer<QBluetoothDeviceDiscoveryAgent> agent;
    QMap<QString, QBluetoothDeviceInfo> deviceInfos;
};

BluetoothService::BluetoothService(QObject* parent):
    QObject(parent),
    d(new Impl())
{
    d->localDevice = new QBluetoothLocalDevice(this);
}

BluetoothService::~BluetoothService()
{
    this->stopDiscovery();
}

bool BluetoothService::isAvailable() const
{
    return d->localDevice->isValid();
}

QStringList BluetoothService::discoveredDevices() const
{
    return d->deviceInfos.keys();
}

bool BluetoothService::isDiscoveryActive() const
{
    return d->agent && d->agent->isActive();
}

QString BluetoothService::deviceAddress(const QString& name) const
{
    return d->deviceInfos.value(name).address().toString();
}

void BluetoothService::startDiscovery()
{
    if (d->agent) this->stopDiscovery();

    d->agent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(d->agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothService::onDeviceDiscovered);
    connect(d->agent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothService::onStopped);
    connect(d->agent, &QBluetoothDeviceDiscoveryAgent::canceled,
            this, &BluetoothService::onStopped);
    connect(d->agent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(
                &QBluetoothDeviceDiscoveryAgent::error), this, &BluetoothService::onError);

    d->agent->setInquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);
    d->agent->start();
}

void BluetoothService::stopDiscovery()
{
    if (d->agent) d->agent->stop();
}

void BluetoothService::onDeviceDiscovered(const QBluetoothDeviceInfo& info)
{
    if (!info.isValid()) return;

    d->deviceInfos[info.name()] = info;
    emit deviceDiscovered(info.name());
}

void BluetoothService::onStopped()
{
    d->agent->deleteLater();

    emit stopped();
}

void BluetoothService::onError(int error)
{
    switch (error)
    {
    case QBluetoothDeviceDiscoveryAgent::NoError:
        break;
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        notificationBus->notify(tr("Bluetooth service"),
                                tr("The Bluetooth adaptor is powered off"),
                                dto::Notification::Warning);
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        notificationBus->notify(tr("Bluetooth service"),
                                tr("Writing or reading from the device resulted in an error"),
                                dto::Notification::Warning);
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        notificationBus->notify(tr("Bluetooth service"),
                                tr("The passed local adapter address does not match the "
                                   "physical adapter address of any local Bluetooth device"),
                                dto::Notification::Warning);
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        notificationBus->notify(tr("Bluetooth service"),
                                tr("Device discovery is not possible or implemented "
                                   "on the current platform"),
                                dto::Notification::Warning);
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
        notificationBus->notify(tr("Bluetooth service"),
                                tr("One of the requested discovery methods is "
                                   "not supported by the current platform"),
                                dto::Notification::Warning);
    default:
    case QBluetoothDeviceDiscoveryAgent::UnknownError:
        notificationBus->notify(tr("Bluetooth service"),
                                tr("An unknown error has occurred"),
                                dto::Notification::Warning);
        break;
    }
}
