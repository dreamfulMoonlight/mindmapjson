import QtQuick 2.7
import QtQuick.Window 2.3
import QtQuick.Controls 2.3

Window {
    visible: true
    width: 1000
    height: 600
    title: qsTr("JSON Visualizer")
    property var select_name: ""
    Rectangle{
        id: jsonEditorFrame
        anchors{
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            margins: 20
        }
        width: 300
        border.color: "black"
        ScrollView {
               anchors.fill: parent
            TextArea{
                id: jsonEditor
                text: JSON.stringify($hub.demoJson)
                selectByMouse: true

            }
        }
    }
    Button{
        id:add
         text: qsTr("Add")
         anchors{
             left: jsonEditorFrame.right
             bottom: visualizeButton.top
             margins: 10
         }
         onClicked: {
             $hub.addNode()
             refreshMappint()
         }
    }

    function refreshMappint()
    {
        if(visualizer.rootNode !== null){
            $hub.destroyNode(visualizer.rootNode);
        }
        var jsonObj=jsonEditor.text;
        visualizer.rootNode = $hub.visualize(visualizer);
    }

    Button{
        id: visualizeButton
        text: qsTr("Visualize")
        anchors{
            left: jsonEditorFrame.right
            verticalCenter: parent.verticalCenter
            margins: 10
        }
        onClicked: {
            refreshMappint()
        }
    }

    Button{
        id:minus
         text: qsTr("minus")
         anchors{
             left: jsonEditorFrame.right
             top: visualizeButton.bottom
             margins: 10
         }
         onClicked: {
             $hub.removeNode()
             refreshMappint()
         }
    }

    Button{
        id: testButton
        text: qsTr("test")
        anchors{
            left: jsonEditorFrame.right
             top: minus.bottom
            margins: 10
        }
        onClicked: {
            $hub.testCode(function testlog(this_value)
            {
                console.log("XXXXX11",this_value)
            })
        }
    }
    
    Rectangle{
        id: visualizeFrame
        anchors{
            left: visualizeButton.right
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            margins: 20
        }
        border.color: "black"
        //Item{
        ScrollView{
            id: visualizeScrollView
            anchors.centerIn: parent
            clip: true
            width: Math.min(parent.width, visualizer.width)
            height: Math.min(parent.height, visualizer.height)
            contentWidth: visualizer.width; contentHeight: visualizer.height
            Visualizer{
                id: visualizer
                anchors.centerIn: parent
            }
        }
    }
}
