#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

void main()
{
	WSAData wsaData;
	int err = WSAStartup(0x0202, &wsaData);
	if(err != 0){
		return;
	}
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){
		WSACleanup();
		printf("start up error\n");
		system("pause");
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
		printf("connect error  \n");
		system("pause");
		return;
	}

	send(con, "sdf", 4, 0);
	printf("send finish\n");
	char buff[2220];
	int buflen;
	while(true){
		buflen = recv(con, buff, 2200, 0);
		if(buflen < 0)
		{
				continue;
		}
		else if(buflen == 0)
		{
			return;
		}

		if(buflen != sizeof(buff))
			printf("%s\n", buff);
		
	}

	WSACleanup();
	system("pause");
}