import QtQuick 2.0
import StratifyLabs.UI 2.2
import QtCharts 2.2
import QtQuick.Dialogs 1.2


SContainer {
  id: i2cprobe;
  name: "Test";
  style: "block fill";

  property real addresses: 4;
  property var scanData: ({});
  property string scanDataJson;

  function saveOptions(){
    logic.setState("i2cprobe.i2cPort", i2cPort.text);
    logic.setState("i2cprobe.sclPin", sclPin.text);
    logic.setState("i2cprobe.sdaPin", sdaPin.text);
    logic.setState("i2cprobe.slaveAddress", slaveAddress.text);
    logic.setState("i2cprobe.offset", offset.text);
    logic.setState("i2cprobe.value", value.text);
  }

  function scanI2C(){
    saveOptions();
    scanData = {};
    scanData.data = [];

    var args = "-i2c " + i2cPort.text +
        (sclPin.text !== "" ? (" -scl " + sclPin.text) : "") +
        (sdaPin.text !== "" ? (" -sda " + sdaPin.text) : "") +
        (slaveAddress.text !== "" ? (" -sda " + sdaPin.text) : "") +
        " -freq 100000 -pu" +
        " -scan -message 1.0";

    logic.appRun("i2cprobe " + args);
  }

  Connections {
    target: logic;

    onDeviceMessageChanged: {
      if( message.type === "address" ){
        i2cprobe.scanData.data.push(message);
        if( message.address === "0x7F" ){
          console.log("JSON " + logic.stringify(scanData));
          scanDataJson = logic.stringify(scanData);
        }
      }
    }
  }

  Component.onCompleted: {
    i2cPort.text = logic.getState("i2cprobe.i2cPort");
    sclPin.text = logic.getState("i2cprobe.sclPin");
    sdaPin.text = logic.getState("i2cprobe.sdaPin");
    slaveAddress.text = logic.getState("i2cprobe.slaveAddress");
    offset.text = logic.getState("i2cprobe.offset");
    value.text = logic.getState("i2cprobe.value");
  }

  SColumn {
    style: "block";
    SRow {
      SLabel {
        attr.paddingHorizontal: 0;
        style: "text-h1 left";
        text: "I2C Probe"
      }
    }

    SRow {
      SLabel {
        style: "btn-primary left";
        text: "Actions";
        attr.paddingHorizontal: 0;
        span: 2;
      }

      SGroup {
        span: 2;
        style: "right";

        SButton {
          icon: Fa.Icon.search;
          style: "btn-primary text-semi-bold";
          onClicked: {
            scanI2C();
          }

          SToolTip {
            text: "Scan I2C";
          }

        }

        SButton {
          icon: Fa.Icon.download;
          style: "btn-primary text-semi-bold";
          onClicked: {

          }
          SToolTip {
            text: "Write Device";
          }
        }

        SButton {
          icon: Fa.Icon.upload;
          style: "btn-primary text-semi-bold";
          onClicked: {

          }
          SToolTip {
            text: "Read Device";
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

    SHLine{}

    SRow {
      SLabel {
        style: "btn-primary left";
        text: "Options";
        attr.paddingHorizontal: 0;
        span: 2;
      }

      SGroup {
        span: 2;
        style: "right";

        SButton {
          id: showButton;
          icon: options.visible ? Fa.Icon.minus: Fa.Icon.plus;
          style: "btn-outline-secondary text-semi-bold";
          onClicked: {
            options.visible = !options.visible;
          }
        }
      }
    }

    SContainer {
      id: options;
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
          text: "Address";
        }
        SInput {
          id: slaveAddress;
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
          id: offset;
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
          id: value;
          span:2;
          style: "right text-center";
          placeholder: "0xXX";
        }
      }
    }

    SHLine{}

    SRow {
      SLabel {
        span: 2;
        style: "left";
        text: "I2C Bus Scan";
        attr.paddingHorizontal: 0;
      }
    }

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
      id: terminalText;
      style: "block fill";
      implicitHeight: 200;
      Connections {
        target: logic;
        onStdioChanged: {
          terminalText.textBox.insert(terminalText.textBox.length, value);
          terminalText.textBox.cursorPosition = terminalText.textBox.text.length;
        }
      }
    }

    SProgressBar {
      visible: logic.progressMax !== 0;
      value: logic.progressMax ? logic.progress / logic.progressMax : 0;
      style: "block success sm bottom";
    }

  }
}
