import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "qrc:/Controls"
import "../Map"

Pane {
    id: root

    property alias missions: missionsBox.model
    property alias vehicles: vehiclesBox.model
    property string selectedMission
    property string assignedVehicle
    property int progress: progressBar.value

    signal selectMission(string name)
    signal addMission()
    signal removeMission()
    signal renameMission(string name)
    signal assignVehicle(string name)
    signal uploadMission()
    signal downloadMission()

    onSelectedMissionChanged: {
        edit.checked = false;
        if (missionsBox.currentText != selectedMission)
        {
            missionsBox.currentIndex = missionsBox.model.indexOf(selectedMission);
        }
    }
    onAssignedVehicleChanged: {
        if (vehiclesBox.currentText != assignedVehicle)
        {
            vehiclesBox.currentIndex = vehiclesBox.model.indexOf(assignedVehicle);
        }
    }

    ColumnLayout {
        id: column
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        Layout.margins: palette.margins

        GridLayout {
            columns: 5

            Label {
                text: qsTr("Mission")
            }

            ComboBox {
                id: missionsBox
                visible: !edit.checked
                onCurrentTextChanged: {
                    selectedMission = currentText;
                    selectMission(currentText);
                }
            }

            TextField {
                id: nameEdit
                visible: edit.checked
            }

            Button {
                id: edit
                iconSource: "qrc:/icons/edit.svg"
                checkable: true
                enabled: selectedMission.length > 0
                onCheckedChanged: {
                    if (checked)
                    {
                        nameEdit.text = selectedMission;
                        nameEdit.forceActiveFocus();
                        nameEdit.selectAll();
                    }
                    else
                    {
                        renameMission(nameEdit.text);
                    }
                }
            }

            Button {
                iconSource: "qrc:/icons/add.svg"
                onClicked: addMission()
            }

            Button {
                iconSource: "qrc:/icons/remove.svg"
                enabled: selectedMission.length > 0
                onClicked: removeMission()
            }

            Label {
                text: qsTr("Vehicle")
            }

            ComboBox {
                id: vehiclesBox
                enabled: selectedMission.length > 0
                Layout.columnSpan: 2
                onCurrentTextChanged: {
                    assignedVehicle = currentText;
                    assignVehicle(currentText);
                }
            }

            Button {
                iconSource: "qrc:/icons/download.svg"
                enabled: selectedMission.length > 0 && assignedVehicle.length > 0
                onClicked: downloadMission()
            }

            Button {
                iconSource: "qrc:/icons/upload.svg"
                enabled: selectedMission.length > 0 && assignedVehicle.length > 0
                onClicked: uploadMission()
            }

            ProgressBar {
                id: progressBar
                to: 100
                Layout.columnSpan: 5
                Layout.fillWidth: true
            }
        }

        MissionItemView {
            objectName: "missionItem"
        }
    }

    MapView {
        id: map
        objectName: "map"
        anchors.left: column.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: palette.margins
    }
}
