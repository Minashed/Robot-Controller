#ifndef PKTDEF_H
#define PKTDEF_H

#include <iostream>
#include <iomanip>
#include <stdio.h>



//Globally defined variables
#define FORWARD 1
#define BACKWARD 2
#define LEFT 3
#define RIGHT 4
#define UP 5
#define DOWN 6
#define OPEN 7
#define CLOSE 8

//Size of the entire header varies in size
#define HEADERSIZE 6


//Enum for flags
enum CmdType { DRIVE = 1, STATUS, SLEEP, ARM, CLAW, ACK };
//                  1       2       3     4    5     6


//Header Struct
struct Header {
	unsigned int PktCount; // 4Bytes 0

	unsigned char Drive : 1;
	unsigned char Status : 1;
	unsigned char Sleep : 1;
	unsigned char Arm : 1;
	unsigned char Claw : 1;
	unsigned char Ack : 1; 

	unsigned char Padding : 2; // 5Bytes 5

	unsigned char Length; //6 Bytes
};

//Body Struct
struct MotorBody {
	unsigned char Direction;
	unsigned char Duration;
};

//Telemery Body
struct TelBody {
	unsigned short int Sonar;
	unsigned short int Arm;

	unsigned char DriveF : 1;
	unsigned char ArmUp : 1;
	unsigned char ArmDown : 1;
	unsigned char ClawOpen : 1;
	unsigned char ClawClosed : 1;

	unsigned char Padding : 3;
};

//MAIN CLASS
class PktDef {
private:
	struct CmdPacket 
	{
		Header header; // 6 Bytes 
		char* Data; // 1or2 Bytes?
		char CRC;// 1 Bytes
	} CP; // 8 Bytes 9 Bytes?

	char* RawBuffer;

public:
	//Constructors
	PktDef();
	PktDef(char*);

	//Setters
	void SetCmd(CmdType);
	void SetBodyData(char*, int);
	void SetPktCount(int);

	//Getters
	CmdType GetCmd();
	bool GetAck();
	int GetLength();
	char* GetBodyData();
	int GetPktCount();
	bool GetStatus();

	//a function that takes a pointer to a RAW data buffer, the
	//size of the buffer in bytes, and calculates the CRC.If the calculated CRC matches the
	//CRC of the packet in the buffer the function returns TRUE, otherwise FALSE
	bool CheckCRC(char*, int);

	//a function that calculates the CRC and sets the objects packet CRC parameter.
	void CalcCRC();

	//a function that allocates the private RawBuffer and transfers the
	//contents from the objects member variables into a RAW data packet(RawBuffer) for
	//transmission.The address of the allocated RawBuffer is returned.
	char* GenPacket();
};
#endif