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
                text: "Pitch track"
                objectName: "pitchTrack"                
                signal togglePitchTrack(toggled: bool)
                onToggled: this.togglePitchTrack(checked)
            }

            Switch {
                text: "Formant tracks"
                objectName: "formantTracks"
                signal toggleFormantTracks(toggled: bool)
                onToggled: this.toggleFormantTracks(checked)
            }
        }

        IfCanvas {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
