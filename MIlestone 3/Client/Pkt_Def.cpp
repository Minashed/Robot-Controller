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


PktDef::PktDef(char* DataBuff) {
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
		int dataSize = CP.header.Length - (HEADERSIZE + 1);

		if (CP.header.Length > (HEADERSIZE + 1))
		{
			CP.Data = new char[dataSize];

			for (int i = 0; i < dataSize; i++)
				memcpy(&CP.Data[i], &DataBuff[HEADERSIZE + i], sizeof(char));

		}

		memcpy(&CP.CRC, &DataBuff[8], sizeof(char));
	}
}

//Setters
void PktDef::SetCmd(CmdType ct) {
	
	if (ct >= DRIVE && ct <= ACK)
	{
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
}

void PktDef::SetBodyData(char* motorBody, int size) {
	
	if (size != 0) {
		CP.Data = new char[size];
		memset(CP.Data, 0x00, 2);

		for (int i = 0; i < size; i++) {
			memcpy(&CP.Data[i], &motorBody[i], 1);
		}
	}

	CP.header.Length = HEADERSIZE + size + 1;
}

void PktDef::SetPktCount(int count) {
	if (count >= 0)
		CP.header.PktCount += count;
}

//Getters
CmdType PktDef::GetCmd() {
	if (CP.header.Drive == 1)
		return DRIVE;
	else if (CP.header.Sleep == 1)
		return SLEEP;
	else if (CP.header.Arm == 1)
		return ARM;
	else if (CP.header.Claw == 1)
		return CLAW;
	else
		return NACK;
}

bool PktDef::GetStatus() {
	return CP.header.Status == 1;
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

bool PktDef::CheckCRC(char* DataBuff, int size) {
	this->CalcCRC();

	if (this->CP.CRC == DataBuff[size - 1])
		return true;
	else
		return false;
}

void PktDef::CalcCRC() {

	char* ptr = (char*)&CP.header;
	char count = 0;
	int bodySize = CP.header.Length - (HEADERSIZE + 1);

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

		for (int i = 0; i < bodySize; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				count += (ptr[i] >> j) & 1;
			}
		}
	}

	CP.CRC = count;
}

char* PktDef::GenPacket() {
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
}


/*
std::ostream& operator<<(std::ostream& os, PktDef& src)
{
	bool valid = true;

	char *ptr;
	ptr = src.GenPacket();

	char locBuffer[12];
	memcpy(locBuffer, ptr, 12);

	int calcdCRC = src.GetBitsSet(locBuffer);
	bool statusSet = (bool)((locBuffer[4] >> 1) & 0x01); // status is 7th bit due to reverse endianness

	if ((calcdCRC != (int)locBuffer[11]) || !statusSet)
	{
		os << "Bad Packet. Dropping Processing." << std::endl;
	}
	else
	{
		int driveSet = (int)(locBuffer[10] & 0x01);
		int armUpSet = (int)((locBuffer[10] >> 1) & 0x01);
		int armDownSet = (int)((locBuffer[10] >> 2) & 0x01);
		int clawOpenSet = (int)((locBuffer[10] >> 3) & 0x01);
		int clawClosedSet = (int)((locBuffer[10] >> 4) & 0x01);

		os << "Raw Data:";
		for (int x = 0; x < (int)src.CP.header.Length; x++)
			os << std::hex << std::setw(2) << (unsigned int)*(ptr++) << ", ";
		os << std::endl << std::dec << "CommandID: " << (int)src.GetCmd() << ", CommandType: " << src.GetCmdStr() << std::endl;
		os << calcdCRC << ", " << statusSet << std::endl;
		os << "Sonar: " << int((unsigned char)locBuffer[7] << 8 | (unsigned char)locBuffer[6]) << ", Arm: " << int((unsigned char)locBuffer[9] << 8 | (unsigned char)locBuffer[8]) << std::endl;

		os << std::dec << "Drive flag: " << driveSet << std::endl;
		if (armUpSet && clawOpenSet)
		{
			os << "Arm is Up, Claw is Open" << std::endl;
		}
		if (armDownSet && clawOpenSet)
		{
			os << "Arm is Down, Claw is Open" << std::endl;
		}
		if (armUpSet && clawClosedSet)
		{
			os << "Arm is Up, Claw is Closed" << std::endl;
		}
		if (armDownSet && clawClosedSet)
		{
			os << "Arm is Down, Claw is Closed" << std::endl;
		}
	}

	return os;
}

int PktDef::GetBitsSet(char* pktBuffer)
{
	int counter = 0;
	char temp[12];
	memcpy(temp, pktBuffer, 12);
	char* ptr = temp;

	//Nine because of extra three byted added on CRC
	for (size_t j = 0; j < 11; j++)
	{
		for (size_t i = 0; i < 8; i++)
		{
			counter += *ptr & 1;
			*ptr >>= 1;
		}

		ptr += 1;
	}
	return counter;
}

std::string PktDef::GetCmdStr()
{
	if (CP.header.Drive) {
		return "Drive";
	}
	else if (CP.header.Status)
	{
		return "Status";
	}
	else if (CP.header.Sleep)
	{
		return "Sleep";
	}
	else if (CP.header.Claw)
	{
		return "Claw";
	}
	return "None";
}

*/