import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: window
    property int workspace: 1
    property int labMode: 1
    property real motorPlaybackTime: 0
    property real motorRotorAngle: 0
    property real motorCameraYaw: -0.55
    property real motorCameraElevation: 0.62
    property int motorViewMode: 1
    property real motorZoom: 1.0
    property string motorInspectedPart: "rotor"
    property int motorDmmMode: 0
    property bool motorScopePaused: false
    property real motorPlaybackSpeed: 1.0
    component RealSpinBox: SpinBox {
        id: decimalControl
        property real realFrom: 0
        property real realTo: 100
        property real realStep: 1
        property int decimalPlaces: 2
        property real realValue: 0
        signal realValueEdited(real value)
        readonly property int scaleFactor: Math.pow(10, decimalPlaces)
        from: Math.round(realFrom * scaleFactor)
        to: Math.round(realTo * scaleFactor)
        stepSize: Math.max(1, Math.round(realStep * scaleFactor))
        value: Math.round(realValue * scaleFactor)
        textFromValue: function(value, locale) { return Number(value / scaleFactor).toLocaleString(locale, 'f', decimalPlaces) }
        valueFromText: function(text, locale) { return Math.round(Number.fromLocaleString(locale, text) * scaleFactor) }
        validator: DoubleValidator { bottom: decimalControl.realFrom; top: decimalControl.realTo; decimals: decimalControl.decimalPlaces; notation: DoubleValidator.StandardNotation }
        palette.text: "#eff6fb"
        palette.button: "#1a2733"
        palette.buttonText: "#eff6fb"
        background: Rectangle {
            implicitWidth: 180
            implicitHeight: 36
            radius: 6
            color: "#17232e"
            border.color: decimalControl.activeFocus ? "#61d4bb" : "#344554"
        }
        onValueModified: realValueEdited(value / scaleFactor)
    }
    width: 1280
    height: 800
    minimumWidth: 1000
    minimumHeight: 640
    visible: true
    title: qsTr("OpenLab Simulator — Laboratoire électrique")
    color: "#0b1118"
    palette.window: "#0b1118"
    palette.windowText: "#eff6fb"
    palette.base: "#111a24"
    palette.alternateBase: "#17232e"
    palette.text: "#eff6fb"
    palette.button: "#1a2733"
    palette.buttonText: "#eff6fb"
    palette.highlight: "#1d716e"
    palette.highlightedText: "#ffffff"
    palette.mid: "#253444"

    menuBar: MenuBar {
        background: Rectangle { color: "#111a24"; border.color: "#253444" }
        delegate: MenuBarItem {
            id: barItem
            contentItem: Label { text: barItem.text; color: "#dce6ee"; leftPadding: 10; rightPadding: 10; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: barItem.highlighted ? "#253444" : "#111a24" }
        }
        Menu {
            title: qsTr("Fichier")
            Action { text: qsTr("Ouvrir un banc RL…"); onTriggered: openDialog.open() }
            Action { text: qsTr("Enregistrer le banc RL…"); onTriggered: saveDialog.open() }
            Action { text: qsTr("Exporter les mesures CSV…"); onTriggered: csvDialog.open() }
            MenuSeparator {}
            Action { text: qsTr("Quitter"); onTriggered: Qt.quit() }
        }
        Menu {
            title: qsTr("Simulation")
            Action { text: lab.running ? qsTr("Annuler le calcul") : qsTr("Calculer le transitoire"); onTriggered: lab.runOrCancel() }
        }
        Menu {
            title: qsTr("Aide")
            Action { text: qsTr("À propos"); onTriggered: aboutDialog.open() }
        }
    }

    header: Rectangle {
        height: 76
        color: "#111a24"
        border.color: "#253444"
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            spacing: 16
            Rectangle {
                Layout.preferredWidth: 38
                Layout.preferredHeight: 38
                radius: 10
                color: "#1d716e"
                Label { anchors.centerIn: parent; text: "O"; color: "white"; font.pixelSize: 23; font.bold: true }
            }
            ColumnLayout {
                spacing: 2
                Label { text: qsTr("OpenLab Simulator"); color: "#eff6fb"; font.pixelSize: 19; font.bold: true }
                Label { text: window.workspace === 0 ? qsTr("ATELIER  /  CONCEPTION") : window.workspace === 2 ? qsTr("APPRENTISSAGE  /  TRAVAUX PRATIQUES") : qsTr("LABORATOIRE  /  ÉLECTRICITÉ  /  ") + (window.labMode === 0 ? qsTr("TRANSITOIRE RL") : window.labMode === 1 ? qsTr("MOTEUR CC") : qsTr("RÉSEAU RC · MNA")); color: "#8293a4"; font.pixelSize: 10; font.letterSpacing: 1.1 }
            }
            RowLayout {
                spacing: 4
                Repeater {
                    model: [qsTr("Atelier"), qsTr("Laboratoire"), qsTr("Cours")]
                    delegate: Button {
                        required property int index
                        required property string modelData
                        text: modelData
                        flat: true
                        highlighted: window.workspace === index
                        onClicked: window.workspace = index
                        contentItem: Label { text: parent.text; color: parent.highlighted ? "#71d9c0" : "#a8b7c4"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        background: Rectangle { radius: 6; color: parent.highlighted ? "#1a302f" : "transparent" }
                    }
                }
            }
            RowLayout {
                spacing: 2
                visible: window.workspace === 1
                Repeater {
                    model: [qsTr("RL"), qsTr("Moteur CC"), qsTr("Réseau RC")]
                    delegate: Button {
                        required property int index
                        required property string modelData
                        text: modelData
                        flat: true
                        highlighted: window.labMode === index
                        onClicked: window.labMode = index
                        contentItem: Label { text: parent.text; color: parent.highlighted ? "#71d9c0" : "#a8b7c4"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 11 }
                        background: Rectangle { radius: 6; color: parent.highlighted ? "#1a302f" : "transparent" }
                    }
                }
            }
            Button {
                id: simulationButton
                visible: window.workspace === 1
                text: lab.running ? qsTr("Annuler") : qsTr("▶  Simuler")
                highlighted: !lab.running
                onClicked: window.labMode === 0 ? lab.runOrCancel() : window.labMode === 1 ? lab.runMotorSimulation() : lab.runMnaSimulation()
                contentItem: Label { text: simulationButton.text; color: "#eff6fb"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true; leftPadding: 14; rightPadding: 14 }
                background: Rectangle { radius: 7; color: simulationButton.down ? "#145552" : "#1d716e" }
            }
            Item { Layout.fillWidth: true }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 16

        Rectangle {
            visible: window.workspace === 1 && window.labMode === 0
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            radius: 12
            color: "#111a24"
            border.color: "#253444"

            ScrollView {
                anchors.fill: parent
                anchors.margins: 18
                clip: true
                ColumnLayout {
                    width: 264
                    spacing: 14
                    Label { text: qsTr("Banc d’essai RL"); color: "#eff6fb"; font.pixelSize: 18; font.bold: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Circuit série idéal soumis à un échelon de tension. Les valeurs sont modifiables et le résultat est calculé par Euler implicite.")
                        color: "#91a2b2"; wrapMode: Text.WordWrap; font.pixelSize: 12
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: "#253444" }
                    Label { text: qsTr("PARAMÈTRES DU CIRCUIT"); color: "#64c5b5"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 1 }

                    Label { text: qsTr("Résistance totale"); color: "#dce6ee"; font.pixelSize: 12 }
                    RowLayout {
                        Layout.fillWidth: true
                        RealSpinBox { Layout.fillWidth: true; realFrom: 0.001; realTo: 1000000; realValue: lab.resistanceOhms; decimalPlaces: 3; realStep: 1; enabled: !lab.running; onRealValueEdited: function(value) { lab.resistanceOhms = value } }
                        Label { text: "Ω"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                    }
                    Label { text: qsTr("Inductance"); color: "#dce6ee"; font.pixelSize: 12 }
                    RowLayout {
                        Layout.fillWidth: true
                        RealSpinBox { Layout.fillWidth: true; realFrom: 0.000001; realTo: 1000; realValue: lab.inductanceHenries; decimalPlaces: 6; realStep: 0.001; enabled: !lab.running; onRealValueEdited: function(value) { lab.inductanceHenries = value } }
                        Label { text: "H"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                    }
                    Label { text: qsTr("Tension de l’échelon"); color: "#dce6ee"; font.pixelSize: 12 }
                    RowLayout {
                        Layout.fillWidth: true
                        RealSpinBox { Layout.fillWidth: true; realFrom: -10000; realTo: 10000; realValue: lab.sourceVolts; decimalPlaces: 2; realStep: 0.5; enabled: !lab.running; onRealValueEdited: function(value) { lab.sourceVolts = value } }
                        Label { text: "V"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                    }
                    Label { text: qsTr("Courant initial i(0)"); color: "#dce6ee"; font.pixelSize: 12 }
                    RowLayout {
                        Layout.fillWidth: true
                        RealSpinBox { Layout.fillWidth: true; realFrom: -10000; realTo: 10000; realValue: lab.initialCurrentAmps; decimalPlaces: 3; realStep: 0.1; enabled: !lab.running; onRealValueEdited: function(value) { lab.initialCurrentAmps = value } }
                        Label { text: "A"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: "#253444" }
                    Label { text: qsTr("CONFIGURATION NUMÉRIQUE"); color: "#64c5b5"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 1 }
                    Label { text: qsTr("Durée de simulation"); color: "#dce6ee"; font.pixelSize: 12 }
                    RowLayout {
                        Layout.fillWidth: true
                        RealSpinBox { Layout.fillWidth: true; realFrom: 0.000001; realTo: 1000; realValue: lab.durationSeconds; decimalPlaces: 6; realStep: 0.01; enabled: !lab.running; onRealValueEdited: function(value) { lab.durationSeconds = value } }
                        Label { text: "s"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                    }
                    Label { text: qsTr("Pas maximal"); color: "#dce6ee"; font.pixelSize: 12 }
                    RowLayout {
                        Layout.fillWidth: true
                        RealSpinBox { Layout.fillWidth: true; realFrom: 0.000001; realTo: 1; realValue: lab.stepSeconds; decimalPlaces: 6; realStep: 0.0001; enabled: !lab.running; onRealValueEdited: function(value) { lab.stepSeconds = value } }
                        Label { text: "s"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Euler implicite • pas ajusté au temps final • maximum 1 000 000 de pas")
                        color: "#738596"; wrapMode: Text.WordWrap; font.pixelSize: 10
                    }
                    Item { Layout.fillHeight: true; Layout.minimumHeight: 6 }
                }
            }
        }

        ColumnLayout {
            visible: window.workspace === 1 && window.labMode === 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 0
            Layout.preferredWidth: parent.width - 316
            Layout.maximumWidth: parent.width - 316
            Layout.alignment: Qt.AlignTop
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    spacing: 3
                    Label { text: qsTr("Réponse temporelle"); color: "#eff6fb"; font.pixelSize: 21; font.bold: true }
                    Label { text: qsTr("Courant dans l’inductance en fonction du temps"); color: "#91a2b2"; font.pixelSize: 12 }
                }
                Item { Layout.fillWidth: true }
                Rectangle {
                    radius: 12; color: lab.running ? "#3b3020" : (lab.sampleCount ? "#183b36" : "#202b36")
                    implicitWidth: stateLabel.implicitWidth + 24; implicitHeight: 30
                    Label { id: stateLabel; anchors.centerIn: parent; text: lab.running ? qsTr("CALCUL…") : (lab.sampleCount ? qsTr("RÉSULTAT CALCULÉ") : qsTr("EN ATTENTE")); color: lab.running ? "#ffc978" : (lab.sampleCount ? "#77d6bd" : "#a5b2bd"); font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.6 }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.minimumWidth: 0
                Layout.preferredWidth: parent.width
                Layout.maximumWidth: parent.width
                Layout.minimumHeight: 250
                Layout.preferredHeight: 320
                Layout.maximumHeight: 360
                radius: 12
                color: "#111a24"
                border.color: "#253444"
                Canvas {
                    id: plot
                    anchors.fill: parent
                    anchors.margins: 16
                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        const left = 78, right = 18, top = 18, bottom = 48
                        const w = width - left - right, h = height - top - bottom
                        const data = lab.currentSamples
                        let minI = 0, maxI = 1
                        if (data.length > 0) {
                            minI = maxI = data[0].current
                            for (let j = 1; j < data.length; ++j) { minI = Math.min(minI, data[j].current); maxI = Math.max(maxI, data[j].current) }
                            if (Math.abs(maxI - minI) < 1e-12) { minI -= 0.5; maxI += 0.5 }
                            const pad = (maxI - minI) * 0.08; minI -= pad; maxI += pad
                        }
                        ctx.font = "11px sans-serif"
                        ctx.lineWidth = 1
                        ctx.strokeStyle = "#273644"
                        ctx.fillStyle = "#8c9cab"
                        for (let g = 0; g <= 4; ++g) {
                            const y = top + h * g / 4
                            ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(left + w, y); ctx.stroke()
                            ctx.textAlign = "right"; ctx.textBaseline = "middle"
                            ctx.fillText((maxI - (maxI - minI) * g / 4).toPrecision(3) + " A", left - 10, y)
                        }
                        for (let g = 0; g <= 4; ++g) {
                            const x = left + w * g / 4
                            ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, top + h); ctx.stroke()
                            ctx.textAlign = g === 0 ? "left" : (g === 4 ? "right" : "center"); ctx.textBaseline = "top"
                            ctx.fillText((lab.durationSeconds * g / 4).toPrecision(3) + " s", x, top + h + 10)
                        }
                        ctx.save()
                        ctx.translate(15, top + h / 2); ctx.rotate(-Math.PI / 2)
                        ctx.textAlign = "center"; ctx.fillStyle = "#b8c5d0"; ctx.fillText(qsTr("Courant (A)"), 0, 0)
                        ctx.restore()
                        ctx.textAlign = "center"; ctx.fillText(qsTr("Temps (s)"), left + w / 2, height - 8)
                        if (lab.currentSamples.length < 2) {
                            ctx.textAlign = "center"; ctx.textBaseline = "middle"; ctx.fillStyle = "#718294"
                            ctx.font = "14px sans-serif"
                            ctx.fillText(qsTr("Lancez la simulation pour afficher les échantillons calculés"), left + w / 2, top + h / 2)
                            return
                        }
                        ctx.beginPath()
                        for (let j = 0; j < data.length; ++j) {
                            const x = left + w * data[j].time / lab.durationSeconds
                            const y = top + h * (maxI - data[j].current) / (maxI - minI)
                            if (j === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                        }
                        ctx.strokeStyle = "#61d4bb"; ctx.lineWidth = 2.5; ctx.stroke()
                        const last = data[data.length - 1]
                        const lx = left + w * last.time / lab.durationSeconds
                        const ly = top + h * (maxI - last.current) / (maxI - minI)
                        ctx.beginPath(); ctx.arc(lx, ly, 4, 0, Math.PI * 2); ctx.fillStyle = "#e9fffa"; ctx.fill()
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredWidth: parent.width
                Layout.maximumWidth: parent.width
                spacing: 12
                Rectangle {
                    Layout.fillWidth: true; Layout.minimumWidth: 0; Layout.preferredWidth: (parent.width - 24) / 3; Layout.preferredHeight: 82; radius: 10; color: "#111a24"; border.color: "#253444"
                    ColumnLayout { anchors.fill: parent; anchors.margins: 14; spacing: 5
                        Label { text: qsTr("CONSTANTE DE TEMPS"); color: "#8092a2"; font.pixelSize: 9; font.bold: true; font.letterSpacing: 0.8 }
                        Label { text: lab.sampleCount ? lab.timeConstantSeconds.toPrecision(5) + " s" : "—"; color: "#eff6fb"; font.pixelSize: 20; font.bold: true }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true; Layout.minimumWidth: 0; Layout.preferredWidth: (parent.width - 24) / 3; Layout.preferredHeight: 82; radius: 10; color: "#111a24"; border.color: "#253444"
                    ColumnLayout { anchors.fill: parent; anchors.margins: 14; spacing: 5
                        Label { text: qsTr("COURANT FINAL"); color: "#8092a2"; font.pixelSize: 9; font.bold: true; font.letterSpacing: 0.8 }
                        Label { text: lab.sampleCount ? lab.finalCurrentAmps.toPrecision(6) + " A" : "—"; color: "#eff6fb"; font.pixelSize: 20; font.bold: true }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true; Layout.minimumWidth: 0; Layout.preferredWidth: (parent.width - 24) / 3; Layout.preferredHeight: 82; radius: 10; color: "#111a24"; border.color: "#253444"
                    ColumnLayout { anchors.fill: parent; anchors.margins: 14; spacing: 5
                        Label { text: qsTr("ÉCHANTILLONS"); color: "#8092a2"; font.pixelSize: 9; font.bold: true; font.letterSpacing: 0.8 }
                        Label { text: lab.sampleCount ? lab.sampleCount.toLocaleString(Qt.locale()) : "—"; color: "#eff6fb"; font.pixelSize: 20; font.bold: true }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true; Layout.minimumWidth: 0; Layout.preferredHeight: 42; radius: 8; color: "#111a24"; border.color: "#253444"
                RowLayout { anchors.fill: parent; anchors.leftMargin: 14; anchors.rightMargin: 14; spacing: 10
                    Rectangle { width: 7; height: 7; radius: 4; color: lab.running ? "#ffc978" : (lab.sampleCount ? "#61d4bb" : "#8293a4") }
                    Label { Layout.fillWidth: true; text: lab.statusText; color: "#b1bfca"; font.pixelSize: 11; elide: Text.ElideRight }
                    Label { text: qsTr("MODÈLE E-001 · SI"); color: "#708293"; font.pixelSize: 9; font.bold: true }
                }
            }
        }
    }

    Rectangle {
        id: motorWorkspace
        anchors.fill: parent
        anchors.margins: 18
        visible: window.workspace === 1 && window.labMode === 1
        color: "#0b1118"
        RowLayout {
            anchors.fill: parent
            spacing: 16
            Rectangle {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                radius: 12; color: "#111a24"; border.color: "#253444"
                ScrollView {
                    anchors.fill: parent; anchors.margins: 16; clip: true
                    ColumnLayout {
                        width: 288; spacing: 12
                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                spacing: 2
                                Label { text: qsTr("Banc d'essai Moteur CC"); color: "#eff6fb"; font.pixelSize: 18; font.bold: true }
                                Label { text: qsTr("Machine à courant continu & charge"); color: "#8ea0b0"; font.pixelSize: 11 }
                            }
                            Item { Layout.fillWidth: true }
                            Rectangle {
                                width: 78; height: 24; radius: 12
                                color: lab.motorSampleCount ? "#143c33" : "#24323e"
                                border.color: lab.motorSampleCount ? "#2bd3a2" : "#45596b"
                                Label {
                                    anchors.centerIn: parent
                                    text: lab.motorSampleCount ? qsTr("SIMULÉ") : qsTr("ATTENTE")
                                    color: lab.motorSampleCount ? "#5ef0d0" : "#a2b4c2"; font.pixelSize: 9; font.bold: true
                                }
                            }
                        }

                        // 1. INSTRUMENT : ALIMENTATION DC RÉGLABLE
                        Rectangle {
                            Layout.fillWidth: true; radius: 10; color: "#0c151e"; border.color: "#253a4c"
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 9
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: qsTr("ALIMENTATION CONTINUE DC"); color: "#5eead4"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.8 }
                                    Item { Layout.fillWidth: true }
                                    Rectangle {
                                        width: 10; height: 10; radius: 5
                                        color: lab.motorPowered ? "#22c55e" : "#ef4444"
                                    }
                                }
                                // Afficheurs LED d'alimentation
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 8
                                    Rectangle {
                                        Layout.fillWidth: true; height: 42; radius: 6; color: "#071118"; border.color: "#1c3242"
                                        ColumnLayout {
                                            anchors.centerIn: parent; spacing: 1
                                            Label { text: qsTr("TENSION"); color: "#72889a"; font.pixelSize: 8; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                                            Label {
                                                text: (lab.motorPowered ? lab.motorVoltage.toFixed(1) : "0.0") + " V"
                                                color: lab.motorPowered ? "#5ef0d0" : "#5a6a78"; font.pixelSize: 15; font.bold: true
                                                Layout.alignment: Qt.AlignHCenter
                                            }
                                        }
                                    }
                                    Rectangle {
                                        Layout.fillWidth: true; height: 42; radius: 6; color: "#071118"; border.color: "#1c3242"
                                        ColumnLayout {
                                            anchors.centerIn: parent; spacing: 1
                                            Label { text: qsTr("DÉBIT I"); color: "#72889a"; font.pixelSize: 8; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                                            Label {
                                                text: (lab.motorSampleCount ? lab.motorCurrentAmps.toFixed(2) : "0.00") + " A"
                                                color: lab.motorPowered ? "#fcd34d" : "#5a6a78"; font.pixelSize: 15; font.bold: true
                                                Layout.alignment: Qt.AlignHCenter
                                            }
                                        }
                                    }
                                }
                                // Interrupteur physique On/Off
                                Button {
                                    Layout.fillWidth: true; height: 32
                                    highlighted: lab.motorPowered
                                    contentItem: RowLayout {
                                        spacing: 8
                                        Item { Layout.fillWidth: true }
                                        Rectangle {
                                            width: 12; height: 12; radius: 6
                                            color: lab.motorPowered ? "#22c55e" : "#ef4444"
                                            border.color: "#ffffff"; border.width: 1
                                        }
                                        Label {
                                            text: lab.motorPowered ? qsTr("INTERRUPTEUR : MARCHE (ON)") : qsTr("INTERRUPTEUR : ARRÊT (OFF)")
                                            color: lab.motorPowered ? "#ffffff" : "#fca5a5"; font.bold: true; font.pixelSize: 11
                                        }
                                        Item { Layout.fillWidth: true }
                                    }
                                    background: Rectangle {
                                        radius: 6
                                        color: lab.motorPowered ? "#166534" : "#7f1d1d"
                                        border.color: lab.motorPowered ? "#22c55e" : "#dc2626"
                                    }
                                    onClicked: {
                                        lab.setMotorPowered(!lab.motorPowered)
                                        lab.runMotorSimulation()
                                    }
                                }
                                // Réglage de tension
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: qsTr("Consigne :"); color: "#b2c3d2"; font.pixelSize: 11 }
                                    Item { Layout.fillWidth: true }
                                    RealSpinBox {
                                        Layout.preferredWidth: 140
                                        realFrom: 0; realTo: 48; realValue: lab.motorVoltage; decimalPlaces: 1; realStep: 0.5
                                        enabled: !lab.running
                                        onRealValueEdited: function(v) {
                                            lab.motorVoltage = v
                                            if (lab.motorPowered) lab.runMotorSimulation()
                                        }
                                    }
                                }
                                // Boutons de présélection de tension
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 4
                                    Repeater {
                                        model: [5, 12, 24, 48]
                                        delegate: Button {
                                            required property int modelData
                                            Layout.fillWidth: true; Layout.preferredHeight: 24
                                            text: modelData + "V"
                                            flat: true
                                            contentItem: Label { text: parent.text; color: "#b9cad7"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            background: Rectangle { radius: 4; color: "#162330"; border.color: "#2a3d4f" }
                                            onClicked: {
                                                lab.motorVoltage = modelData
                                                if (lab.motorPowered) lab.runMotorSimulation()
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // 2. INSTRUMENT : CHARGE MÉCANIQUE (FREIN DYNAMOMÉTRIQUE)
                        Rectangle {
                            Layout.fillWidth: true; radius: 10; color: "#0c151e"; border.color: "#253a4c"
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 8
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: qsTr("FREIN DYNAMOMÉTRIQUE"); color: "#f59e0b"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.8 }
                                    Item { Layout.fillWidth: true }
                                    Label { text: "Cr = " + lab.motorLoadTorque.toFixed(3) + " N·m"; color: "#fcd34d"; font.pixelSize: 10; font.bold: true }
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    Slider {
                                        id: torqueSlider
                                        Layout.fillWidth: true
                                        from: 0; to: 0.12; stepSize: 0.005; value: lab.motorLoadTorque
                                        onMoved: {
                                            lab.motorLoadTorque = Math.round(value * 1000) / 1000
                                            if (lab.motorPowered) lab.runMotorSimulation()
                                        }
                                    }
                                    Label { text: (torqueSlider.value * 1000).toFixed(0) + " mN·m"; color: "#cbd5e1"; font.pixelSize: 10; Layout.preferredWidth: 54 }
                                }
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 4
                                    Button {
                                        Layout.fillWidth: true; Layout.preferredHeight: 24; text: qsTr("À vide (0)")
                                        contentItem: Label { text: parent.text; color: "#a5b6c4"; font.pixelSize: 9; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        background: Rectangle { radius: 4; color: "#162330"; border.color: "#2a3d4f" }
                                        onClicked: { lab.motorLoadTorque = 0.0; if (lab.motorPowered) lab.runMotorSimulation() }
                                    }
                                    Button {
                                        Layout.fillWidth: true; Layout.preferredHeight: 24; text: qsTr("Nominal (0,03)")
                                        contentItem: Label { text: parent.text; color: "#a5b6c4"; font.pixelSize: 9; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        background: Rectangle { radius: 4; color: "#162330"; border.color: "#2a3d4f" }
                                        onClicked: { lab.motorLoadTorque = 0.03; if (lab.motorPowered) lab.runMotorSimulation() }
                                    }
                                    Button {
                                        Layout.fillWidth: true; Layout.preferredHeight: 24; text: qsTr("Forte (0,06)")
                                        contentItem: Label { text: parent.text; color: "#a5b6c4"; font.pixelSize: 9; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        background: Rectangle { radius: 4; color: "#162330"; border.color: "#2a3d4f" }
                                        onClicked: { lab.motorLoadTorque = 0.06; if (lab.motorPowered) lab.runMotorSimulation() }
                                    }
                                }
                            }
                        }

                        // 3. INSTRUMENT : MULTIMÈTRE NUMÉRIQUE (DMM)
                        Rectangle {
                            Layout.fillWidth: true; radius: 10; color: "#0c151e"; border.color: "#253a4c"
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 8
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: qsTr("MULTIMÈTRE DE BANC (DMM)"); color: "#38bdf8"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.8 }
                                    Item { Layout.fillWidth: true }
                                    Label { text: "AUTO-RANGE"; color: "#64748b"; font.pixelSize: 8; font.bold: true }
                                }
                                // Écran LCD multimètre
                                Rectangle {
                                    Layout.fillWidth: true; height: 50; radius: 6; color: "#051e22"; border.color: "#0f4a42"
                                    RowLayout {
                                        anchors.fill: parent; anchors.margins: 8
                                        Label {
                                            text: window.motorDmmMode === 0 ? "V DC" : window.motorDmmMode === 1 ? "A DC" : window.motorDmmMode === 2 ? "WATT" : "EFF %"
                                            color: "#2dd4bf"; font.pixelSize: 10; font.bold: true
                                        }
                                        Item { Layout.fillWidth: true }
                                        Label {
                                            text: {
                                                if (!lab.motorSampleCount) return "0.000"
                                                if (window.motorDmmMode === 0) return (lab.motorPowered ? lab.motorVoltage.toFixed(2) : "0.00") + " V"
                                                if (window.motorDmmMode === 1) return lab.motorCurrentAmps.toFixed(3) + " A"
                                                if (window.motorDmmMode === 2) return lab.motorElectricalPowerWatts.toFixed(2) + " W"
                                                return lab.motorEfficiencyPercent.toFixed(1) + " %"
                                            }
                                            color: "#5ef0d0"; font.pixelSize: 20; font.bold: true; font.family: "Consolas, monospace"
                                        }
                                    }
                                }
                                // Sélecteur rotatif de fonction
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 4
                                    Repeater {
                                        model: [qsTr("U [V]"), qsTr("I [A]"), qsTr("P [W]"), qsTr("η [%]")]
                                        delegate: Button {
                                            required property int index
                                            required property string modelData
                                            Layout.fillWidth: true; Layout.preferredHeight: 26
                                            text: modelData
                                            highlighted: window.motorDmmMode === index
                                            contentItem: Label { text: parent.text; color: parent.highlighted ? "#38bdf8" : "#94a3b8"; font.pixelSize: 10; font.bold: parent.highlighted; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            background: Rectangle { radius: 4; color: parent.highlighted ? "#082f49" : "#162330"; border.color: parent.highlighted ? "#38bdf8" : "#2a3d4f" }
                                            onClicked: window.motorDmmMode = index
                                        }
                                    }
                                }
                            }
                        }

                        // 4. INSTRUMENT : TACHYMÈTRE NUMÉRIQUE
                        Rectangle {
                            Layout.fillWidth: true; radius: 10; color: "#0c151e"; border.color: "#253a4c"
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 8
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: qsTr("TACHYMÈTRE OPTIQUE"); color: "#a78bfa"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.8 }
                                    Item { Layout.fillWidth: true }
                                    Label {
                                        text: lab.motorAngularVelocity >= 0 ? "⟳ CW" : "⟲ CCW"
                                        color: lab.motorSampleCount && lab.motorAngularVelocity > 0 ? "#4ade80" : "#64748b"; font.pixelSize: 10; font.bold: true
                                    }
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label {
                                        text: (lab.motorSampleCount ? Math.round(lab.motorRpm) : 0) + " tr/min"
                                        color: "#eff6fb"; font.pixelSize: 22; font.bold: true; font.family: "Consolas, monospace"
                                    }
                                    Item { Layout.fillWidth: true }
                                    Label {
                                        text: (lab.motorSampleCount ? lab.motorAngularVelocity.toFixed(1) : "0.0") + " rad/s"
                                        color: "#a78bfa"; font.pixelSize: 12; font.bold: true
                                    }
                                }
                                // Bargraphe tachymétrique dynamique
                                Rectangle {
                                    Layout.fillWidth: true; height: 8; radius: 4; color: "#111827"
                                    Rectangle {
                                        width: Math.min(parent.width, Math.max(0, parent.width * (lab.motorRpm / 3000.0)))
                                        height: parent.height; radius: 4
                                        gradient: Gradient {
                                            orientation: Gradient.Horizontal
                                            GradientStop { position: 0.0; color: "#22c55e" }
                                            GradientStop { position: 0.7; color: "#eab308" }
                                            GradientStop { position: 1.0; color: "#ef4444" }
                                        }
                                    }
                                }
                            }
                        }

                        // Bouton principal pour recalculer / relancer
                        Button {
                            Layout.fillWidth: true; Layout.preferredHeight: 38
                            text: lab.running ? qsTr("Calcul en cours…") : qsTr("⚡ Relancer la simulation du banc")
                            highlighted: !lab.running
                            onClicked: lab.runMotorSimulation()
                            contentItem: Label { text: parent.text; color: "#eff6fb"; font.bold: true; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { radius: 7; color: parent.down ? "#145552" : "#1d716e" }
                        }
                    }
                }
            }

            // COLONNE CENTRALE & DROITE : SCÈNE 3D + OSCILLOSCOPE
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumWidth: 0; spacing: 10
                // Barre d'outils de la vue 3D
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Label { text: qsTr("VUE 3D DU BANC :"); color: "#94a3b8"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.7 }
                    Repeater {
                        model: [qsTr("Extérieure"), qsTr("Coupe (Rotor & Stator)"), qsTr("Éclatée")]
                        delegate: Button {
                            required property int index
                            required property string modelData
                            Layout.preferredHeight: 26
                            text: modelData
                            highlighted: window.motorViewMode === index
                            contentItem: Label { text: parent.text; color: parent.highlighted ? "#5ef0d0" : "#94a3b8"; font.pixelSize: 10; font.bold: parent.highlighted }
                            background: Rectangle { radius: 5; color: parent.highlighted ? "#134e48" : "#131d27"; border.color: parent.highlighted ? "#2dd4bf" : "#253444" }
                            onClicked: { window.motorViewMode = index; motorScene.requestPaint() }
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        Layout.preferredHeight: 26; text: qsTr("⟲ Réinit.")
                        contentItem: Label { text: parent.text; color: "#94a3b8"; font.pixelSize: 10 }
                        background: Rectangle { radius: 5; color: "#131d27"; border.color: "#253444" }
                        onClicked: { window.motorCameraYaw = -0.55; window.motorCameraElevation = 0.62; window.motorZoom = 1.0; motorScene.requestPaint() }
                    }
                    Button {
                        Layout.preferredHeight: 26; text: qsTr("＋")
                        contentItem: Label { text: parent.text; color: "#94a3b8"; font.pixelSize: 11; font.bold: true }
                        background: Rectangle { radius: 5; color: "#131d27"; border.color: "#253444" }
                        onClicked: { window.motorZoom = Math.min(2.0, window.motorZoom + 0.15); motorScene.requestPaint() }
                    }
                    Button {
                        Layout.preferredHeight: 26; text: qsTr("－")
                        contentItem: Label { text: parent.text; color: "#94a3b8"; font.pixelSize: 11; font.bold: true }
                        background: Rectangle { radius: 5; color: "#131d27"; border.color: "#253444" }
                        onClicked: { window.motorZoom = Math.max(0.6, window.motorZoom - 0.15); motorScene.requestPaint() }
                    }
                }

                // SCÈNE 3D INTERACTIVE (Canvas motorScene)
                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 260
                    radius: 12; color: "#0c131c"; border.color: "#253444"; clip: true
                    Canvas {
                        id: motorScene
                        anchors.fill: parent; anchors.margins: 4
                        property real rotorAngle: window.motorRotorAngle
                        property real yaw: window.motorCameraYaw
                        property real elevation: window.motorCameraElevation
                        property int viewMode: window.motorViewMode
                        property real zoom: window.motorZoom
                        property string inspected: window.motorInspectedPart
                        onRotorAngleChanged: requestPaint()
                        onYawChanged: requestPaint()
                        onElevationChanged: requestPaint()
                        onViewModeChanged: requestPaint()
                        onZoomChanged: requestPaint()
                        onInspectedChanged: requestPaint()

                        function renderMotor() {
                            const triangles = []
                            const addQuad = (a, b, c, d, color) => { triangles.push([a, b, c, color], [a, c, d, color]) }

                            const cylinderX = (x1, x2, r, n, color, phase = 0, angleSpan = 2 * Math.PI) => {
                                for (let j = 0; j < n; j++) {
                                    const a = phase + angleSpan * j / n
                                    const b = phase + angleSpan * (j + 1) / n
                                    const p1 = [x1, r * Math.cos(a), r * Math.sin(a)]
                                    const p2 = [x2, r * Math.cos(a), r * Math.sin(a)]
                                    const p3 = [x2, r * Math.cos(b), r * Math.sin(b)]
                                    const p4 = [x1, r * Math.cos(b), r * Math.sin(b)]
                                    addQuad(p1, p2, p3, p4, color)
                                }
                            }

                            const box = (x1, x2, y1, y2, z1, z2, color) => {
                                const p = [
                                    [x1, y1, z1], [x2, y1, z1], [x2, y2, z1], [x1, y2, z1],
                                    [x1, y1, z2], [x2, y1, z2], [x2, y2, z2], [x1, y2, z2]
                                ]
                                for (const f of [[0, 1, 2, 3], [4, 7, 6, 5], [0, 4, 5, 1], [1, 5, 6, 2], [2, 6, 7, 3], [3, 7, 4, 0]]) {
                                    addQuad(p[f[0]], p[f[1]], p[f[2]], p[f[3]], color)
                                }
                            }

                            const ringX = (x, r1, r2, n, color, phase = 0, angleSpan = 2 * Math.PI) => {
                                for (let j = 0; j < n; j++) {
                                    const a = phase + angleSpan * j / n
                                    const b = phase + angleSpan * (j + 1) / n
                                    addQuad(
                                        [x, r1 * Math.cos(a), r1 * Math.sin(a)],
                                        [x, r2 * Math.cos(a), r2 * Math.sin(a)],
                                        [x, r2 * Math.cos(b), r2 * Math.sin(b)],
                                        [x, r1 * Math.cos(b), r1 * Math.sin(b)],
                                        color
                                    )
                                }
                            }

                            const tube3D = (pts, r, n, color) => {
                                for (let i = 0; i < pts.length - 1; i++) {
                                    const p1 = pts[i], p2 = pts[i + 1]
                                    const dx = p2[0] - p1[0], dy = p2[1] - p1[1], dz = p2[2] - p1[2]
                                    const len = Math.hypot(dx, dy, dz) || 1
                                    let ux = -dy, uy = dx, uz = 0
                                    const ulen = Math.hypot(ux, uy, uz)
                                    if (ulen < 1e-4) { ux = 1; uy = 0; uz = 0 } else { ux /= ulen; uy /= ulen; uz /= ulen }
                                    const wx = (dy * uz - dz * uy) / len
                                    const wy = (dz * ux - dx * uz) / len
                                    const wz = (dx * uy - dy * ux) / len
                                    for (let j = 0; j < n; j++) {
                                        const a1 = 2 * Math.PI * j / n, a2 = 2 * Math.PI * (j + 1) / n
                                        const c1 = Math.cos(a1), s1 = Math.sin(a1)
                                        const c2 = Math.cos(a2), s2 = Math.sin(a2)
                                        const v1 = [p1[0] + r * (c1 * ux + s1 * wx), p1[1] + r * (c1 * uy + s1 * wy), p1[2] + r * (c1 * uz + s1 * wz)]
                                        const v2 = [p2[0] + r * (c1 * ux + s1 * wx), p2[1] + r * (c1 * uy + s1 * wy), p2[2] + r * (c1 * uz + s1 * wz)]
                                        const v3 = [p2[0] + r * (c2 * ux + s2 * wx), p2[1] + r * (c2 * uy + s2 * wy), p2[2] + r * (c2 * uz + s2 * wz)]
                                        const v4 = [p1[0] + r * (c2 * ux + s2 * wx), p1[1] + r * (c2 * uy + s2 * wy), p1[2] + r * (c2 * uz + s2 * wz)]
                                        addQuad(v1, v2, v3, v4, color)
                                    }
                                }
                            }

                            // Décalages en mode vue éclatée
                            const isExploded = (viewMode === 2)
                            const isCutaway = (viewMode === 1)
                            const fanOffset = isExploded ? -0.85 : 0.0
                            const rearEndbellOffset = isExploded ? -0.45 : 0.0
                            const frontEndbellOffset = isExploded ? 0.45 : 0.0
                            const couplingOffset = isExploded ? 0.85 : 0.0
                            const loadOffset = isExploded ? 1.30 : 0.0

                            // 1. CHÂSSIS / BANC EN PROFILÉ D'ALUMINIUM RAINURÉ
                            box(-1.6, 2.5, -0.72, 0.72, -0.80, -0.62, "#2b3844")
                            box(-1.58, 2.48, -0.22, 0.22, -0.63, -0.61, "#1e2933") // Rainure en T centrale
                            box(-1.58, 2.48, -0.58, -0.42, -0.63, -0.61, "#1e2933") // Rainures latérales
                            box(-1.58, 2.48, 0.42, 0.58, -0.63, -0.61, "#1e2933")
                            // Pieds antivibratoires sous le banc
                            for (const px of [-1.45, 2.35]) {
                                for (const py of [-0.58, 0.58]) {
                                    cylinderX(px - 0.05, px + 0.05, 0.08, 12, "#181f26", 0)
                                }
                            }

                            // 2. MOTEUR CC : PATTES ET SOCLE DE FIXATION
                            box(-0.82, -0.54, -0.42, 0.42, -0.62, -0.48, "#4b5966")
                            box(0.54, 0.82, -0.42, 0.42, -0.62, -0.48, "#4b5966")
                            for (const x of [-0.68, 0.68]) {
                                for (const y of [-0.34, 0.34]) {
                                    cylinderX(x - 0.02, x + 0.02, 0.045, 10, "#9eaab3", y > 0 ? Math.PI / 4 : 0)
                                }
                            }

                            // 3. STATOR DU MOTEUR CC
                            const statorHighlight = (inspected === "stator") ? "#5ef0d0" : "#3c4d5c"
                            if (isCutaway) {
                                // Coupe à 180° : moitié inférieure du carter fermée, moitié supérieure ouverte !
                                cylinderX(-0.72, 0.72, 0.49, 20, statorHighlight, Math.PI, Math.PI)
                                for (let x = -0.62; x <= 0.63; x += 0.12) {
                                    cylinderX(x, x + 0.028, 0.535, 20, "#5b6975", Math.PI, Math.PI)
                                }
                                // Pôles magnétiques statoriques (Nord & Sud) visibles à l'intérieur !
                                box(-0.55, 0.40, -0.46, -0.34, -0.15, 0.15, "#1e3a5f") // Pôle Nord bleu
                                box(-0.55, 0.40, 0.34, 0.46, -0.15, 0.15, "#7f1d1d")  // Pôle Sud rouge
                                // Enroulements inducteurs statoriques en cuivre
                                cylinderX(-0.50, 0.35, 0.44, 16, "#c27838", Math.PI * 0.8, Math.PI * 0.4)
                                cylinderX(-0.50, 0.35, 0.44, 16, "#c27838", Math.PI * 1.8, Math.PI * 0.4)
                            } else {
                                // Carcasse fermée complète avec ailettes de refroidissement
                                cylinderX(-0.72, 0.72, 0.49, 28, statorHighlight)
                                for (let x = -0.62; x <= 0.63; x += 0.12) {
                                    cylinderX(x, x + 0.028, 0.535, 28, "#5b6975")
                                }
                            }

                            // Flasque arrière et avant
                            cylinderX(-0.83 + rearEndbellOffset, -0.71 + rearEndbellOffset, 0.47, 26, "#7c8994")
                            cylinderX(0.71 + frontEndbellOffset, 0.84 + frontEndbellOffset, 0.47, 26, "#8895a0")

                            // Boîte à bornes supérieure avec presse-étoupes et câbles
                            box(-0.31, 0.30, -0.18, 0.18, 0.47, 0.70, "#364754")
                            box(-0.34, 0.33, -0.20, 0.20, 0.68, 0.73, inspected === "terminal_box" ? "#5ef0d0" : "#6c7a86")
                            // Câbles d'alimentation (Rouge + et Noir GND)
                            box(-0.15, -0.05, 0.18, 0.35, 0.56, 0.62, "#dc2626") // Câble rouge positif
                            box(0.05, 0.15, 0.18, 0.35, 0.56, 0.62, "#18181b")  // Câble noir négatif

                            // 4. ROTOR / INDUIT TOURNANT DU MOTEUR CC
                            // L'angle de rotation suit rotorAngle
                            const rotorHighlight = (inspected === "rotor") ? "#fcd34d" : "#556473"
                            // Noyau cylindrique de l'induit en tôles magnétiques feuilletées
                            cylinderX(-0.55, 0.38, 0.33, 24, rotorHighlight, rotorAngle)
                            // Enroulements induits en cuivre (spires visibles dans les encoches du rotor !)
                            const coilCount = 8
                            for (let c = 0; c < coilCount; c++) {
                                const ca = rotorAngle + 2 * Math.PI * c / coilCount
                                const cr = 0.335
                                const p1 = [-0.56, cr * Math.cos(ca), cr * Math.sin(ca)]
                                const p2 = [0.39, cr * Math.cos(ca), cr * Math.sin(ca)]
                                const p3 = [0.39, cr * Math.cos(ca + 0.15), cr * Math.sin(ca + 0.15)]
                                const p4 = [-0.56, cr * Math.cos(ca + 0.15), cr * Math.sin(ca + 0.15)]
                                addQuad(p1, p2, p3, p4, inspected === "rotor" ? "#f59e0b" : "#d97736")
                            }
                            // Chignon de bobinage aux deux extrémités de l'induit
                            cylinderX(-0.62, -0.55, 0.31, 20, "#c26d2e", rotorAngle)
                            cylinderX(0.38, 0.43, 0.31, 20, "#c26d2e", rotorAngle)

                            // 5. COLLECTEUR À LAMES ET PORTE-BALAIS (COMMUTATEUR)
                            const commHighlight = (inspected === "commutator") ? "#5ef0d0" : "#d97736"
                            cylinderX(0.44, 0.62, 0.19, 20, commHighlight, rotorAngle)
                            // Balais en graphite et porte-balais en laiton
                            box(0.48, 0.58, -0.06, 0.06, 0.19, 0.32, inspected === "commutator" ? "#5ef0d0" : "#232323") // Balai supérieur (charbon)
                            box(0.47, 0.59, -0.07, 0.07, 0.31, 0.37, "#ca8a04") // Porte-balai laiton
                            box(0.48, 0.58, -0.06, 0.06, -0.32, -0.19, inspected === "commutator" ? "#5ef0d0" : "#232323") // Balai inférieur

                            // Ventilateur arrière
                            cylinderX(-0.95 + fanOffset, -0.84 + fanOffset, 0.38, 22, "#34424e")
                            for (let b = 0; b < 6; b++) {
                                const ba = rotorAngle + 2 * Math.PI * b / 6
                                const bp = (r, a, x) => [x, r * Math.cos(a), r * Math.sin(a)]
                                addQuad(bp(0.12, ba - 0.16, -0.86 + fanOffset), bp(0.36, ba + 0.1, -0.86 + fanOffset), bp(0.36, ba + 0.22, -0.92 + fanOffset), bp(0.12, ba + 0.22, -0.92 + fanOffset), "#c26d2e")
                            }

                            // 6. ARBRE DE SORTIE ET ACCOUPLEMENT MÉCANIQUE
                            const shaftHighlight = (inspected === "shaft") ? "#5ef0d0" : "#cbd5e1"
                            cylinderX(-0.88, 1.12, 0.075, 20, shaftHighlight, rotorAngle)
                            // Manchon d'accouplement élastique (rouge anodisé avec vis)
                            const coupX1 = 1.12 + couplingOffset
                            const coupX2 = 1.34 + couplingOffset
                            cylinderX(coupX1, coupX2, 0.165, 20, inspected === "shaft" ? "#5ef0d0" : "#dc2626", rotorAngle)
                            ringX(coupX1, 0.076, 0.165, 16, "#991b1b")
                            ringX(coupX2, 0.076, 0.165, 16, "#991b1b")
                            cylinderX(coupX2, 1.85 + loadOffset, 0.075, 20, shaftHighlight, rotorAngle) // Arbre côté charge

                            // 7. CHARGE MÉCANIQUE : BANC DE FREIN DYNAMOMÉTRIQUE
                            const loadHighlight = (inspected === "load_brake") ? "#5ef0d0" : "#3b4854"
                            const lx = 1.45 + loadOffset
                            // Support et palier du frein
                            box(lx, lx + 0.45, -0.32, 0.32, -0.62, -0.42, "#475569")
                            box(lx + 0.06, lx + 0.38, -0.18, 0.18, -0.42, 0.48, loadHighlight)
                            // Volant d'inertie / disque de frein ventilé monté sur l'arbre
                            const diskX = lx + 0.22
                            cylinderX(diskX - 0.04, diskX + 0.04, 0.45, 26, inspected === "load_brake" ? "#fcd34d" : "#94a3b8", rotorAngle)
                            ringX(diskX - 0.04, 0.12, 0.45, 24, "#64748b", rotorAngle)
                            ringX(diskX + 0.04, 0.12, 0.45, 24, "#64748b", rotorAngle)
                            // Étriers de freinage et capteur dynamométrique
                            box(diskX - 0.07, diskX + 0.07, 0.32, 0.52, 0.18, 0.48, "#ef4444")
                            box(diskX - 0.06, diskX + 0.06, 0.38, 0.46, 0.48, 0.65, "#e2e8f0") // Cadran dynamomètre

                            // 8. CÂBLAGE COMPLET DU BANC D'ESSAI
                            const cabHighlight = (inspected === "cabling")
                            // Bornier de table / pupitre de connexion sur profilé
                            box(-1.48, -1.18, 0.36, 0.64, -0.62, -0.46, "#1e293b")
                            cylinderX(-1.42, -1.30, 0.045, 12, "#ef4444") // Borne rouge (+)
                            cylinderX(-1.42, -1.30, 0.045, 12, "#18181b") // Borne noire (-)
                            box(-1.38, -1.22, 0.40, 0.48, -0.46, -0.36, "#ef4444") // Fiche banane rouge
                            box(-1.38, -1.22, 0.52, 0.60, -0.46, -0.36, "#18181b") // Fiche banane noire

                            // Câble rouge (+) d'induit avec courbe souple
                            const redPts = [
                                [-1.30, 0.44, -0.36],
                                [-1.15, 0.40, -0.12],
                                [-0.88, 0.34, 0.18],
                                [-0.60, 0.26, 0.44],
                                [-0.35, 0.20, 0.58],
                                [-0.10, 0.16, 0.62]
                            ]
                            tube3D(redPts, 0.020, 8, cabHighlight ? "#ff6b6b" : "#dc2626")

                            // Câble noir (-) d'induit avec courbe souple
                            const blackPts = [
                                [-1.30, 0.56, -0.36],
                                [-1.12, 0.50, -0.10],
                                [-0.85, 0.42, 0.20],
                                [-0.56, 0.34, 0.46],
                                [-0.28, 0.26, 0.60],
                                [0.10, 0.16, 0.62]
                            ]
                            tube3D(blackPts, 0.020, 8, cabHighlight ? "#71717a" : "#18181b")

                            // Tresse de terre équipotentielle (Verte)
                            const earthPts = [
                                [0.55, 0.38, -0.48],
                                [0.50, 0.46, -0.55],
                                [0.35, 0.52, -0.60]
                            ]
                            tube3D(earthPts, 0.015, 6, "#16a34a")

                            // Câble capteur tachymètre / frein
                            const sensorPts = [
                                [lx + 0.15, 0.42, 0.12],
                                [lx + 0.05, 0.50, -0.22],
                                [1.10, 0.55, -0.55],
                                [0.40, 0.58, -0.60],
                                [-0.40, 0.58, -0.60],
                                [-1.10, 0.58, -0.55]
                            ]
                            tube3D(sensorPts, 0.018, 6, "#64748b")

                            // Sonde de mesure oscilloscope BNC
                            const probePts = [
                                [-0.10, 0.16, 0.62],
                                [-0.05, 0.30, 0.72],
                                [0.20, 0.40, 0.45],
                                [0.40, 0.48, 0.10],
                                [0.60, 0.54, -0.30],
                                [0.80, 0.58, -0.58]
                            ]
                            tube3D(probePts, 0.014, 6, "#38bdf8")

                            // PROJECTION 3D ET RENDU
                            const cy = Math.cos(yaw), sy = Math.sin(yaw)
                            const ce = Math.cos(elevation), se = Math.sin(elevation)
                            const baseScale = Math.min(width / 3.4, height / 2.2) * zoom
                            const cx = width * 0.46, cz = height * 0.52

                            const project = (p) => {
                                const x = p[0], y = p[1], z = p[2]
                                const u = x * cy - y * sy
                                const v = x * sy + y * cy
                                return [cx + baseScale * u, cz + baseScale * (v * se - z * ce), v * ce + z * se]
                            }

                            const shade = (color, n) => {
                                const light = [-0.45, -0.65, 0.62]
                                const len = Math.hypot(...n) || 1
                                const d = Math.max(0, (n[0] * light[0] + n[1] * light[1] + n[2] * light[2]) / len / Math.hypot(...light))
                                const k = 0.55 + 0.55 * d
                                const hex = color.slice(1)
                                const r = parseInt(hex.slice(0, 2), 16), g = parseInt(hex.slice(2, 4), 16), b = parseInt(hex.slice(4, 6), 16)
                                return `rgb(${Math.min(255, Math.round(r * k))},${Math.min(255, Math.round(g * k))},${Math.min(255, Math.round(b * k))})`
                            }

                            const projected = triangles.map(t => {
                                const a = project(t[0]), b = project(t[1]), c = project(t[2])
                                const ab = [b[0] - a[0], b[1] - a[1], b[2] - a[2]]
                                const ac = [c[0] - a[0], c[1] - a[1], c[2] - a[2]]
                                const n = [ab[1] * ac[2] - ab[2] * ac[1], ab[2] * ac[0] - ab[0] * ac[2], ab[0] * ac[1] - ab[1] * ac[0]]
                                return { a, b, c, z: (a[2] + b[2] + c[2]) / 3, color: shade(t[3], n) }
                            }).sort((a, b) => a.z - b.z)

                            const ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)

                            // Dessin des polygones triés
                            for (const t of projected) {
                                ctx.beginPath()
                                ctx.moveTo(t.a[0], t.a[1])
                                ctx.lineTo(t.b[0], t.b[1])
                                ctx.lineTo(t.c[0], t.c[1])
                                ctx.closePath()
                                ctx.fillStyle = t.color
                                ctx.fill()
                                ctx.strokeStyle = "#1a2632"
                                ctx.lineWidth = 0.25
                                ctx.stroke()
                            }

                            // Incrustation d'informations techniques
                            ctx.font = "bold 11px sans-serif"
                            ctx.fillStyle = "#5ef0d0"
                            ctx.fillText(qsTr("BANC D'ESSAI MOTEUR CC · MULTIPHYSIQUE 3D"), 14, 20)
                            ctx.font = "10px sans-serif"
                            ctx.fillStyle = "#94a3b8"
                            const modeStr = (viewMode === 1) ? qsTr("Vue en coupe : Stator (inducteur) & Rotor (induit bobiné)")
                                          : (viewMode === 2) ? qsTr("Vue éclatée : Décomposition axiale des sous-ensembles")
                                          : qsTr("Vue extérieure : Machine industrielle sur banc profilé alu")
                            ctx.fillText(modeStr, 14, 35)

                            // Fiche technique de la pièce inspectée
                            ctx.fillStyle = "rgba(12, 21, 30, 0.85)"
                            ctx.fillRect(10, height - 52, width - 20, 42)
                            ctx.strokeStyle = "#253a4c"
                            ctx.lineWidth = 1
                            ctx.strokeRect(10, height - 52, width - 20, 42)
                            ctx.font = "bold 11px sans-serif"
                            ctx.fillStyle = "#38bdf8"
                            let title = "", desc = ""
                            if (inspected === "rotor") {
                                title = qsTr("ROTOR (INDUIT) :")
                                desc = qsTr("Paquet de tôles magnétiques au silicium avec 8 encoches et bobinage cuivre émaillé en rotation. Siège de la f.é.m. induite e = Ke·ω.")
                            } else if (inspected === "stator") {
                                title = qsTr("STATOR (INDUCTEUR) :")
                                desc = qsTr("Carcasse cylindrique en fonte d'alu à ailettes avec pôles magnétiques générant le flux d'induction permanent.")
                            } else if (inspected === "commutator") {
                                title = qsTr("COLLECTEUR & BALAIS :")
                                desc = qsTr("Bague de cuivre segmentée à lames isolées au mica et balais électrographite assurant la commutation du courant dans l'induit.")
                            } else if (inspected === "shaft") {
                                title = qsTr("ARBRE & ACCOUPLEMENT :")
                                desc = qsTr("Arbre en acier rectifié C45E et manchon élastique reliant le couple électromagnétique Cem = Kt·i à la charge mécanique.")
                            } else if (inspected === "load_brake") {
                                title = qsTr("FREIN DYNAMOMÉTRIQUE :")
                                desc = qsTr("Disque ventilé en rotation avec étriers de freinage appliquant un couple résistant Cr réglable (J·dω/dt = Cem - Cr - f·ω).")
                            } else {
                                title = qsTr("CÂBLAGE ÉLECTRIQUE DU BANC :")
                                desc = qsTr("Câbles de puissance extra-souples 2,5 mm² à fiches bananes 4mm (Rouge +, Noir -), tresse de terre verte et sondes BNC.")
                            }
                            ctx.fillText(title, 20, height - 32)
                            ctx.font = "10px sans-serif"
                            ctx.fillStyle = "#cbd5e1"
                            ctx.fillText(desc, 20, height - 17)
                        }
                        onPaint: renderMotor()
                        MouseArea {
                            anchors.fill: parent; property point previous
                            onPressed: function(mouse) { previous = Qt.point(mouse.x, mouse.y) }
                            onPositionChanged: function(mouse) {
                                if (pressed) {
                                    motorScene.yaw += (mouse.x - previous.x) * 0.009
                                    motorScene.elevation = Math.max(0.12, Math.min(1.42, motorScene.elevation + (mouse.y - previous.y) * 0.006))
                                    previous = Qt.point(mouse.x, mouse.y)
                                    motorScene.requestPaint()
                                }
                            }
                            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        }
                    }

                    // Sélecteur de pièce rapide en haut à droite
                    RowLayout {
                        anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 10
                        spacing: 4
                        Label { text: qsTr("Inspecter :"); color: "#64748b"; font.pixelSize: 9; font.bold: true }
                        Repeater {
                            model: [
                                { id: "rotor", label: qsTr("Rotor") },
                                { id: "stator", label: qsTr("Stator") },
                                { id: "commutator", label: qsTr("Collecteur") },
                                { id: "shaft", label: qsTr("Arbre") },
                                { id: "load_brake", label: qsTr("Frein") },
                                { id: "cabling", label: qsTr("Câblage") }
                            ]
                            delegate: Button {
                                required property var modelData
                                Layout.preferredHeight: 22
                                text: modelData.label
                                highlighted: window.motorInspectedPart === modelData.id
                                contentItem: Label { text: parent.text; color: parent.highlighted ? "#38bdf8" : "#94a3b8"; font.pixelSize: 9; font.bold: parent.highlighted }
                                background: Rectangle { radius: 4; color: parent.highlighted ? "#082f49" : "#111a24"; border.color: parent.highlighted ? "#38bdf8" : "#253444" }
                                onClicked: { window.motorInspectedPart = modelData.id; motorScene.requestPaint() }
                            }
                        }
                    }
                }

                // 5. INSTRUMENT : OSCILLOSCOPE NUMÉRIQUE DE BANC (DSO)
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 200; Layout.minimumHeight: 160
                    radius: 10; color: "#071118"; border.color: "#1c3242"
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 8; spacing: 4
                        // Barre de statut de l'oscilloscope
                        RowLayout {
                            Layout.fillWidth: true; spacing: 8
                            Rectangle { width: 8; height: 8; radius: 4; color: "#22c55e" }
                            Label { text: qsTr("OSCILLOSCOPE NUMÉRIQUE (DSO)"); color: "#eff6fb"; font.pixelSize: 10; font.bold: true }
                            Rectangle { width: 10; height: 3; color: "#4ee3c2" }
                            Label { text: qsTr("CH1 : Courant induit i(t) [A]"); color: "#4ee3c2"; font.pixelSize: 9; font.bold: true }
                            Rectangle { width: 10; height: 3; color: "#fbb358" }
                            Label { text: qsTr("CH2 : Vitesse angulaire ω(t) [rad/s]"); color: "#fbb358"; font.pixelSize: 9; font.bold: true }
                            Item { Layout.fillWidth: true }
                            Label {
                                text: qsTr("Curseur t = ") + window.motorPlaybackTime.toFixed(3) + " s"
                                color: "#fcd34d"; font.pixelSize: 9; font.bold: true; font.family: "Consolas, monospace"
                            }
                            Button {
                                Layout.preferredHeight: 22; text: window.motorScopePaused ? qsTr("▶ Reprendre") : qsTr("❚❚ Pause")
                                contentItem: Label { text: parent.text; color: "#94a3b8"; font.pixelSize: 9 }
                                background: Rectangle { radius: 4; color: "#162330"; border.color: "#2a3d4f" }
                                onClicked: window.motorScopePaused = !window.motorScopePaused
                            }
                            Button {
                                Layout.preferredHeight: 22; text: qsTr("↺ Rejouer échelon")
                                contentItem: Label { text: parent.text; color: "#5ef0d0"; font.pixelSize: 9; font.bold: true }
                                background: Rectangle { radius: 4; color: "#134e48"; border.color: "#2dd4bf" }
                                onClicked: {
                                    window.motorPlaybackTime = 0
                                    window.motorRotorAngle = 0
                                    window.motorScopePaused = false
                                    motorPlot.requestPaint()
                                    motorScene.requestPaint()
                                }
                            }
                        }

                        // Écran CRT de l'oscilloscope
                        Canvas {
                            id: motorPlot
                            Layout.fillWidth: true; Layout.fillHeight: true
                            onPaint: {
                                const ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                const left = 48, right = 54, top = 8, bottom = 22
                                const w = width - left - right, h = height - top - bottom

                                // Fond d'écran vert phosphore sombre
                                ctx.fillStyle = "#041411"
                                ctx.fillRect(left, top, w, h)

                                // Grille de mesure oscilloscope (8 divisions verticales x 10 divisions horizontales)
                                ctx.strokeStyle = "#0d352b"
                                ctx.lineWidth = 1
                                for (let i = 0; i <= 8; i++) {
                                    const y = top + h * i / 8
                                    ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(left + w, y); ctx.stroke()
                                }
                                for (let j = 0; j <= 10; j++) {
                                    const x = left + w * j / 10
                                    ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, top + h); ctx.stroke()
                                }

                                // Réticule central
                                ctx.strokeStyle = "#1b5a4a"
                                ctx.lineWidth = 1.2
                                ctx.beginPath()
                                ctx.moveTo(left, top + h / 2); ctx.lineTo(left + w, top + h / 2)
                                ctx.moveTo(left + w / 2, top); ctx.lineTo(left + w / 2, top + h)
                                ctx.stroke()

                                // Graduations des axes
                                ctx.fillStyle = "#638980"
                                ctx.font = "9px sans-serif"
                                ctx.textBaseline = "middle"
                                ctx.textAlign = "right"
                                ctx.fillText("A", left - 6, top + 6)
                                ctx.textAlign = "left"
                                ctx.fillText("rad/s", left + w + 6, top + 6)

                                ctx.textBaseline = "top"
                                ctx.textAlign = "left"
                                ctx.fillText("0 ms", left, top + h + 4)
                                ctx.textAlign = "center"
                                ctx.fillText("250 ms", left + w / 2, top + h + 4)
                                ctx.textAlign = "right"
                                ctx.fillText("500 ms", left + w, top + h + 4)

                                const data = lab.motorSamples
                                if (!data || data.length < 2) {
                                    ctx.fillStyle = "#2dd4bf"
                                    ctx.textAlign = "center"
                                    ctx.textBaseline = "middle"
                                    ctx.font = "11px sans-serif"
                                    ctx.fillText(qsTr("Lancez la simulation pour acquérir les voies CH1 et CH2"), width / 2, height / 2)
                                    return
                                }

                                let minI = Infinity, maxI = -Infinity, minW = Infinity, maxW = -Infinity
                                for (const s of data) {
                                    minI = Math.min(minI, s.current)
                                    maxI = Math.max(maxI, s.current)
                                    minW = Math.min(minW, s.omega)
                                    maxW = Math.max(maxW, s.omega)
                                }
                                if (maxI - minI < 1e-12) { minI -= 0.5; maxI += 0.5 }
                                if (maxW - minW < 1e-12) { minW -= 0.5; maxW += 0.5 }

                                // Voie CH1 (Cyan #4ee3c2) : Courant induit i(t)
                                ctx.beginPath()
                                for (let j = 0; j < data.length; j++) {
                                    const x = left + w * data[j].time / 0.5
                                    const y = top + h * (maxI - data[j].current) / (maxI - minI)
                                    if (j === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                                }
                                ctx.strokeStyle = "#4ee3c2"
                                ctx.lineWidth = 2.2
                                ctx.stroke()

                                // Voie CH2 (Ambre #fbb358) : Vitesse angulaire omega(t)
                                ctx.beginPath()
                                for (let j = 0; j < data.length; j++) {
                                    const x = left + w * data[j].time / 0.5
                                    const y = top + h * (maxW - data[j].omega) / (maxW - minW)
                                    if (j === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                                }
                                ctx.strokeStyle = "#fbb358"
                                ctx.lineWidth = 2.2
                                ctx.stroke()

                                // Curseur temporel de balayage vertical
                                const curT = Math.min(0.5, window.motorPlaybackTime)
                                const curX = left + w * curT / 0.5
                                ctx.beginPath()
                                ctx.setLineDash([4, 3])
                                ctx.moveTo(curX, top)
                                ctx.lineTo(curX, top + h)
                                ctx.strokeStyle = "#fcd34d"
                                ctx.lineWidth = 1.2
                                ctx.stroke()
                                ctx.setLineDash([])

                                // Points d'échantillon instantanés sous le curseur
                                let sampleA = data[0], sampleB = data[data.length - 1]
                                for (let k = 1; k < data.length; k++) {
                                    if (data[k].time >= curT) { sampleA = data[k - 1]; sampleB = data[k]; break }
                                }
                                const frac = (sampleB.time === sampleA.time) ? 0 : (curT - sampleA.time) / (sampleB.time - sampleA.time)
                                const curCurrent = sampleA.current + (sampleB.current - sampleA.current) * frac
                                const curOmega = sampleA.omega + (sampleB.omega - sampleA.omega) * frac
                                const dotY1 = top + h * (maxI - curCurrent) / (maxI - minI)
                                const dotY2 = top + h * (maxW - curOmega) / (maxW - minW)

                                ctx.fillStyle = "#ffffff"
                                ctx.beginPath(); ctx.arc(curX, dotY1, 4, 0, 2 * Math.PI); ctx.fill()
                                ctx.beginPath(); ctx.arc(curX, dotY2, 4, 0, 2 * Math.PI); ctx.fill()

                                // Valeurs aux divisions
                                ctx.fillStyle = "#4ee3c2"
                                ctx.textAlign = "right"
                                ctx.textBaseline = "top"
                                ctx.fillText(maxI.toFixed(2) + " A", left - 4, top)
                                ctx.textBaseline = "bottom"
                                ctx.fillText(minI.toFixed(2) + " A", left - 4, top + h)

                                ctx.fillStyle = "#fbb358"
                                ctx.textAlign = "left"
                                ctx.textBaseline = "top"
                                ctx.fillText(maxW.toFixed(1) + " rad/s", left + w + 4, top)
                                ctx.textBaseline = "bottom"
                                ctx.fillText(minW.toFixed(1) + " rad/s", left + w + 4, top + h)
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: mnaWorkspace
        anchors.fill: parent; anchors.margins: 18
        visible: window.workspace === 1 && window.labMode === 2
        color: "#0b1118"
        RowLayout {
            anchors.fill: parent; spacing: 16
            Rectangle {
                Layout.preferredWidth: 300; Layout.fillHeight: true
                radius: 12; color: "#111a24"; border.color: "#253444"
                ScrollView { anchors.fill: parent; anchors.margins: 18; clip: true
                    ColumnLayout { width: 264; spacing: 13
                        Label { text: qsTr("Circuit RC linéaire"); color: "#eff6fb"; font.pixelSize: 18; font.bold: true }
                        Label { text: qsTr("Le moteur MNA résout le réseau nodal à chaque pas. Modifiez les trois valeurs et observez la tension du condensateur."); color: "#91a2b2"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#253444" }
                        Label { text: qsTr("PARAMÈTRES DU RÉSEAU"); color: "#64c5b5"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.7 }
                        Label { text: qsTr("Source idéale U"); color: "#dce6ee"; font.pixelSize: 12 }
                        RowLayout { Layout.fillWidth: true
                            RealSpinBox { Layout.fillWidth: true; realFrom: -1000; realTo: 1000; realValue: lab.mnaVoltage; decimalPlaces: 2; realStep: 0.5; enabled: !lab.running; onRealValueEdited: function(v) { lab.mnaVoltage = v } }
                            Label { text: "V"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                        }
                        Label { text: qsTr("Résistance série R"); color: "#dce6ee"; font.pixelSize: 12 }
                        RowLayout { Layout.fillWidth: true
                            RealSpinBox { Layout.fillWidth: true; realFrom: 0.001; realTo: 1000000; realValue: lab.mnaResistanceOhms; decimalPlaces: 3; realStep: 10; enabled: !lab.running; onRealValueEdited: function(v) { lab.mnaResistanceOhms = v } }
                            Label { text: "Ω"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                        }
                        Label { text: qsTr("Condensateur C"); color: "#dce6ee"; font.pixelSize: 12 }
                        RowLayout { Layout.fillWidth: true
                            RealSpinBox { Layout.fillWidth: true; realFrom: 0.000000001; realTo: 100; realValue: lab.mnaCapacitanceFarads; decimalPlaces: 9; realStep: 0.0000001; enabled: !lab.running; onRealValueEdited: function(v) { lab.mnaCapacitanceFarads = v } }
                            Label { text: "F"; color: "#91a2b2"; Layout.preferredWidth: 24 }
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#253444" }
                        Label { text: qsTr("CONFIGURATION TEMPORELLE"); color: "#64c5b5"; font.pixelSize: 10; font.bold: true; font.letterSpacing: 0.6 }
                        Label { text: qsTr("Durée : 10 ms   ·   pas : 0,1 ms\nEuler implicite   ·   100 pas de calcul"); color: "#91a2b2"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        Label { text: qsTr("MNA dense, maximum 256 inconnues. Les circuits flottants ou singuliers sont rejetés avec un diagnostic."); color: "#738596"; font.pixelSize: 10; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumWidth: 0; spacing: 12
                RowLayout { Layout.fillWidth: true
                    ColumnLayout { spacing: 2
                        Label { text: qsTr("Réponse nodale du condensateur"); color: "#eff6fb"; font.pixelSize: 20; font.bold: true }
                        Label { text: qsTr("Circuit RC série soumis à un échelon de tension · analyse MNA"); color: "#91a2b2"; font.pixelSize: 11 }
                    }
                    Item { Layout.fillWidth: true }
                    Label { text: lab.mnaSampleCount ? qsTr("RÉSULTAT MNA") : qsTr("EN ATTENTE"); color: lab.mnaSampleCount ? "#77d6bd" : "#a5b2bd"; font.pixelSize: 10; font.bold: true }
                }
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 72; radius: 9; color: "#111a24"; border.color: "#253444"
                    Canvas { id: mnaDiagram; anchors.fill: parent; anchors.margins: 8
                        onPaint: {const c=getContext("2d");c.clearRect(0,0,width,height);const y=height/2;c.strokeStyle="#61d4bb";c.lineWidth=2;c.beginPath();c.moveTo(50,y);c.lineTo(width-50,y);c.stroke()
                            const box=(x,title,sub,col)=>{c.fillStyle=col;c.strokeStyle="#4a626d";c.lineWidth=1.5;c.fillRect(x-42,y-22,84,44);c.strokeRect(x-42,y-22,84,44);c.textAlign="center";c.fillStyle="#eff6fb";c.font="bold 12px sans-serif";c.fillText(title,x,y-2);c.fillStyle="#9aabba";c.font="10px sans-serif";c.fillText(sub,x,y+13)}
                            box(55,"U","DC","#173c3a");box(width*0.42,"R",lab.mnaResistanceOhms.toPrecision(3)+" Ω","#293540");box(width*0.78,"C",lab.mnaCapacitanceFarads.toExponential(1)+" F","#253b49");c.fillStyle="#91a2b2";c.font="9px sans-serif";c.fillText("0 V",width-28,y+34)
                        }
                    }
                }
                Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 220; radius: 12; color: "#111a24"; border.color: "#253444"
                    Canvas { id: mnaPlot; anchors.fill: parent; anchors.margins: 16
                        onPaint: {const c=getContext("2d");c.clearRect(0,0,width,height);const left=64,right=20,top=20,bottom=42,w=width-left-right,h=height-top-bottom;const data=lab.mnaSamples
                            let maxV=Math.max(1e-9,Math.abs(lab.mnaVoltage))*1.08
                            c.font="10px sans-serif";c.strokeStyle="#293844";c.fillStyle="#8c9cab";c.lineWidth=1
                            for(let i=0;i<=4;i++){const y=top+h*i/4;c.beginPath();c.moveTo(left,y);c.lineTo(left+w,y);c.stroke();c.textAlign="right";c.textBaseline="middle";c.fillText((maxV*(4-i)/4).toPrecision(3)+" V",left-9,y)}
                            for(let i=0;i<=4;i++){const x=left+w*i/4;c.beginPath();c.moveTo(x,top);c.lineTo(x,top+h);c.stroke();c.textAlign=i===0?"left":(i===4?"right":"center");c.textBaseline="top";c.fillText((0.01*i/4).toPrecision(2)+" s",x,top+h+9)}
                            c.save();c.translate(15,top+h/2);c.rotate(-Math.PI/2);c.textAlign="center";c.fillStyle="#b8c5d0";c.fillText(qsTr("Tension du condensateur (V)"),0,0);c.restore()
                            if(data.length<2){c.textAlign="center";c.textBaseline="middle";c.fillStyle="#718294";c.font="13px sans-serif";c.fillText(qsTr("Lancez l'analyse pour calculer la réponse du réseau"),left+w/2,top+h/2);return}
                            c.beginPath();for(let i=0;i<data.length;i++){const x=left+w*data[i].time/0.01,y=top+h*(maxV-data[i].voltage)/maxV;if(i===0)c.moveTo(x,y);else c.lineTo(x,y)}c.strokeStyle="#61d4bb";c.lineWidth=2.5;c.stroke()
                        }
                    }
                }
                RowLayout { Layout.fillWidth: true; spacing: 10
                    Repeater { model: [qsTr("CONSTANTE RC"),qsTr("TENSION FINALE"),qsTr("ÉCHANTILLONS")]
                        delegate: Rectangle { required property int index; required property string modelData
                            Layout.fillWidth: true; Layout.minimumWidth: 0; Layout.preferredHeight: 70; radius: 9; color: "#111a24"; border.color: "#253444"
                            ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 3
                                Label { text: modelData; color: "#8092a2"; font.pixelSize: 9; font.bold: true }
                                Label { text: lab.mnaSampleCount ? (index===0?(lab.mnaResistanceOhms*lab.mnaCapacitanceFarads).toPrecision(4)+" s":(index===1?lab.mnaFinalVoltage.toPrecision(5)+" V":lab.mnaSampleCount.toString())) : "—"; color: "#eff6fb"; font.pixelSize: 17; font.bold: true }
                            }
                        }
                    }
                }
                Label { text: lab.mnaSampleCount ? lab.statusText : qsTr("Circuit préassemblé illustratif. Le placement libre des composants et les fils interactifs arrivent avec l'éditeur de schéma."); color: "#91a2b2"; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
            }
        }
    }

    Rectangle {
        id: workshopWorkspace
        anchors.fill: parent; anchors.margins: 18
        visible: window.workspace === 0
        color: "#0b1118"
        RowLayout {
            anchors.fill: parent; spacing: 16
            Rectangle {
                Layout.preferredWidth: 260; Layout.fillHeight: true; radius: 12; color: "#111a24"; border.color: "#253444"
                ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 10
                    Label { text: qsTr("Bibliothèque"); color: "#eff6fb"; font.pixelSize: 18; font.bold: true }
                    Label { text: qsTr("ÉLECTRICITÉ"); color: "#64c5b5"; font.pixelSize: 10; font.bold: true }
                    Repeater { model: [qsTr("Source DC"),qsTr("Moteur CC"),qsTr("Charge mécanique")]
                        delegate: Button { required property string modelData; text: "+  "+modelData; Layout.fillWidth: true; enabled: false; ToolTip.visible: hovered; ToolTip.text: qsTr("Insertion libre et connexions arbitraires : étape ultérieure") }
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: "#253444" }
                    Label { text: qsTr("Le banc moteur préconfiguré est déjà simulable dans l’espace Laboratoire."); color: "#91a2b2"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 11 }
                    Item { Layout.fillHeight: true }
                }
            }
            ColumnLayout { Layout.fillWidth: true; Layout.fillHeight: true; spacing: 12
                Label { text: qsTr("Atelier de conception"); color: "#eff6fb"; font.pixelSize: 22; font.bold: true }
                Label { text: qsTr("Vue fonctionnelle du banc moteur CC — les traits représentent le circuit modélisé E-002."); color: "#91a2b2"; font.pixelSize: 12 }
                Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; radius: 12; color: "#111a24"; border.color: "#253444"
                    Canvas { anchors.fill: parent; anchors.margins: 20
                        onPaint: {const c=getContext("2d");c.clearRect(0,0,width,height);const y=height/2
                            c.strokeStyle="#61d4bb";c.lineWidth=3;c.beginPath();c.moveTo(90,y);c.lineTo(width-220,y);c.stroke()
                            const node=(x,label,sub,color="#17232e")=>{c.fillStyle=color;c.strokeStyle="#507080";c.lineWidth=2;c.fillRect(x-68,y-48,136,96);c.strokeRect(x-68,y-48,136,96);c.fillStyle="#eff6fb";c.font="bold 14px sans-serif";c.textAlign="center";c.fillText(label,x,y-5);c.fillStyle="#9aabba";c.font="11px sans-serif";c.fillText(sub,x,y+18)}
                            node(95,"Source DC",lab.motorVoltage.toFixed(1)+" V","#173c3a")
                            node(width*0.45,"Moteur CC","R, L, Ke, Kt, J, f")
                            node(width-100,"Charge","Cr = "+lab.motorLoadTorque.toFixed(3)+" N·m","#3a2a20")
                            c.fillStyle="#8193a2";c.font="12px sans-serif";c.textAlign="center";c.fillText("Circuit électrique d’induit",width*0.45,y+90)
                            c.fillText("Couplage électromécanique",width*0.72,y-80)
                        }
                    }
                }
                Label { text: qsTr("Le placement libre, les ports dessinés et le routage des fils ne sont pas encore implémentés. La commande Simuler utilise les équations du modèle dans l’espace Laboratoire."); color: "#899aa8"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 11 }
            }
        }
    }

    Rectangle {
        id: coursesWorkspace
        anchors.fill: parent; anchors.margins: 18
        visible: window.workspace === 2
        color: "#0b1118"
        ColumnLayout { anchors.fill: parent; spacing: 14
            Label { text: qsTr("Cours et travaux pratiques"); color: "#eff6fb"; font.pixelSize: 22; font.bold: true }
            Label { text: qsTr("Expériences guidées à partir des moteurs réellement disponibles."); color: "#91a2b2"; font.pixelSize: 12 }
            RowLayout { Layout.fillWidth: true; Layout.fillHeight: true; spacing: 14
                Repeater { model: [qsTr("Échelon RL · E-001"),qsTr("Moteur CC · E-002")]
                    delegate: Rectangle { required property int index; required property string modelData
                        Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumWidth: 0; radius: 12; color: "#111a24"; border.color: "#253444"
                        ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 12
                            Label { text: modelData; color: "#eff6fb"; font.pixelSize: 18; font.bold: true }
                            Label { Layout.fillWidth: true; Layout.fillHeight: true; wrapMode: Text.WordWrap; verticalAlignment: Text.AlignTop; color: "#a1b1bd"; font.pixelSize: 12
                                text: index===0 ? qsTr("Étudier la constante de temps tau=L/R et la réponse i(t) à un échelon. Modifiez R, L et le pas numérique dans le banc RL, puis comparez au résultat analytique de la fiche E-001.") : qsTr("Observer l’influence de la tension et du couple de charge sur le courant, la vitesse et le couple. Le modèle E-002 couple simultanément les équations électrique et mécanique par Euler implicite.")
                            }
                            Button { text: qsTr("Ouvrir le laboratoire"); highlighted: true; onClicked: {window.workspace=1;window.labMode=index} }
                            Label { Layout.fillWidth: true; text: qsTr("Niveau : découverte · Modèle idéal documenté · Valeurs illustratives"); color: "#778998"; font.pixelSize: 10; wrapMode: Text.WordWrap }
                        }
                    }
                }
            }
        }
    }

    Timer {
        id: motorPlayback
        interval: 16
        repeat: true
        running: window.workspace === 1 && window.labMode === 1 && lab.motorSampleCount > 1 && !window.motorScopePaused
        onTriggered: {
            const dt = (interval / 1000) * window.motorPlaybackSpeed
            const data = lab.motorSamples
            if (!data || data.length < 2) return
            if (window.motorPlaybackTime < 0.5) {
                const oldTime = window.motorPlaybackTime
                const newTime = Math.min(0.5, oldTime + dt)
                let a = data[0], b = data[data.length - 1]
                for (let i = 1; i < data.length; i++) {
                    if (data[i].time >= newTime) {
                        a = data[i - 1]
                        b = data[i]
                        break
                    }
                }
                const f = (b.time === a.time) ? 0 : (newTime - a.time) / (b.time - a.time)
                const omega = a.omega + (b.omega - a.omega) * f
                window.motorRotorAngle += omega * (newTime - oldTime)
                window.motorPlaybackTime = newTime
            } else {
                const steadyOmega = lab.motorAngularVelocity
                window.motorRotorAngle += steadyOmega * dt
            }
            motorScene.requestPaint()
            motorPlot.requestPaint()
        }
    }

    FileDialog {
        id: openDialog
        title: qsTr("Ouvrir une expérience RL")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Projet OpenLab (*.olsim)"), qsTr("Tous les fichiers (*)")]
        onAccepted: lab.loadProject(selectedFile)
    }
    FileDialog {
        id: saveDialog
        title: qsTr("Enregistrer le banc RL")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "olsim"
        nameFilters: [qsTr("Projet OpenLab (*.olsim)")]
        onAccepted: lab.saveProject(selectedFile)
    }
    FileDialog {
        id: csvDialog
        title: qsTr("Exporter les échantillons calculés")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "csv"
        nameFilters: [qsTr("Données CSV (*.csv)")]
        onAccepted: lab.exportCsv(selectedFile)
    }
    Dialog {
        id: aboutDialog
        width: 440
        modal: true
        title: qsTr("À propos d’OpenLab Simulator")
        standardButtons: Dialog.Ok
        anchors.centerIn: Overlay.overlay
        contentItem: Label {
            text: qsTr("OpenLab Simulator 0.1.0\nBanc RL série calculé par Euler implicite.\nModèle idéal ; consultez la fiche E-001 pour les hypothèses et limites.")
            color: "#dce6ed"; wrapMode: Text.WordWrap; padding: 18
        }
    }

    Connections {
        target: lab
        function onResultsChanged() { plot.requestPaint() }
        function onInputsChanged() { plot.requestPaint() }
        function onMotorResultsChanged() { window.motorPlaybackTime = 0; window.motorRotorAngle = 0; motorScene.requestPaint(); motorPlot.requestPaint() }
        function onMotorInputsChanged() { motorScene.requestPaint() }
        function onMnaResultsChanged() { mnaPlot.requestPaint() }
        function onMnaInputsChanged() { mnaDiagram.requestPaint(); mnaPlot.requestPaint() }
    }

    Component.onCompleted: {
        lab.runMotorSimulation()
    }
}
