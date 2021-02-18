import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtCharts 2.15

ApplicationWindow {
    id: synthWindow
    title: "InFormant - Synthesizer"
    visible: true

    width: 400
    height: 300

    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple

    Flickable {
        id: container
        width: synthWindow.width
        height: synthWindow.height
        contentWidth: contents.width
        contentHeight: contents.height

        ScrollBar.vertical: ScrollBar {}
        
        RowLayout {
            id: contents
            width: Math.max(implicitWidth, synthWindow.width)
            height: Math.max(implicitHeight, synthWindow.height)

            ColumnLayout {
                Layout.margins: 10

                Switch {
                    text: "Turn on/off"
                    checked: synth.enabled
                    onToggled: synth.enabled = checked
                }

                MenuSeparator {}

                Label { text: "Aspirate noise volume" }
                Slider {
                    value: synth.noiseGain
                    onMoved: synth.noiseGain = value
                    Layout.topMargin: -10
                    Label {
                        anchors.top: parent.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.handle.horizontalCenter
                        text: Math.round(100 * synth.noiseGain) + "%"
                    }

                }

                MenuSeparator {}

                Label { text: "Glottal source volume" }
                Slider {
                    value: synth.glotGain
                    onMoved: synth.glotGain = value
                    Layout.topMargin: -10
                    Label {
                        anchors.top: parent.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.handle.horizontalCenter
                        text: Math.round(100 * synth.glotGain) + "%"
                    }

                }

                MenuSeparator {}

                Label { text: "Glottal source pitch" }
                Slider {
                    from: mel(60)
                    to: mel(8000)
                    value: mel(synth.glotPitch)
                    onMoved: synth.glotPitch = hz(value)
                    Layout.topMargin: -10
                    Label {
                        anchors.top: parent.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.handle.horizontalCenter
                        text: Math.round(synth.glotPitch) + " Hz"
                    }
                }

                CheckBox {
                    font.italic: true
                    text: "Follow pitch estimates"
                    checked: synth.followPitch
                    onToggled: synth.followPitch = checked
                }

                MenuSeparator {}

                Label {
                    textFormat: Text.RichText
                    text: "Glottal source <var>R<sub>d</sub></var>"
                }
                Slider {
                    from: 0.7
                    to: 2.6
                    value: synth.glotRd
                    onMoved: synth.glotRd = value
                    Layout.topMargin: -10
                    Label {
                        anchors.top: parent.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.handle.horizontalCenter
                        text: +synth.glotRd.toFixed(2)
                    }
                }

                MenuSeparator {}
                
                Label {
                    textFormat: Text.RichText
                    text: "Glottal source <var>T<sub>c</sub></var>"
                }
                Slider {
                    from: 0.9
                    to: 1.0
                    value: synth.glotTc
                    onMoved: synth.glotTc = value
                    Layout.topMargin: -10
                    Label {
                        anchors.top: parent.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.handle.horizontalCenter
                        text: +synth.glotTc.toFixed(2)
                    }
                }

                MenuSeparator {}

                Label { text: "Vocal tract filter shift" }
                Slider {
                    from: 0.2
                    to: 2.0
                    value: synth.filterShift
                    onMoved: synth.filterShift = value
                    Layout.topMargin: -10
                    Label {
                        anchors.top: parent.handle.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.handle.horizontalCenter
                        text: Math.round(100 * synth.filterShift) + "%"
                    }
                }

                MenuSeparator {}

                Switch {
                    text: "Voicing"
                    checked: synth.voiced
                    onToggled: synth.voiced = checked
                }
            }

            ToolSeparator {
                id: verticalSep
                Layout.fillHeight: true
                Layout.topMargin: 10
                Layout.bottomMargin: 10
            }

            ColumnLayout {
                Layout.margins: 10

                CheckBox {
                    font.italic: true
                    text: "Follow formant estimates"
                    checked: synth.followFormants
                    onToggled: synth.followFormants = checked
                }

                ChartView {
                    title: "Synthesis filter"
                    antialiasing: true
                    
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    theme: ChartView.ChartThemeDark
                    backgroundColor: Qt.lighter(Material.background, 0.9)
                    legend.visible: false

                    axes: [
                        LogValueAxis {
                            id: filterSpectrumAxisX
                            minorTickCount: 10
                            labelFormat: "%d Hz"
                        },
                        ValueAxis {
                            id: filterSpectrumAxisY
                            minorTickCount: 5
                            labelFormat: "%d dB"
                        }
                    ]

                    SplineSeries {
                        id: filterSpectrumSeries
                        axisXTop: filterSpectrumAxisX
                        axisY: filterSpectrumAxisY
                    }
                }

                MenuSeparator {}

                RowLayout {
                    ChartView {
                        title: "Synthesized source"
                        antialiasing: true 

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        theme: ChartView.ChartThemeDark
                        backgroundColor: Qt.lighter(Material.background, 0.9)
                        legend.visible: false

                        axes: [
                            ValueAxis {
                                id: sourceAxisX
                                labelFormat: "%d ms"
                            },
                            ValueAxis {
                                id: sourceAxisY
                                tickCount: 7
                                labelFormat: "%g"
                            }
                        ]

                        LineSeries {
                            id: sourceSeries
                            axisX: sourceAxisX
                            axisY: sourceAxisY
                        }
                    }

                    ChartView {
                        title: "Synthesized source spectrum"
                        antialiasing: true 
                        
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        theme: ChartView.ChartThemeDark
                        backgroundColor: Qt.lighter(Material.background, 0.9)
                        legend.visible: false
                        
                        axes: [
                            LogValueAxis {
                                id: sourceSpectrumAxisX
                                minorTickCount: 10
                                labelFormat: "%d Hz"
                            },
                            ValueAxis {
                                id: sourceSpectrumAxisY
                                minorTickCount: 5
                                labelFormat: "%d dB"
                            }
                        ]

                        SplineSeries {
                            id: sourceSpectrumSeries
                            axisXTop: sourceSpectrumAxisX
                            axisY: sourceSpectrumAxisY
                        }
                    }
                }

                Connections {
                    target: synth
                    function onFilterResponseChanged() {
                        synth.updateFilterResponseSeries(filterSpectrumSeries, filterSpectrumAxisX, filterSpectrumAxisY)
                    }
                    function onSourceChanged() {
                        synth.updateSourceSeries(sourceSeries, sourceAxisX, sourceAxisY)
                    }
                    function onSourceSpectrumChanged() {
                        synth.updateSourceSpectrumSeries(sourceSpectrumSeries, sourceSpectrumAxisX, sourceSpectrumAxisY);
                    }
                }
            }
        }
    }
}
