import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtCharts 2.15

ApplicationWindow {
    id: synthWindow
    title: appName + " - Oscilloscope"
    visible: false

    width: 400
    height: 300

    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple
    
    RowLayout {
        anchors.fill: parent

        ChartView {
            title: "Oscilloscope"
            antialiasing: true 

            Layout.fillWidth: true
            Layout.fillHeight: true

            theme: ChartView.ChartThemeDark
            backgroundColor: Qt.lighter(Material.background, 0.9)

            axes: [
                ValueAxis {
                    id: oscAxisX
                    labelFormat: "%d ms"
                },
                ValueAxis {
                    id: oscAxisY
                    tickCount: 7
                    labelFormat: "%g"
                }
            ]

            SplineSeries {
                name: "Input sound"
                id: oscSoundSeries
                axisX: oscAxisX
                axisY: oscAxisY
            }

            SplineSeries {
                name: "Estimated glottal flow"
                id: oscGifSeries
                axisX: oscAxisX
                axisY: oscAxisY
            }
        }
    }

    Connections {
        target: dataVis
        function onSoundChanged() {
            dataVis.updateSoundSeries(oscSoundSeries, oscAxisX, oscAxisY)
        }
        function onGifChanged() {
            dataVis.updateGifSeries(oscGifSeries, oscAxisX, oscAxisY)
        }
    }
}
