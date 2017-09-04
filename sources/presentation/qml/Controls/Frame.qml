import QtQuick 2.6
import QtQuick.Controls 2.0

import "./"

Frame {
    id: control

    property color backgroundColor: palette.raisedColor

    background: Rectangle {
        color: backgroundColor
        border.color: palette.backgroundColor
        radius: 3

        // TODO: hatch
    }
}
