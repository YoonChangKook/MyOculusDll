// MyOculusDll.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "MyOculusDll.h"

MyOculus::MyOculus()
	: isOvrInit(false), session(NULL), robotSocket(INVALID_SOCKET)
{
	InitOculus();
	this->reader = new thread(&MyOculus::Read, this);
}

MyOculus::~MyOculus()
{
	if (this->isOvrInit)
	{
		ovr_Destroy(this->session);
		ovr_Shutdown();
	}
	if (this->robotSocket != INVALID_SOCKET)
	{
		closesocket(this->robotSocket);
	}
	if (this->reader != NULL)
	{
		this->reader->detach();
		delete this->reader;
	}
}

bool MyOculus::InitOculus()
{
	if (ovr_Initialize(nullptr) == ovrSuccess)
	{
		this->session = nullptr;
		this->result = ovr_Create(&session, &this->luid);

		if (this->result == ovrSuccess)
		{
			this->isOvrInit = true;
			return true;
		}
		else
		{
			ovr_Shutdown();
			return false;
		}
	}
	else
		return false;
}

bool MyOculus::IsOculusInitialized()
{
	return this->isOvrInit;
}

bool MyOculus::ConnectToRobot_UDP(__in const char* ip, __in const short& port)
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	if ((this->robotSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		return false;

	memset((char*)&this->addr, 0, sizeof(this->addr));
	this->addr.sin_family = AF_INET;
	this->addr.sin_port = htons(port);
	this->addr.sin_addr.S_un.S_addr = inet_addr(ip);

	return true;
}

void MyOculus::DisconnectToRobot()
{
	if (this->robotSocket != INVALID_SOCKET)
	{
		closesocket(this->robotSocket);
		this->robotSocket = INVALID_SOCKET;
	}
}

#pragma pack(push, 1)
bool MyOculus::RequestCalibration()
{
	RobotPacket packet = { RobotPacketType::CALIB, 0, 0 };

	if (sendto(this->robotSocket, (const char*)&packet, sizeof(packet), 0, (struct sockaddr *) &this->addr, sizeof(this->addr)) == SOCKET_ERROR)
	{
		return false;
	}
	
	return true;
}

bool MyOculus::RequestRotation(__in const double& pitch, __in const double& yaw)
{
	RobotPacket packet = { RobotPacketType::ROT, pitch, yaw };

	if (sendto(this->robotSocket, (const char*)&packet, sizeof(packet), 0, (struct sockaddr *) &this->addr, sizeof(this->addr)) == SOCKET_ERROR)
	{
		return false;
	}

	//debug
	//printf("Pitch: %lf, Yaw: %lf\n", pitch, yaw);

	return true;
}
#pragma pack(pop)

void MyOculus::Read()
{
	ovrTrackingState ts;

	while (1)
	{
		this_thread::sleep_for(chrono::milliseconds(READ_INTERVAL));

		if (!MyOculus::IsOculusInitialized())
			continue;
		if (this->robotSocket == INVALID_SOCKET)
			continue;

		ts = ovr_GetTrackingState(session, 0, true);

		ovrPoseStatef tempHeadPose = ts.HeadPose;
		ovrPosef tempPose = tempHeadPose.ThePose;
		ovrQuatf tempOrient = tempPose.Orientation;

		MyOculus::RequestRotation(-tempOrient.x * 180, tempOrient.y * 180);
	}
}