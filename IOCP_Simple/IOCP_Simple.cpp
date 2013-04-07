#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define DATA_SIZE 1024
typedef enum
{
	RECV,
	SEND,
}OPERATION_TYPE;       //Ã¶¾Ù,±íÊ¾×´Ì¬


typedef struct{
	OVERLAPPED overLapped;
	WSABUF Buffer;
	char szMessage[DATA_SIZE];
	OPERATION_TYPE OperationType;
	DWORD NumberOfBytesRecvd;
	DWORD Flags;
}PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

typedef struct{
	SOCKET sock;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


void WorkThread(LPVOID completionPortID){
	HANDLE compleionPort = (HANDLE)completionPortID;
	SOCKET con;
	DWORD dataRecv;
	LPPER_IO_OPERATION_DATA lpOperationData;
	LPPER_HANDLE_DATA lpHandleData;
	while(1){
		if(GetQueuedCompletionStatus(compleionPort, &dataRecv, (PULONG_PTR)&lpHandleData, (LPOVERLAPPED)&lpOperationData, INFINITE) == 0){
			return;
		}


	}
}
void main()
{
	WSAData wsaData;
	int err = WSAStartup(0x0202, &wsaData);
	if(err != 0){
		return;
	}
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){
		WSACleanup();
		return;
	}

	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN sockAddrs;
	sockAddrs.sin_family = AF_INET;
	sockAddrs.sin_port = htons(9000);
	sockAddrs.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int len = sizeof(SOCKADDR);
	SOCKET con = connect(sockClient, (SOCKADDR*)&sockAddrs, len);
	if(WSAGetLastError() != 0){
		WSACleanup();
		return;
	}
	char buff[100];
	recv(con, buff, 100, 0);
	printf("%s\n", buff);

	WSACleanup();
	system("pause");
}