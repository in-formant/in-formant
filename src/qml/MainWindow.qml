import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import IfCanvas 1.0

ApplicationWindow {
    id: mainWindow
    title: appName
    visible: true

    width: 640
    height: 480

    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple

    function mel(f) {
        return 2595 * Math.log10(1 + f / 700);
    }

    function hz(m) {
        return 700 * (Math.pow(10, m / 2595) - 1);
    }

    Shortcut {
        sequence: "P"
        onActivated: config.paused = !config.paused
    }

    header: ToolBar {
        Material.background: Material.color(Material.BlueGrey, Material.Shade800)

        RowLayout {
            anchors.fill: parent

            ToolButton {
                id: drawerButton
                icon.source: drawer.visible ? "icons/menu_open.svg" : "icons/menu.svg"
                checked: drawer.visible 
                onPressed: drawer.visible = !drawer.visible

                transform: [
                    Rotation {
                        angle: 90 * drawer.position
                        origin.x: drawerButton.x + drawerButton.width / 2
                        origin.y: drawerButton.y + drawerButton.height / 2
                    },
                    Scale {
                        xScale: 1 - 0.15 * drawer.position
                        yScale: 1 - 0.15 * drawer.position
                        origin.x: drawerButton.x + drawerButton.width / 2
                        origin.y: drawerButton.y + drawerButton.height / 2
                    }
                ]

                Component.onCompleted: {
                    icon.width = 2 * icon.width
                    icon.height = 2 * icon.height
                }
            }

            ToolButton {
                icon.source: config.paused ? "icons/play_arrow.svg" : "icons/stop.svg"
                checked: config.paused 
                onPressed: config.paused = !config.paused 
                
                Component.onCompleted: {
                    icon.width = 2 * icon.width
                    icon.height = 2 * icon.height
                }
            } 

            Button {
                visible: HAS_SYNTH
                text: "Synthesizer"
                Material.background: Material.color(Material.DeepPurple, Material.ShadeA200)
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 10
                onPressed: synthWindow.show()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Drawer {
        id: drawer
        position: 0.0
        visible: position > 0
        y: header.height
        width: sidebar.width
        height: parent.height - header.height
        modal: false

        Flickable {
            id: sidebar
            width: sidebarContent.width
            height: parent.height
            contentWidth: sidebarContent.width
            contentHeight: sidebarContent.height
            clip: true

            ScrollBar.vertical: ScrollBar {}

            Column {
                id: sidebarContent
                
                padding: 20

                ColumnLayout {

                    Behavior on x {
                        NumberAnimation {
                            easing.type: Easing.InOutQuad
                            duration: 200
                        }
                    }

                    Behavior on opacity {
                        NumberAnimation {
                            easing.type: Easing.InOutQuad
                            duration: 200
                        }
                    }

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

                    MenuSeparator {}

                    Label { text: "View frequency range:" }
                    RangeSlider {
                        from: mel(1)
                        to: mel(16000)
                        first.value: mel(config.viewMinFrequency)
                        first.onMoved: config.viewMinFrequency = Math.max(1, hz(first.value))
                        second.value: mel(config.viewMaxFrequency)
                        second.onMoved: config.viewMaxFrequency = Math.max(1, hz(second.value))
                        Label {
                            anchors.top: parent.first.handle.bottom
                            anchors.topMargin: 5
                            anchors.horizontalCenter: parent.first.handle.horizontalCenter
                            text: config.viewMinFrequency + " Hz"
                        }
                        Label {
                            id: frequencyRangeHandleLabel
                            anchors.top: parent.second.handle.bottom
                            anchors.topMargin: 5
                            anchors.horizontalCenter: parent.second.handle.horizontalCenter
                            text: config.viewMaxFrequency + " Hz"
                        }
                        Layout.bottomMargin: frequencyRangeHandleLabel.height - 10
                    }

                    MenuSeparator {}

                    Label { text: "View frequency scale:" }
                    ComboBox {
                        implicitWidth: parent.width - 10
                        model: [ "Linear", "Logarithmic", "Mel", "ERB" ]
                        currentIndex: config.viewFrequencyScale
                        onActivated: config.viewFrequencyScale = currentIndex
                        Layout.alignment: Qt.AlignHCenter
                    }

                    MenuSeparator {}
                    
                    Label { text: "FFT size" }
                    Slider {
                        from: Math.log2(16)
                        to: Math.log2(16384)
                        stepSize: 1
                        value: Math.log2(config.viewFFTSize)
                        onMoved: config.viewFFTSize = Math.pow(2, Math.round(value))
                        Label {
                            id: fftSizeHandleLabel
                            anchors.top: parent.handle.bottom
                            anchors.topMargin: 5
                            anchors.horizontalCenter: parent.handle.horizontalCenter
                            text: config.viewFFTSize
                        }
                        Layout.bottomMargin: fftSizeHandleLabel.height - 10
                    }

                    MenuSeparator {}

                    Label { text: "Gain normalization:" }
                    Slider {
                        from: -100
                        to: 40
                        value: config.viewMaxGain
                        onMoved: config.viewMaxGain = value
                        Label {
                            id: gainHandleLabel
                            anchors.top: parent.handle.bottom
                            anchors.topMargin: 5
                            anchors.horizontalCenter: parent.handle.horizontalCenter
                            text: config.viewMaxGain + " dB"
                        }
                        Layout.bottomMargin: gainHandleLabel.height - 10
                    }

                    MenuSeparator {}

                    Label { text: "Pitch algorithm:" }
                    ComboBox {
                        implicitWidth: parent.width - 10
                        model: [ "YIN", "McLeod", "RAPT" ]
                        currentIndex: config.pitchAlgorithm
                        onActivated: config.pitchAlgorithm = currentIndex
                        Layout.alignment: Qt.AlignHCenter
                    }
         
                    MenuSeparator {}

                    Label { text: "Formant algorithm:" }
                    ComboBox {
                        implicitWidth: parent.width - 10
                        model: [ "Simple LPC", "Filtered LPC", "DeepFormants" ]
                        currentIndex: config.formantAlgorithm
                        onActivated: config.formantAlgorithm = currentIndex
                        Layout.alignment: Qt.AlignHCenter
                    }

                    MenuSeparator {}

                    Label { text: "LPC algorithm:" }
                    ComboBox {
                        implicitWidth: parent.width - 10
                        model: [ "Autocorrelation", "Covariance", "Burg" ]
                        currentIndex: config.linpredAlgorithm
                        onActivated: config.linpredAlgorithm = currentIndex
                        Layout.alignment: Qt.AlignHCenter
                    }

                    MenuSeparator {}

                    Label { text: "Glottal inverse algorithm:" }
                    ComboBox {
                        implicitWidth: parent.width - 10
                        //model: [ "IAIF", "GFM-IAIF", "AM-GIF" ]
                        model: [ "IAIF", "GFM-IAIF" ]
                        currentIndex: config.invglotAlgorithm
                        onActivated: config.invglotAlgorithm = currentIndex
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }

    IfCanvas {
        id: canvas
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        Behavior on x {
            NumberAnimation {
                easing.type: Easing.InOutQuad
                duration: 200
            }
        }

        Behavior on width {
            NumberAnimation {
                easing.type: Easing.InOutQuad
                duration: 200
            }
        }

        // If in portrait mode, don't push the canvas when drawer is open.
        width: ((mainWindow.height > mainWindow.width) 
                    ? parent.width  
                    : parent.width - drawer.position * sidebar.width)
    }

    Timer {
        repeat: false; running: true; interval: 10
        onTriggered: {
            sidebar.state = "visible"
            if (!config.uiShowSidebar)
                sidebar.state = "hidden"
        }
    }

    property SynthWindow synthWindow

    Component.onCompleted: {
        if (HAS_SYNTH) {
            synthWindow = Qt.createQmlObject('SynthWindow {}', mainWindow);
        }
    }
}

