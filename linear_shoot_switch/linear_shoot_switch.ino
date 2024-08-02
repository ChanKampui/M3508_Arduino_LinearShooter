#include <mcp2515.h>

struct can_frame send;
struct can_frame read;

int rpm[2];
int position[2];
int flag = 0;
int var = 0;

int32_t lastEncoder[2] = { 0 }, deltaPos[2] = { 0 };

int count = 0;       //圈數
int32_t turned = 0;  //pulse

int16_t shoot_current = 16384;
int16_t brake_current = 16384;

MCP2515 mcp2515(10);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  send.can_id = 0x200;
  send.can_dlc = 8;
}

void loop() {
  // put your main code here, to run repeatedly:

  if (mcp2515.readMessage(&read) == MCP2515::ERROR_OK) {
    position[read.can_id - 0x201] = int(word(read.data[0], read.data[1]));
    rpm[read.can_id - 0x201] = int(word(read.data[2], read.data[3]));


    deltaPos[0] = position[0] - lastEncoder[0];
    if (deltaPos[0] > 8192 / 2) {
      deltaPos[0] -= 8192;
      count++;
    } else if (deltaPos[0] < -8192 / 2) {
      deltaPos[0] += 8192;
      count--;
    }

    if (count == 0 && rpm[0] == 0) {
      turned = 0;
    } else {
      turned -= deltaPos[0];
    }
    lastEncoder[0] = position[0];



    if (flag == 0)
      var = 1;

    switch (var) {
      case 1:
        while (turned <= 34500) {
          mcp2515.readMessage(&read);
          position[read.can_id - 0x201] = int(word(read.data[0], read.data[1]));
          rpm[read.can_id - 0x201] = int(word(read.data[2], read.data[3]));

          deltaPos[0] = position[0] - lastEncoder[0];
          if (deltaPos[0] > 8192 / 2) {
            deltaPos[0] -= 8192;
            count++;
          } else if (deltaPos[0] < -8192 / 2) {
            deltaPos[0] += 8192;
            count--;
          }

          if (count == 0 && rpm[0] == 0) {
            turned = 0;
          } else {
            turned -= deltaPos[0];
          }
          lastEncoder[0] = position[0];

          send.data[0] = highByte(-shoot_current);
          send.data[1] = lowByte(-shoot_current);
          send.data[2] = highByte(-shoot_current);
          send.data[3] = lowByte(-shoot_current);
          mcp2515.sendMessage(&send);
        }
        var = 2;
        flag = 1;
        break;

      case 2:

        if (min(rpm[0], rpm[1]) < 0 && var == 2) {
          send.data[0] = highByte(brake_current);
          send.data[1] = lowByte(brake_current);
          send.data[2] = highByte(brake_current);
          send.data[3] = lowByte(brake_current);
          mcp2515.sendMessage(&send);
        } else
          var = 3;
        break;
      case 3:
        if ((min(rpm[0], rpm[1]) > 0) && var == 3) {
          send.data[0] = highByte(500);
          send.data[1] = lowByte(500);
          send.data[2] = highByte(500);
          send.data[3] = lowByte(500);
          mcp2515.sendMessage(&send);
        } else
          var = 4;
        break;
      case 4:
        send.data[0] = highByte(0);
        send.data[1] = lowByte(0);
        send.data[2] = highByte(0);
        send.data[3] = lowByte(0);
        mcp2515.sendMessage(&send);
        //Serial.println("Case 4");
        break;
      default:
        send.data[0] = highByte(0);
        send.data[1] = lowByte(0);
        send.data[2] = highByte(0);
        send.data[3] = lowByte(0);
        mcp2515.sendMessage(&send);
        Serial.println("Stop");
        break;
    }

  }
}
