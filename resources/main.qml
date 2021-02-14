import QtQuick 2.15
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import IfCanvas 1.0

ApplicationWindow {
    visible: true

    minimumWidth: 600
    minimumHeight: 400

    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple

    header: ToolBar {
        RowLayout {
            Switch {
                text: "Show sidebar"
                checked: config.uiShowSidebar
                onToggled: config.uiShowSidebar = checked
            }

            ToolSeparator {}
        }
    }

    RowLayout {
        anchors.fill: parent

        ScrollView {
            id: sidebar

            Layout.margins: 10 

            states: [
                State {
                    name: "visible"; when: config.uiShowSidebar
                    PropertyChanges { target: sidebar; x: 10; opacity: 1 }
                    PropertyChanges { target: canvas; x: sidebar.width + 20; width: parent.width - sidebar.width }
                },
                State {
                    name: "hidden"; when: !config.uiShowSidebar
                    PropertyChanges { target: sidebar; x: -sidebar.width; opacity: 0 }
                    PropertyChanges { target: canvas; x: 0; width: parent.width  }
                }
            ]

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

            ColumnLayout {
                
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

                Label {
                    text: "View frequency range:"
                }
                RangeSlider {
                    function mel(f) {
                        return 2595 * Math.log10(1 + f / 700);
                    }
                    function hz(m) {
                        return 700 * (Math.pow(10, m / 2595) - 1);
                    }

                    id: viewFrequency
                    from: mel(1)
                    to: mel(16000)
                    first.value: mel(config.viewMinFrequency)
                    first.onMoved: config.viewMinFrequency = hz(first.value)
                    second.value: mel(config.viewMaxFrequency)
                    second.onMoved: config.viewMaxFrequency = hz(second.value)
                    Label {
                        anchors.top: parent.first.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.first.handle.horizontalCenter
                        text: config.viewMinFrequency + " Hz"
                    }
                    Label {
                        id: handleLabel
                        anchors.top: parent.second.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.second.handle.horizontalCenter
                        text: config.viewMaxFrequency + " Hz"
                    }
                    Layout.bottomMargin: handleLabel.height - 10
                }

                MenuSeparator {}

                Label {
                    text: "Pitch algorithm:"
                }
                ComboBox {
                    implicitWidth: parent.width - 10
                    model: [ "YIN", "McLeod", "RAPT" ]
                    currentIndex: config.pitchAlgorithm
                    onActivated: config.pitchAlgorithm = currentIndex
                    Layout.alignment: Qt.AlignHCenter
                }
     
                MenuSeparator {}

                Label {
                    text: "Formant algorithm:"
                }
                ComboBox {
                    implicitWidth: parent.width - 10
                    model: [ "Simple LPC", "Filtered LPC", "DeepFormants" ]
                    currentIndex: config.formantAlgorithm
                    onActivated: config.formantAlgorithm = currentIndex
                    Layout.alignment: Qt.AlignHCenter
                }

                MenuSeparator {}

                Label {
                    text: "LPC algorithm:"
                }
                ComboBox {
                    implicitWidth: parent.width - 10
                    model: [ "Autocorrelation", "Covariance", "Burg" ]
                    currentIndex: config.linpredAlgorithm
                    onActivated: config.linpredAlgorithm = currentIndex
                    Layout.alignment: Qt.AlignHCenter
                }

                MenuSeparator {}

                Label {
                    text: "Glottal inverse algorithm:"
                }
                ComboBox {
                    implicitWidth: parent.width - 10
                    model: [ "IAIF", "GFM-IAIF", "AM-GIF" ]
                    currentIndex: config.invglotAlgorithm
                    onActivated: config.invglotAlgorithm = currentIndex
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        IfCanvas {
            id: canvas
            Layout.fillWidth: true
            Layout.fillHeight: true

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
        }
    }

    Timer {
        repeat: false; running: true; interval: 10
        onTriggered: {
            sidebar.state = "visible"
            if (!config.uiShowSidebar)
                sidebar.state = "hidden"
        }
    }
}
