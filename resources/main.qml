import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import IfCanvas 1.0

ApplicationWindow {
    visible: true

    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple

    menuBar: MenuBar {
    }

    header: ToolBar {
    }

    footer: TabBar {
    }

    RowLayout {
        anchors.fill: parent
    
        ColumnLayout {
            Layout.fillHeight: true
    
            Switch {
                text: "Spectrogram"
                checked: config.viewShowSpectrogram
                onToggled: config.viewShowSpectrogram = checked
            }

            Switch {
                text: "Pitch track"
                checked: config.viewShowPitch
                onToggled: config.viewShowPitch = checked
            }

            Switch {
                text: "Formant tracks"
                checked: config.viewShowFormants
                onToggled: config.viewShowFormants = checked
            }
        }

        IfCanvas {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 320
            Layout.minimumHeight: 240
        }
    }
}
