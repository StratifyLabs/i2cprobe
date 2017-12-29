import QtQuick 2.0
import StratifyLabs.UI 2.1
import QtCharts 2.2
import QtQuick.Dialogs 1.2


SContainer {
  name: "Test";
  style: "block fill";

  function scanI2C(){


    logic.setState("i2cprobe.i2cPort", i2cPort.text);
    logic.setState("i2cprobe.sclPin", sclPin.text);
    logic.setState("i2cprobe.sdaPin", sdaPin.text);
    logic.setState("i2cprobe.slaveAddress", slaveAddress.text);
    logic.setState("i2cprobe.offset", offset.text);
    logic.setState("i2cprobe.value", value.text);

  }

  Connections {
    target: logic;

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
            logic.runApp("uartprobe -p 0");
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

      SButton {
        span: 2;
        style: "btn-outline-secondary right text-semi-bold";
        text: "Clear";
        icon: Fa.Icon.times;
        onClicked: {
          terminalTextBox.textBox.clear();
        }
      }
    }

    SPane {
      style: "block fill";
      implicitHeight: 600;
      SRow {
        style: "block fill";
        Repeater {
          model: 128;
          SLabel {
            span: 1;
            text: "0x" + (index.toString(16)).toUpperCase();
            style: "label-naked block";
          }
        }
      }

    }

    SProgressBar {
      visible: logic.progressMax !== 0;
      value: logic.progressMax ? logic.progress / logic.progressMax : 0;
      style: "block success sm";
    }

  }
}
