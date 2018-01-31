#include <iostream>
#include <sstream>
#include <thread>

#include "MySocket.h"
#include "Pkt_Def.h"

bool exeComplete = false;

void CommandThread(unsigned int port, std::string ipAddr) {
	//Create a MySocket object configured as a SocketType::CLIENT and ConnectionType::TCP
	MySocket CmdSocket(CLIENT, ipAddr, port, TCP, 100);

	//Perform the 3-way handshake to establish a reliable connection with the robots command interface
	CmdSocket.ConnectTCP();

	//Query the user in order to get all required information to form a packet, as defined in PktDef
	CmdType cmd;
	MotorBody Data;
	PktDef sendPacket;
	
	while(exeComplete == false){
		
		unsigned short int option,optcmd, direction = 0, duration = 0;
		char* sendBuff, recvBuff[128];

		//First setting the command and then the direction and duration if needed
		std::cout << "Please enter a command." << std::endl;
		std::cout << "1 - Drive" << std::endl;
		std::cout << "4 - Lift/Lower Arm" << std::endl;
		std::cout << "5 - Open/Close Claw" << std::endl;
		std::cout << "3 - Sleep" << std::endl;
		std::cout << "Command: ";
		std::cin >> option;
		std::cout << std::endl;

		sendPacket.SetCmd((CmdType)option);
		cmd = sendPacket.GetCmd();
		
		if (cmd != SLEEP) {

			if (cmd == DRIVE) {
				std::cout << "Please enter the Direction you wish to move in!." << std::endl;
				std::cout << "1 - Forward" << std::endl;
				std::cout << "2 - Backward" << std::endl;
				std::cout << "3 - Right" << std::endl;
				std::cout << "4 - Left" << std::endl;
				std::cin >> optcmd;

				if (optcmd == 1) 
				{
					direction = optcmd;
					duration = 1;
				}
				else
				{
					direction = optcmd;
					duration = 1;
				}
				
			}
			else if (cmd == ARM) {
				std::cout << "Would you like to move the arm Up or Down." << std::endl;
				std::cout << "1 - Up" << std::endl;
				std::cout << "2 - Down" << std::endl;
				std::cin >> optcmd;
				if (optcmd == 1)
					direction = 5;
				else if (optcmd == 2)
					direction = 6;
			}
			else if (cmd == CLAW) {
				std::cout << "Would you like to open or close the claw." << std::endl;
				std::cout << "1 - Open" << std::endl;
				std::cout << "2 - Close" << std::endl;
				std::cin >> optcmd;
				if (optcmd == 1)
					direction = 7;
				else if (optcmd == 2)
					direction = 8;

			}

			

			/*
			if (cmd == DRIVE) {
				std::cout << "Please enter the Duration you wish to move(in seconds): ";
				std::cin >> duration;
				std::cout << std::endl;
			}
			*/
			

			Data.Direction = (char)direction;
			Data.Duration = (char)duration;
			sendPacket.SetBodyData((char*)&Data, sizeof(Data));

		}else if (cmd == SLEEP) {
			sendPacket.SetBodyData((char*)&Data, 0);
		}

		//Set the PktCount adn increment
		sendPacket.SetPktCount(1);
		
		//Generate CRC first
		sendPacket.CalcCRC();

		//Generate a Packet of type PktDef based on the user input and increment the PktCount number.
		sendBuff = new char[sendPacket.GetLength()];
		sendBuff = sendPacket.GenPacket();
		
		//Transmit the Packet to the robot via the MySocket connection
		CmdSocket.SendData(sendBuff, sendPacket.GetLength());

		//Wait for an acknowledgement packet from the robot
		CmdSocket.GetData(recvBuff);
		
		//Creating new packet out of recieved data
		PktDef recvPacket(recvBuff);

		//This process will continue until the user requests to send a SLEEP command to the robot. Upon
		//receiving an Ack packet from the robot, acknowledging the SLEEP command.
		if (sendPacket.GetCmd() == SLEEP) {
			CmdSocket.DisconnectTCP();
			exeComplete = true;
		}
		else if (recvPacket.GetAck() == true) {
			std::cout << "Command Acknowlaged: ";

			if (recvPacket.GetCmd() == 1) {
				std::cout << "Drive" << std::endl;
			}
			else if (recvPacket.GetCmd() == 4) {
				std::cout << "Lift/Lower Arm" << std::endl;
			}
			else if (recvPacket.GetCmd() == 5) {
				std::cout << "Open/Close Claw" << std::endl;
			}
				
		}
		else if (recvPacket.GetAck() == false) {
			std::cout << "Packet Failure" << std::endl;
		}
	}	
}

void TeleThread(unsigned int port, std::string ipAddr) {
	//Create a MySocket object configured as a SocketType::CLIENT and ConnectionType::TCP
	MySocket TeleSocket(CLIENT, ipAddr, port, TCP, 100);

	//Perform the 3 - way handshake to establish a reliable connection with the robot’s command interface
	TeleSocket.ConnectTCP();

	
	//Receive and process all incoming telemetry packets from the Robot.Processing includes, but is not limited to:
	//Verification of the CRC
	while (exeComplete == false) {

		
		char recvBuff[100];
		char* ptr = recvBuff;
		TelBody telData;

		TeleSocket.GetData(recvBuff);

		//Creating new packet out of recieved data
		PktDef recvPacket(recvBuff);
		bool x = recvPacket.CheckCRC(recvBuff, recvPacket.GetLength());

		if (x) {

			if (recvPacket.GetStatus()) {

				//Display RAW Data
				std::cout << "RAW Data: ";
				for (int i = 0; i < (int)recvPacket.GetLength(); i++) {
					std::cout << std::hex << std::setw(4) << (unsigned int)*(ptr++) << ", ";
				}

				//Display sonar and arm data
				std::cout << std::dec << std::endl;

				unsigned short int* ptr2 = (unsigned short int*)recvPacket.GetBodyData();

				std::cout << "Sonar & Arm Data: " << std::endl;
					for (int i = 0; i < 2; i++)
						std::cout << *ptr2++ << ", ";

				/*
				std::cout << "Sonar Reading: " << *ptr2 << std::endl;
				ptr2++;

				std::cout << "Arm Reading: " << *ptr2 << std::endl;
				*/
				
				std::cout << std::endl;
				//Loading TelBody struct for interpretation
				ptr = recvPacket.GetBodyData() + (sizeof(unsigned short int) * 2);
				telData.DriveF = *ptr & 1;
				telData.ArmUp = (*ptr >> 1) & 1;
				telData.ArmDown = (*ptr >> 2) & 1;
				telData.ClawOpen = (*ptr >> 3) & 1;
				telData.ClawClosed = (*ptr >> 4) & 1;
				telData.Padding = 0;

				std::cout << "Drive Bit: " << (bool)telData.DriveF << std::endl;

				if (telData.ArmUp == 1 && telData.ClawOpen == 1) {
					std::cout << "Arm: UP, Claw: OPEN" << std::endl;
				}
				else if (telData.ArmDown == 1 && telData.ClawOpen == 1) {
					std::cout << "Arm: DOWN, Claw: OPEN" << std::endl;
				}
				else if (telData.ArmUp == 1 && telData.ClawClosed == 1) {
					std::cout << "Arm: UP, Claw: CLOSED" << std::endl;
				}
				else if (telData.ArmDown == 1 && telData.ClawClosed == 1) {
					std::cout << "Arm: DOWN, Claw: CLOSED" << std::endl;
				}

			}
			else{
				std::cout << "ERROR: STATUS FLAG NOT SET" << std::endl;
			}

		}
		else{
			std::cout << "ERROR: CRC's DID NOT MATCH" << std::endl;
		}
	}
}

int main() {
	//By default the simulator will use an IP address of 127.0.0.1 and ports 27000 for
	//Commands and 27501 for Telemetry

	std::string IpAddr = "";
	unsigned int cmdPort = 0, telPort = 0;

	std::cout << "Please enter the IP Address you wish to connect to" << std::endl;
	std::cin >> IpAddr;
	std::cout << "Please enter the Port you wish to connect to for the command thread" << std::endl;
	std::cin >> cmdPort;
	std::cout << "Please enter the Port you wish to connect to for the telemetry thread" << std::endl;
	std::cin >> telPort;

	//Creating and detaching threads
	std::thread(CommandThread, cmdPort, IpAddr).detach();
	std::thread(TeleThread, telPort, IpAddr).detach();

	//Runs main process until thread complete and change
	while (!exeComplete) {};

	return 0;
}
