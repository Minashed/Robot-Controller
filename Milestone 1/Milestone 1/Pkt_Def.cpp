#include "Pkt_Def.h"

//Constructors
PktDef::PktDef() {
	//All Header information set to zero
	CP.header.PktCount = 0;
	CP.header.Drive = 0;
	CP.header.Status = 0;
	CP.header.Sleep = 0;
	CP.header.Arm = 0;
	CP.header.Claw = 0;
	CP.header.Ack = 0;

	CP.header.Padding = 0;

	CP.header.Length = 0;

	//Data pointer set to nullptr
	CP.Data = nullptr;

	//CRC set to zero
	CP.CRC = 0;

	RawBuffer = nullptr;
}


PktDef::PktDef(char * DataBuff) {
	/*
	if (DataBuff != nullptr)
	{
		memcpy(&CP.header.PktCount, DataBuff, sizeof(int));

		CP.header.Drive = (DataBuff[4] & 1);
		CP.header.Status = ((DataBuff[4] >> 1) & 1);
		CP.header.Sleep = ((DataBuff[4] >> 2) & 1);
		CP.header.Arm = ((DataBuff[4] >> 3) & 1);
		CP.header.Claw = ((DataBuff[4] >> 4) & 1);
		CP.header.Ack = ((DataBuff[4] >> 5) & 1);
		CP.header.Padding = 0;

		memcpy(&CP.header.Length, &DataBuff[1 + sizeof(int)], sizeof(char));

		if (CP.header.Length > (HEADERSIZE + 1))
		{
			CP.Data = new char[sizeof(MotorBody)];

			memcpy(&CP.Data[0], &DataBuff[6], sizeof(char));
			memcpy(&CP.Data[1], &DataBuff[7], sizeof(char));
		}

		memcpy(&CP.CRC, &DataBuff[8], sizeof(char));
	}
	*/
	
	char * ptr = DataBuff;

	//Copy PktCount over
	memcpy(&CP.header.PktCount, ptr, sizeof(int));

	ptr += sizeof(int);

	//Copy Command Flags over
	CP.header.Drive = (*ptr) & 1;
	CP.header.Status = (*ptr >> 1) & 1;
	CP.header.Sleep = (*ptr >> 2) & 1;
	CP.header.Arm = (*ptr >> 3) & 1;
	CP.header.Claw = (*ptr >> 4) & 1;
	CP.header.Ack = (*ptr >> 5) & 1;
	CP.header.Padding = 0;


	ptr += sizeof(unsigned char);

	memcpy(&CP.header.Length, ptr, sizeof(char));

	//Copies over data if required
	if (CP.header.Length > (HEADERSIZE + 1)) {

		CP.Data = new char[sizeof(MotorBody)];

		memcpy(&CP.Data[0], ptr, sizeof(char));

		ptr += 1;

		memcpy(&CP.Data[1], ptr, sizeof(char));
	}

	//Copies over crc
	memcpy(&CP.CRC, ptr, sizeof(char));
	
}

//Setters
void PktDef::SetCmd(CmdType ct) {
	
	if (ct != ACK) {
		CP.header.Drive = 0;
		CP.header.Ack = 0;
		CP.header.Status = 0;
		CP.header.Sleep = 0;
		CP.header.Arm = 0;
		CP.header.Claw = 0;
	}

	if (ct == DRIVE) {
		CP.header.Drive = 1;
	}
	else if (ct == SLEEP) {
		CP.header.Sleep = 1;
	}
	else if (ct == ARM) {
		CP.header.Arm = 1;
	}
	else if (ct == CLAW) {
		CP.header.Claw = 1;
	}
	else if (ct == ACK) {
		CP.header.Ack = 1;
	}
}

void PktDef::SetBodyData(char * motorBody, int size) {
	CP.Data = new char[size];

	memset(&CP.Data,0x00, 2);

	for (int i = 0; i < size; i++) {
		memcpy(&CP.Data[i], &motorBody[i], 1);
	}

	CP.header.Length = HEADERSIZE + size;
}

void PktDef::SetPktCount(int count) {
	if (count >= 0)
		CP.header.PktCount = count;
}

//Getters
CmdType PktDef::GetCmd() {
	if (CP.header.Drive == 1) {
		return DRIVE;
	}
	else if (CP.header.Ack == 1) {
		return ACK;
	}
	else if (CP.header.Sleep == 1) {
		return SLEEP;
	}
	else if (CP.header.Arm == 1) {
		return ARM;
	}
	else if (CP.header.Claw == 1) {
		return CLAW;
	}
	else if (CP.header.Status == 1){
		return STATUS;
	}
	
}

bool PktDef::GetAck() {

	if (CP.header.Ack) 
		return true;
	else 
		return false;

}

int PktDef::GetLength() { return (int)CP.header.Length; }

char* PktDef::GetBodyData() { return CP.Data; }

int PktDef::GetPktCount() { return CP.header.PktCount; }

bool PktDef::CheckCRC(char * DataBuff, int size) {
	this->CalcCRC();
	return this->CP.CRC == DataBuff[size - 1];

}

void PktDef::CalcCRC() {
	char* ptr = (char*)&CP.header;

	char count = 0;

	for (int i = 0; i < HEADERSIZE; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			count += (ptr[i] >> j) & 1;
		}
	}

	if (CP.header.Length > 7)
	{
		ptr = (char*)CP.Data;

		for (int i = 0; i < sizeof(MotorBody); i++)
		{
			for (int j = 0; j < 8; j++)
			{
				count += (ptr[i] >> j) & 1;
			}
		}
	}

	CP.CRC = count;
}

char * PktDef::GenPacket() {
	int size = this->CP.header.Length;

	if (size <= 7) {
		RawBuffer = new char[HEADERSIZE + 1];
		memset(RawBuffer, 0x00, HEADERSIZE + 1);
		memcpy(RawBuffer, &CP.header, HEADERSIZE);
		RawBuffer[HEADERSIZE] = CP.CRC;
	}
	else
	{
		RawBuffer = new char[size];
		memset(RawBuffer, 0x00, size);
		memcpy(RawBuffer, &CP.header, HEADERSIZE);

		for (int i = 0; i < sizeof(MotorBody); i++)
			RawBuffer[i + HEADERSIZE] = CP.Data[i];

		RawBuffer[size - 1] = CP.CRC;
	}

	return RawBuffer;
	
	/*
	if (CP.header.Length == 0) {
		RawBuffer = new char[HEADERSIZE + 1];
	}else{
		RawBuffer = new char[CP.header.Length];
	}

	memcpy(RawBuffer, &CP.header.PktCount, HEADERSIZE);

	for (int i = 0; i < sizeof(MotorBody); i++) {
		RawBuffer[HEADERSIZE + i] = CP.Data[i];
	}
	for (int i = 0; i < size; i++) {
		memcpy(&CP.Data[i], &motorBody[i], 1);
	}

	memcpy(&RawBuffer[9], &CP.CRC, sizeof(char));

	return RawBuffer;
	*/
}