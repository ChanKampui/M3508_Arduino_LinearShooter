#include <mcp2515.h>

struct can_frame send;
struct can_frame read;
uint8_t BUTTON_PIN = 7;

int rpm[2];
int position[2];
int flag = 0;

int32_t lastEncoder[2] = { 0 }, deltaPos[2] = { 0 };
int count = 0;

int32_t turned = 0;
int16_t shoot_current = 16384;
int16_t brake_current = 16384;

MCP2515 mcp2515(10);


void setup() {
  Serial.begin(115200);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  send.can_id = 0x200;
  send.can_dlc = 8;
}

void loop() {

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


    if (flag == 0 && turned < 35000) {
      //shoot
      send.data[0] = highByte(-shoot_current);
      send.data[1] = lowByte(-shoot_current);
      send.data[2] = highByte(-shoot_current);
      send.data[3] = lowByte(-shoot_current);
      mcp2515.sendMessage(&send);

    } else if (min(rpm[0], rpm[1]) < 0 && flag != 2) {
      //brake
      flag = 1;
      send.data[0] = highByte(brake_current);
      send.data[1] = lowByte(brake_current);
      send.data[2] = highByte(brake_current);
      send.data[3] = lowByte(brake_current);
      mcp2515.sendMessage(&send);

    } else if ((min(rpm[0], rpm[1]) > 0) && flag != 2) {
      //back
      send.data[0] = highByte(450);
      send.data[1] = lowByte(450);
      send.data[2] = highByte(450);
      send.data[3] = lowByte(450);
      mcp2515.sendMessage(&send);

    } else if (count == 0) {
      //stop
      flag = 2;
      send.data[0] = highByte(0);
      send.data[1] = lowByte(0);
      send.data[2] = highByte(0);
      send.data[3] = lowByte(0);
      mcp2515.sendMessage(&send);
    }
  }
}