#ifndef MAP_HANDLE_H
#define MAP_HANDLE_H

#include <QObject>

namespace presentation
{
    class AbstractMapPresenter;

    class MapHandle: public QObject
    {
        Q_OBJECT

    public:
        MapHandle(AbstractMapPresenter* map);

    signals:
        void reloadMap();

    private:
        AbstractMapPresenter* const m_map;
    };
}

#endif // MAP_HANDLE_H
