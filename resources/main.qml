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

    ColumnLayout {
        id: layout
        anchors.fill: parent

        IfCanvas {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
