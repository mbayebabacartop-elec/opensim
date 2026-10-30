import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1240
    height: 780
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("OpenLab Simulator")
    color: "#101820"

    menuBar: MenuBar {
        Menu {
            title: qsTr("Fichier")
            Action {
                text: qsTr("Quitter")
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: qsTr("Aide")
            Action {
                text: qsTr("À propos")
                onTriggered: aboutDialog.open()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 250
                Layout.fillHeight: true
                color: "#17232e"
                border.color: "#283846"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 10

                    Label {
                        text: qsTr("Bibliothèque")
                        color: "#edf4fa"
                        font.pixelSize: 17
                        font.bold: true
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#344552"
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Les composants seront ajoutés avec le Core.\n\nAucun composant n’est encore disponible dans cette version.")
                        color: "#b3c2ce"
                        wrapMode: Text.WordWrap
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#101820"

                ColumnLayout {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 96, 680)
                    spacing: 14

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Bienvenue dans OpenLab Simulator")
                        color: "#f2f6fa"
                        font.pixelSize: 30
                        font.bold: true
                        wrapMode: Text.WordWrap
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Le shell natif Qt Quick est opérationnel. Les éditeurs, moteurs et instruments seront ajoutés après validation de leurs contrats scientifiques.")
                        color: "#bdcbd6"
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Aucune simulation n’est active.")
                        color: "#83c8b5"
                        font.pixelSize: 13
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 32
            color: "#17232e"
            border.color: "#283846"

            Label {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 12
                text: qsTr("OpenLab Simulator 0.1.0 — prêt")
                color: "#b3c2ce"
                font.pixelSize: 12
            }
        }
    }

    Dialog {
        id: aboutDialog
        width: 460
        modal: true
        title: qsTr("À propos d’OpenLab Simulator")
        standardButtons: Dialog.Ok
        anchors.centerIn: Overlay.overlay

        contentItem: Item {
            implicitWidth: 420
            implicitHeight: 100

            Label {
                anchors.fill: parent
                text: qsTr("OpenLab Simulator 0.1.0\nLaboratoire numérique modulaire en construction.\nAucun moteur de simulation n’est encore inclus.")
                color: "#dce6ed"
                wrapMode: Text.WordWrap
                padding: 12
            }
        }
    }
}

