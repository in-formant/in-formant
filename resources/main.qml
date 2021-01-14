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
        id: rowLayout
        anchors.fill: parent
    
        ColumnLayout {
            Layout.fillHeight: true

            Switch {
                text: "Pitch track"
                objectName: "pitchTrack"                
                signal togglePitchTrack(bool toggled)
                onToggled: this.togglePitchTrack(checked)
            }

            Switch {
                text: "Formant tracks"
                objectName: "formantTracks"
                signal toggleFormantTracks(bool toggled)
                onToggled: this.toggleFormantTracks(checked)
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
