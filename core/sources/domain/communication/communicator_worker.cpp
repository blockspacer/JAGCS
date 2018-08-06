#include "communicator_worker.h"

// Qt
#include <QTimerEvent>
#include <QTime>
#include <QThread>
#include <QDebug>

// Internal
#include "link_description.h"

#include "abstract_communicator.h"
#include "abstract_link.h"

namespace
{
    const int second = 1000;

    dto::LinkDescription::Protocol toDtoProtocol(data_source::AbstractCommunicator::Protocol protocol)
    {
        switch (protocol) {
        case data_source::AbstractCommunicator::MavLink1: return dto::LinkDescription::MavLink1;
        case data_source::AbstractCommunicator::MavLink2: return dto::LinkDescription::MavLink2;
        default:
        case data_source::AbstractCommunicator::Unknown: return dto::LinkDescription::UnknownProtocol;
        }
    }
}

using namespace data_source;
using namespace domain;

class CommunicatorWorker::Impl
{
public:
    QMap<int, data_source::AbstractLink*> descriptedLinks;
    QList<data_source::AbstractCommunicator*> communicators;

    int statisticsTimer = 0;
};

CommunicatorWorker::CommunicatorWorker(QObject* parent):
    QObject(parent),
    d(new Impl())
{
    connect(this, &CommunicatorWorker::addCommunicator,
            this, &CommunicatorWorker::addCommunicatorImpl);
    connect(this, &CommunicatorWorker::deleteCommunicator,
            this, &CommunicatorWorker::deleteCommunicatorImpl);
    connect(this, &CommunicatorWorker::updateLink,
            this, &CommunicatorWorker::updateLinkImpl);
    connect(this, &CommunicatorWorker::removeLink,
            this, &CommunicatorWorker::removeLinkImpl);
    connect(this, &CommunicatorWorker::setLinkConnected,
            this, &CommunicatorWorker::setLinkConnectedImpl);

    d->statisticsTimer = this->startTimer(::second);
}

CommunicatorWorker::~CommunicatorWorker()
{}

void CommunicatorWorker::onMavLinkStatisticsChanged(AbstractLink* link,
                                                    int packetsReceived,
                                                    int packetsDrops)
{
    int linkId = d->descriptedLinks.key(link, 0);
    if (!linkId) return;

    emit mavLinkStatisticsChanged(linkId, packetsReceived, packetsDrops);
}

void CommunicatorWorker::onMavLinkProtocolChanged(AbstractLink* link,
                                                  AbstractCommunicator::Protocol protocol)
{
    int linkId = d->descriptedLinks.key(link, 0);
    if (!linkId) return;

    emit mavLinkProtocolChanged(linkId, ::toDtoProtocol(protocol));
}

void CommunicatorWorker::addCommunicatorImpl(AbstractCommunicator* communicator)
{
    if (!communicator || d->communicators.contains(communicator)) return;

    communicator->setParent(this);

    connect(communicator, &AbstractCommunicator::mavLinkStatisticsChanged,
            this, &CommunicatorWorker::onMavLinkStatisticsChanged);
    connect(communicator, &AbstractCommunicator::mavLinkProtocolChanged,
            this, &CommunicatorWorker::onMavLinkProtocolChanged);

    connect(communicator, &AbstractCommunicator::mavLinkStatisticsChanged,
            this, &CommunicatorWorker::onMavLinkStatisticsChanged);
    connect(communicator, &AbstractCommunicator::mavLinkProtocolChanged,
            this, &CommunicatorWorker::onMavLinkProtocolChanged);

    for (AbstractLink* link: d->descriptedLinks.values())
    {
        communicator->addLink(link);
    }

    d->communicators.append(communicator);
}

void CommunicatorWorker::deleteCommunicatorImpl(AbstractCommunicator* communicator)
{
    if (!d->communicators.removeOne(communicator)) return;

    disconnect(communicator, 0, this, 0);

    for (AbstractLink* link: d->descriptedLinks.values())
    {
        communicator->removeLink(link);
    }

    delete communicator;
}

void CommunicatorWorker::updateLinkImpl(int linkId, const LinkFactoryPtr& factory, bool autoconnect)
{
    AbstractLink* link = nullptr;

    if (d->descriptedLinks.contains(linkId))
    {
        link = d->descriptedLinks[linkId];
        factory->update(link);
    }
    else
    {
        link = factory->create();
        if (!link) return;

        link->setParent(this);
        connect(link, &AbstractLink::connectedChanged, this, [this, linkId](bool connected) {
            emit linkStatusChanged(linkId, connected); });
        connect(link, &AbstractLink::dataReceived, this, [this, linkId]() { emit linkRecv(linkId); });
        connect(link, &AbstractLink::dataSent, this, [this, linkId]() { emit linkSent(linkId); });
        connect(link, &AbstractLink::errored, this,
                [this, linkId](const QString& error) { emit linkErrored(linkId, error); });

        d->descriptedLinks[linkId] = link;
        for (AbstractCommunicator* communicator: d->communicators)
        {
            communicator->addLink(link);
        }

        if (autoconnect)
        {
            link->connectLink();
            emit linkStatusChanged(linkId, link->isConnected());
        }
    }
}

void CommunicatorWorker::removeLinkImpl(int linkId)
{
    if (d->descriptedLinks.contains(linkId))
    {
        AbstractLink* link = d->descriptedLinks.take(linkId);

        for (AbstractCommunicator* communicator: d->communicators)
        {
            communicator->removeLink(link);
        }
        delete link;
    }
}

void CommunicatorWorker::setLinkConnectedImpl(int linkId, bool connected)
{
    AbstractLink* link = d->descriptedLinks[linkId];
    if (link) link->setConnected(connected);
}

void CommunicatorWorker::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != d->statisticsTimer) return QObject::timerEvent(event);

    for (int id: d->descriptedLinks.keys())
    {
        AbstractLink* link = d->descriptedLinks[id];

        emit linkStatisticsChanged(id, QTime::currentTime().msecsSinceStartOfDay(),
                                   link->takeBytesReceived(), link->takeBytesSent());
    }
}
