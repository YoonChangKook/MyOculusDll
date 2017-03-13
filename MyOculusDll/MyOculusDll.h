#pragma once

#ifndef _MY_OCULUS_H_
#define _MY_OCULUS_H_

#ifdef MY_OCULUS_EXPORTS
#define MY_OCULUS_API __declspec(dllexport)
#else
#define MY_OCULUS_API __declspec(dllimport)
#endif

#define READ_INTERVAL 50

#include <OVR_CAPI.h>
#include <chrono>
#include <thread>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;

enum RobotPacketType {
	CALIB = 0,
	CALIB_OK = 1,
	ROT = 2,
	ROT_OK = 3,
	PACKET_ERROR = 100
};

#pragma pack(push, 1)
typedef struct RobotPacket {
	BYTE type;
	double pitch;
	double yaw;
} RobotPacket;
#pragma pack(pop)

class MY_OCULUS_API MyOculus
{
public:
	MyOculus();
	virtual ~MyOculus();

private:
	// read oculus and send to the robot task
	thread* reader;
	// udp socket members
	struct sockaddr_in addr;
	SOCKET robotSocket;
	// oculus members
	ovrSession session;
	ovrGraphicsLuid luid;
	ovrResult result;
	bool isOvrInit;

	// thread task
	void Read();

public:
	bool IsOculusInitialized();
	bool InitOculus();
	bool ConnectToRobot_UDP(__in const char* ip, __in const short& port);
	void DisconnectToRobot();
	bool RequestCalibration();
	bool RequestRotation(__in const double& pitch, __in const double& yaw);
};

#endif