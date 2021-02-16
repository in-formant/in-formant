import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

ApplicationWindow {
    title: "InFormant - Synthesizer"
    visible: true

    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple

    RowLayout {

        ColumnLayout {
            
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
                Label {
                    anchors.top: parent.handle.bottom
                    anchors.topMargin: 5
                    anchors.horizontalCenter: parent.handle.horizontalCenter
                    text: Math.round(synth.glotPitch) + " Hz"
                }
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
                to: 4.0
                value: synth.filterShift
                onMoved: synth.filterShift = value
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
    }
}
