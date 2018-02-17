import QtQuick 2.0
import StratifyLabs.UI 2.2
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.1



SContainer {
  id: i2cprobe;
  style: "block fill";

  property real addresses: 4;
  property var scanData: ({});
  property string scanDataJson;
  property real count;

  function saveOptions(){
    logic.setState("i2cprobe.i2cPort", i2cPort.text);
    logic.setState("i2cprobe.sclPin", sclPin.text);
    logic.setState("i2cprobe.sdaPin", sdaPin.text);
    logic.setState("i2cprobe.writeAddress", writeAddress.text);
    logic.setState("i2cprobe.writeOffset", writeOffset.text);
    logic.setState("i2cprobe.writeValue", writeValue.text);
    logic.setState("i2cprobe.readAddress", readAddress.text);
    logic.setState("i2cprobe.readOffset", readOffset.text);
    logic.setState("i2cprobe.readBytes", readBytes.text);
  }

  function readI2C(){
    saveOptions();
    scanData = {};
    scanDataJson = "";
    count = 0;

    var args = "-i2c " + i2cPort.text +
        (sclPin.text !== "" ? (" -scl " + sclPin.text) : "") +
        (sdaPin.text !== "" ? (" -sda " + sdaPin.text) : "") +
        (bitrate.text !== "" ? (" -freq " + bitrate.text) : "") +
        (pullup.checked === true ? (" -pu") : "") +
        (readBytes.text !== "" ? (" -n " + readBytes.text) : "-n 1") +
        (readAddress.text !== "" ? (" -slave " + readAddress.text) : "-slave 0") +
        (readOffset.text !== "" ? (" -o " + readOffset.text) : "-o 0") +

        " -r  -message 255.0";

    logic.appRun("i2cprobe " + args);
    terminalTextBox.visible = true;
    scrollingPane.updateContentRect();
  }

  function scanI2C(){
    saveOptions();
    scanData = {};
    scanData.data = [];
    count = 0;
    terminalTextBox.visible = false;

    var args = "-i2c " + i2cPort.text +
        (sclPin.text !== "" ? (" -scl " + sclPin.text) : "") +
        (sdaPin.text !== "" ? (" -sda " + sdaPin.text) : "") +
        (bitrate.text !== "" ? (" -freq " + bitrate.text) : "") +
        (pullup.checked === true ? (" -pu") : "") +
        " -scan -message 255.0";

    logic.appRun("i2cprobe " + args);


  }

  Connections {
    target: logic;

    onDeviceMessageChanged: {
      if( message.type === "address" ){
        i2cprobe.scanData.data.push(message);
        logic.progress = count++;
        logic.progressMax = 127;
        logic.progressChanged();
        if( message.address === "0x7F" ){
          console.log("JSON " + logic.stringify(scanData));
          scanDataJson = logic.stringify(scanData);
          logic.progress = 0;
          logic.progressMax = 0;
          logic.progressChanged();
          scrollingPane.updateContentRect();
        }
      }
    }
  }

  Component.onCompleted: {
    i2cPort.text = logic.getState("i2cprobe.i2cPort");
    sclPin.text = logic.getState("i2cprobe.sclPin");
    sdaPin.text = logic.getState("i2cprobe.sdaPin");
    writeAddress.text = logic.getState("i2cprobe.writeAddress");
    writeOffset.text = logic.getState("i2cprobe.writeOffset");
    writeValue.text = logic.getState("i2cprobe.writeValue");
    readAddress.text = logic.getState("i2cprobe.readAddress");
    readOffset.text = logic.getState("i2cprobe.readOffset");
    readBytes.text = logic.getState("i2cprobe.readBytes");
  }

  SPane {
    id: scrollingPane;
    clip: false;

    SColumn {
      style: "block";

      SRow {
        SLabel {
          attr.paddingHorizontal: 0;
          style: "text-h1 left";
          text: "I2C Probe"
        }
      }

      SProgressBar {
        visible: logic.progressMax !== 0;
        value: logic.progressMax ? logic.progress / logic.progressMax : 0;
        style: "block success sm bottom";
      }

      SRow {
        SLabel {
          style: "btn-primary left";
          text: "I2C Settings";
          attr.paddingHorizontal: 0;
          span: 2;
        }

        SGroup {
          span: 2;
          style: "right";

          SButton {
            icon: Fa.Icon.search;
            style: "btn-primary text-semi-bold";
            text: "Scan";
            onClicked: {
              scanI2C();
            }

            SToolTip {
              text: "Scan I2C";
            }
          }

          SButton {
            style: "btn-primary";
            label: "Install";
            onClicked: {
              logic.appInstall("/Users/tgil/git/StratifyApps/i2cprobe");
            }
          }
        }
      }

      SRow {
        SLabel {
          span: 2;
          style: "left";
          text: "I2C";
        }
        SInput {
          id: i2cPort;
          span:2;
          style: "right text-center";
          placeholder: "X";
        }
        SLabel {
          span: 2;
          style: "left";
          text: "SCL";
        }
        SInput {
          id: sclPin;
          span:2;
          style: "right text-center";
          placeholder: "X.Y";
        }
        SLabel {
          span: 2;
          style: "left";
          text: "SDA";
        }
        SInput {
          id: sdaPin;
          span:2;
          style: "right text-center";
          placeholder: "X.Y";
        }
        SLabel {
          span: 2;
          style: "left";
          text: "Bitrate";
        }
        SInput {
          id: bitrate;
          span:2;
          style: "right text-center";
          placeholder: "100000";
        }

        SCheckbox {
          id: pullup;
          span:4;
          text: "Internal Pullup";
          style: "right text-center";
        }
      }

      SHLine{}

      SRow {
        SButton {
          span: 2;
          icon: Fa.Icon.download;
          text: "Write";
          style: "btn-primary text-semi-bold left";
          onClicked: {
          }
          SToolTip {
            text: "Write Device";
          }
        }
      }

      SContainer {
        id: writeOptions;
        SRow {
          SLabel {
            span: 2;
            style: "left";
            text: "Address";
          }
          SInput {
            id: writeAddress;
            span:2;
            style: "right text-center";
            placeholder: "0xXX";
          }
          SLabel {
            span: 2;
            style: "left";
            text: "Offset";
          }
          SInput {
            id: writeOffset;
            span:2;
            style: "right text-center";
            placeholder: "0xXX";
          }
          SLabel {
            span: 2;
            style: "left";
            text: "Value";
          }
          SInput {
            id: writeValue;
            span:2;
            style: "right text-center";
            placeholder: "0xXX";
          }
        }
      }

      SHLine{}

      SRow {
        SGroup {
          style: "left";
          SButton {
            span: 2;
            icon: Fa.Icon.upload;
            text: "Read";
            style: "btn-primary text-semi-bold left";
            onClicked: {
              readI2C();
            }
            SToolTip {
              text: "Read Device";
            }
          }
          SButton {
            span: 2;
            icon: Fa.Icon.times;
            text: "Clear";
            style: "btn-outline-secondary text-semi-bold left";
            onClicked: {
              terminalTextBox.textBox.clear();
            }
            SToolTip {
              text: "Clear Output";
            }
          }
        }
      }

      SContainer {
        id: readOptions;
        SRow {
          SLabel {
            span: 2;
            style: "left";
            text: "Address";
          }
          SInput {
            id: readAddress;
            span:2;
            style: "right text-center";
            placeholder: "0xXX";
          }
          SLabel {
            span: 2;
            style: "left";
            text: "Offset";
          }
          SInput {
            id: readOffset;
            span:2;
            style: "right text-center";
            placeholder: "0xXX";
          }
          SLabel {
            span: 2;
            style: "left";
            text: "Bytes";
          }
          SInput {
            id: readBytes;
            span:2;
            style: "right text-center";
            placeholder: "0xXX";
          }
        }
      }

      SHLine{}

      SRow {
        style: "block fill";

        SRepeater {
          id: repeater;
          model: SJsonModel {
            json: scanDataJson;
          }
          SLabel {
            span: 1;
            text: model.address;
            style: model.value === true  ? "label-primary block text-center" : "label-naked block text-center";
          }
        }
      }

      STextBox {
        id: terminalTextBox;
        style: "fill";
        attr.textFont: STheme.font_family_monospace.name;
        textBox.readOnly: true;
        implicitHeight: 250;
        visible: false;

        Connections {
          target: logic;
          onStdioChanged: {
            terminalTextBox.textBox.insert(terminalTextBox.textBox.length, value);
            terminalTextBox.textBox.cursorPosition = terminalTextBox.textBox.text.length;
          }
        }
      }
    }
  }
}
