import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: window
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
                Label { text: qsTr("LABORATOIRE  /  ÉLECTRICITÉ  /  TRANSITOIRE RL"); color: "#8293a4"; font.pixelSize: 10; font.letterSpacing: 1.1 }
            }
            Button {
                id: simulationButton
                text: lab.running ? qsTr("Annuler") : qsTr("▶  Simuler")
                highlighted: !lab.running
                onClicked: lab.runOrCancel()
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
    }
}
