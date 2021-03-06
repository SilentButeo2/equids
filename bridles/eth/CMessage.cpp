#include "CMessage.h"

const char* StrMessage[] = {
		"None", // line 41 in CMessage.h
		"Start",
		"Stop", // line 43
		"Reset",
		"Quit",
		"Acknowledgment",
		"Init",
		"Speed set",
		"Move hinge",
		"Pos",
		"Recruiting",
		"Seeding",
		"Docking", // line 53
		"Neighbour's IP REQ",
		"Neighbour's IP",
		"Seed's IP REQ",
		"Seed's IP",
		"AllRobot's IP REQ",
		"AllRobot's IP",
		"Progress REQ",
		"Progress",
		"Disassembly",
		"State REQ", // line 63
		"State",
		"Stop video Stream",
		"Start video Stream",
		"Start detect docking",
		"Start detect mapping",
		"Start detect stair",
		"Detected blob",
		"Detected blob array",
		"Detect stair",
		"Detect stair by laser", // line 73
		"Results of motor calibration",
		"MSG_GET_POSITION",
		"MSG_SET_POSITION",
		"Robot position from Ubisense",
		"Map data",
		"Get all mapped objects",
		"Get nearest object of type",
		"Map covariance",
		"MSG_MAP_COMPLETE",
		"MSG_CALIBRATE", // line 83
		"MSG_ZIGBEE_MSG",
		"MSG_MOVETOPOSITION",
		"MSG_MOVETOPOSITION_DONE",
		"MSG_SOCKET_DOCKING_DONE",
		"MSG_FORCE_CHANGE_JOCKEY",
		"MSG_REMOTE_CONTROL",
		"MSG_INIT_ORGANISM",
		"MSG_LEADER",
		"MSG_DOCK_ORGANISM",
		"Collision detected with wall, step, or robot", // line 93
		"MSG_FIND_DONE",
		"MSG_FIND_FAIL",
		"Which jockeys are active?",
		"Turn on camera all the time",
		"Robot need organism",
		"I can help to create organism",
		"Help accepted",
		"My ZigBee Identity",
		"MSG_NUMBER"
};

CMessage::CMessage()
{
	type = MSG_NONE;
	data = NULL;
	len = 0;
}

CMessage::~CMessage()
{
	/*if (data!=NULL) {
      delete[] data;
      data = NULL;
   }
	 */
}

const char * CMessage::getStrType()
{
	return StrMessage[type];
}

void CMessage::set(const ELolMessage*msg) {
	type = (TMessageType)msg->command;
	len = msg->length;
	if (len>0) {
		data = new uint8_t[len];
		memcpy(data,msg->data,msg->length);
	} else {
		data = NULL;
	}
}

void CMessage::set(const CMessage *msg) {
	type = (TMessageType)msg->type;
	len = msg->len;
	if (len>0) {
		data = new uint8_t[len];
		memcpy(data,msg->data,msg->len);
	} else {
		data = NULL;
	}
}

CMessage CMessage::packToZBMessage(uint64_t ubitag, int type, void *data,
		int len) {
	//printf("packing to ZB message \n");
	CMessage message;
	message.len = sizeof(uint64_t) + sizeof(int) + len;
	message.data = new uint8_t[message.len];
	message.type = MSG_ZIGBEE_MSG;
	memcpy(message.data, &ubitag, sizeof(uint64_t));
	memcpy(message.data + sizeof(uint64_t), &type, sizeof(int));
	memcpy(message.data + sizeof(uint64_t) + sizeof(int), data, len);
	return message;
}

CMessage CMessage::unpackZBMessage(CMessage ZBmessage) {
	CMessage message;
	message.len = ZBmessage.len - sizeof(uint64_t) - sizeof(int);
	//printf("whole zigbee size is : %d \n", message.len);
	if (message.len > 0) {
		message.data = new uint8_t[message.len];
		memcpy(message.data, ZBmessage.data + sizeof(uint64_t) + sizeof(int),
				message.len);
	} else {
		message.data = NULL;
	}
	memcpy(&message.type, ZBmessage.data + sizeof(uint64_t), sizeof(int));
	//printf("type is : %d \n", message.type);
	return message;
}
