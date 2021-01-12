import QtQuick 2.12
import QtQuick.Controls 2.12
import IfCanvas 1.0

ApplicationWindow {
    visible: true

    menuBar: MenuBar {
    }

    header: ToolBar {
    }

    footer: TabBar {
    }

    StackView {
        anchors.fill: parent

        IfCanvas {
        }
    }
}
