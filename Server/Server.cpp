#include "resource.h"
#include <winsock2.h> 
#include <WindowsX.h>
#include <Windows.h>
#include <tchar.h>
#include <StrSafe.h>
#include <stdarg.h>


#pragma comment(lib, "Ws2_32")

////////////////////////
#define DATA_BUFSIZE 8192

///////////////////////
////////////////////////
//结构体
typedef struct                        //这个玩意就是灌数据，取数据的一个自定义数据结构
	//和那个wm_data差不了多少，不过就是老要塞一个OverLapped结构， 
{ 
	OVERLAPPED Overlapped; 
	WSABUF DataBuf; 
	CHAR Buffer[DATA_BUFSIZE];                     
	DWORD BytesSEND;                                 //发送字节数 
	DWORD BytesRECV;                                 
} PER_IO_OPERATION_DATA, * LPPER_IO_OPERATION_DATA;

typedef struct 
{ 
	SOCKET Socket; 
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;
/////////////////////////////
/////////////////////////
//全局变量

SOCKET* g_SocketVecotr[30];
int g_SocketIndex = 0;
SOCKET g_Listen; 

HINSTANCE g_hInstance = NULL;
HWND g_hMainDlg = NULL;

#define INFO_CTRL GetDlgItem(g_hMainDlg, IDC_INFO)

///////////////////////////////////////////////////////////////////////////////


// Adds a string to the "Details" edit control
void AddText(PCTSTR pszFormat, ...) {

	va_list argList;
	va_start(argList, pszFormat);

	TCHAR sz[20 * 1024];

	Edit_GetText(INFO_CTRL, sz, _countof(sz));
	_vstprintf_s(
		_tcschr(sz, TEXT('\0')), _countof(sz) - _tcslen(sz), 
		pszFormat, argList);
	Edit_SetText(INFO_CTRL, sz);
	va_end(argList);
}
// 
DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID){

	AddText("create thread %d \r\n", GetCurrentThreadId());
	HANDLE CompletionPort = (HANDLE) CompletionPortID; 

	DWORD BytesTransferred; 
	LPOVERLAPPED Overlapped; 
	LPPER_HANDLE_DATA PerHandleData; 
	LPPER_IO_OPERATION_DATA PerIoData;         
	DWORD SendBytes, RecvBytes; 
	DWORD Flags; 

	while(TRUE)
	{ 
		// 
		//在这里检查完成端口部分的数据buf区，数据来了吗？ 
		// 这个函数参数要看说明， 
		// PerIoData 就是从管子流出来的数据, 
		//PerHandleData 也是从管子里取出的，是何时塞进来的， 
		//就是在建立第2次createIocompletionPort时 
		// 
		AddText("begint GetQueuedCompletionStatus  threadID %d  \r\n", GetCurrentThreadId());
		if (GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (LPDWORD)&PerHandleData, (LPOVERLAPPED *) &PerIoData, INFINITE) == 0) 
		{ 
			AddText("GetQueuedCompletionStatus failed with error %d\r\n", GetLastError());
			AddText("Closing socket %d\r\n", PerHandleData->Socket);
			if (closesocket(PerHandleData->Socket) == SOCKET_ERROR) 
			{ 
				AddText("closesocket() failed with error %d\r\n", WSAGetLastError()); 
				return 0; 
			}
			GlobalFree(PerHandleData); 
			GlobalFree(PerIoData); 
			// 把列表对应的玩家去掉

			continue;
			//return 0; 
		}
		AddText("end GetQueuedCompletionStatus \r\n");
		// 检查数据传送完了吗 (传输为0,只有调用接口结束了,正常的操作不会出现)
		if (BytesTransferred == 0) 
		{ 
			AddText("Closing socket %d\r\n", PerHandleData->Socket);
			if (closesocket(PerHandleData->Socket) == SOCKET_ERROR) 
			{ 
				AddText("closesocket() failed with error %d\r\n", WSAGetLastError()); 
				return 0; 
			}
			GlobalFree(PerHandleData); 
			GlobalFree(PerIoData); 
			continue; 
		}     
		// 
		//看看管子里面有数据来了吗？=0，那是刚收到数据 
		// 
		if (PerIoData->BytesRECV == 0) 
		{ 
			PerIoData->BytesRECV = BytesTransferred; 
			PerIoData->BytesSEND = 0; 
		} 
		else   //来了， 
		{ 
			PerIoData->BytesSEND += BytesTransferred; 
		} 
		AddText("print rev data: %d, end %d , BytesTransferred %d \r\n", PerIoData->BytesRECV, PerIoData->BytesSEND, BytesTransferred); 

		// 
		// 数据没发完？继续send出去 
		// 
		if (PerIoData->BytesRECV > PerIoData->BytesSEND) 
		{
			ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED)); //清0为发送准备 

			//解析数据,获取转发的客户端
			CHAR* dataStart = strchr(PerIoData->Buffer, int(':'));
			if(dataStart == NULL)
			{
				AddText("socket from %d, send data error \r\n ", PerHandleData->Socket);
				//继续等待
				if (WSARecv(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags, 
					&(PerIoData->Overlapped), NULL) == SOCKET_ERROR) 
				{ 
					if (WSAGetLastError() != ERROR_IO_PENDING) 
					{ 
						AddText("WSARecv() failed with error %d\r\n", WSAGetLastError()); 
						return 0; 
					} 
				} 
				AddText("end WSARecv data\r\n");
			}
			else
			{
				//找到端口
				CHAR strIndex[MAX_PATH] = {0};
				strncpy(strIndex, PerIoData->Buffer,dataStart-PerIoData->Buffer);
				int index = atoi(strIndex);
				AddText("index %d , socket %d\r\n ", index, *g_SocketVecotr[index]);
				if (WSASend(*g_SocketVecotr[index], &(PerIoData->DataBuf), 1, &SendBytes, 0, 
					&(PerIoData->Overlapped), NULL) == SOCKET_ERROR) 
				{ 
					if (WSAGetLastError() != ERROR_IO_PENDING) 
					{ 
						AddText("WSASend() failed with error %d\r\n", WSAGetLastError()); 
						return 0; 
					} 
				} 

				AddText("send data\r\n");
			}

		} 
		else 
		{ 
			AddText("WSARecv data\r\n");

			PerIoData->BytesRECV = 0;
			Flags = 0; 
			ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));
			PerIoData->DataBuf.len = DATA_BUFSIZE; 
			PerIoData->DataBuf.buf = PerIoData->Buffer;
			if (WSARecv(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags, 
				&(PerIoData->Overlapped), NULL) == SOCKET_ERROR) 
			{ 
				if (WSAGetLastError() != ERROR_IO_PENDING) 
				{ 
					AddText("WSARecv() failed with error %d\r\n", WSAGetLastError()); 
					return 0; 
				} 
			} 
			AddText("end WSARecv data\r\n");
		} 
	} 

	return 0;
}

DWORD WINAPI ServerMainThread(LPVOID CompletionPortID){
	HANDLE CompletionPort = (HANDLE) CompletionPortID; 
	AddText("wait for accept\r\n");
	SOCKET Accept; 
	LPPER_HANDLE_DATA PerHandleData; 
	LPPER_IO_OPERATION_DATA PerIoData; 

	DWORD RecvBytes; 
	DWORD Flags;
	while(TRUE) 
	{
		//主线程跑到这里就等啊等啊，但是线程却开工了， 
		if ((Accept =  (g_Listen, NULL, NULL, NULL, 0)) == SOCKET_ERROR) 
		{ 
			AddText("WSAAccept() failed with error %d\r\n", WSAGetLastError()); 
			return -1; 
		} 
		AddText("accept \r\n");
		//该函数从堆中分配一定数目的字节数.Win32内存管理器并不提供相互分开的局部和全局堆.提供这个函数只是为了与16位的Windows相兼容
		//
		if ((PerHandleData = (LPPER_HANDLE_DATA) GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA))) == NULL) 
		{ 
			AddText("GlobalAlloc() failed with error %d\r\n", GetLastError()); 
			return -1; 
		}

		PerHandleData->Socket = Accept;
		AddText("socket %d \r\n", Accept);
		g_SocketVecotr[g_SocketIndex++] =  &PerHandleData->Socket;
		// 
		//把这头和完成端口completionPort连起来 
		//就像你把漏斗接到管子口上,开始要灌数据了 
		// 
		if (CreateIoCompletionPort((HANDLE) Accept, CompletionPort, (DWORD) PerHandleData, 0) == NULL) 
		{ 
			AddText("CreateIoCompletionPort failed with error %d\r\n", GetLastError()); 
			return -1; 
		} 

		// 
		//清管子的数据结构,准备往里面灌数据 
		// 
		if ((PerIoData = (LPPER_IO_OPERATION_DATA) GlobalAlloc(GPTR,sizeof(PER_IO_OPERATION_DATA))) == NULL) 
		{ 
			AddText("GlobalAlloc() failed with error %d\r\n", GetLastError()); 
			return -1; 
		}
		//用0填充一块内存区域
		ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED)); 
		PerIoData->BytesSEND = 0; 
		PerIoData->BytesRECV = 0; 
		PerIoData->DataBuf.len = DATA_BUFSIZE; 
		PerIoData->DataBuf.buf = PerIoData->Buffer;
		Flags = 0; 

		// 
		// accept接到了数据，就放到PerIoData中,而perIoData又通过线程中的函数取出, 
		// 
		AddText("wait for main WSARecv \r\n");
		if (WSARecv(Accept, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags, &(PerIoData->Overlapped), NULL) == SOCKET_ERROR) 
		{ 
			if (WSAGetLastError() != ERROR_IO_PENDING) 
			{ 
				AddText("WSARecv() failed with error %d\r\n", WSAGetLastError()); 
				return -1; 
			} 
			AddText("end wait for main WSARecv \r\n");
		} 
	} 
	return 0;
}
////////////////////////
LRESULT WINAPI Dlg_OnInitDialog(HWND hwnd, HWND, LPARAM lParam){
	g_hMainDlg = hwnd;
	SetDlgItemText(hwnd, IDC_PORT, "9000");
	SetDlgItemText(hwnd, IDC_IPADDRESS, "127.0.0.1");
	WSADATA wsaData; 
	DWORD Ret;
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0) 
	{ 
		AddText("WSAStartup failed with error %d\r\n", Ret); 
		return FALSE;
	}
	AddText("WSAStartup\r\n"); 
	return FALSE;
}

void StartServer(){
	CHAR szAddress[MAX_PATH] = {0};
	CHAR sz[MAX_PATH] = {0};
	GetDlgItemText(g_hMainDlg, IDC_PORT, sz, MAX_PATH);
	GetDlgItemText(g_hMainDlg, IDC_IPADDRESS, szAddress, MAX_PATH);
	UINT uPort = atoi(sz);
	
	HANDLE CompletionPort; 
	SYSTEM_INFO SystemInfo;
	SOCKADDR_IN InternetAddr; 
	
	DWORD ThreadID; 
	int i;
	HANDLE ThreadHandle; 
	
	// 
	//完成端口的建立得搞2次，这是第一次调用，至于为什么？我问问你 
	// 
	if ((CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) 
	{ 
		AddText( "CreateIoCompletionPort failed with error: %d\r\n", GetLastError()); 
		return;
	} 
	//老套子api，不谈也罢 
	GetSystemInfo(&SystemInfo); 

	//发现2个CPU，那就开个双倍的线程跑吧 
	for(i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++) 
	{ 
		// 
		//完成端口挂到线程上面来了，就像管子把灌数据的和读数据的两头都连上了
		// 
		if ((ThreadHandle = CreateThread(NULL, 0, ServerWorkerThread, CompletionPort, 0, &ThreadID)) == NULL)
		{ 
			AddText("CreateThread() failed with error %d\r\n", GetLastError()); 
			return; 
		}      
		CloseHandle(ThreadHandle);
	}
	// 
	//启动一个监听socket ，以下都是长长的交代 
	// 
	if ((g_Listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{ 
		AddText("WSASocket() failed with error %d\r\n", WSAGetLastError());
		return; 
	}
	InternetAddr.sin_family = AF_INET; 
	InternetAddr.sin_addr.s_addr = inet_addr(szAddress); 
	InternetAddr.sin_port = htons(uPort);
	if (bind(g_Listen, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR) 
	{ 
		AddText("bind() failed with error %d\r\n", WSAGetLastError()); 
		return; 
	}  
	if (listen(g_Listen, 5) == SOCKET_ERROR) 
	{ 
		AddText("listen() failed with error %d\r\n", WSAGetLastError()); 
		return; 
	}

	if ((ThreadHandle = CreateThread(NULL, 0, ServerMainThread, CompletionPort, 0, &ThreadID)) == NULL)
	{ 
		AddText("CreateThread() failed with error %d\r\n", GetLastError()); 
		return; 
	}      
	CloseHandle(ThreadHandle);
}
LRESULT WINAPI Dlg_OnCommand(HWND hwnd, int id, HWND hwndctl, UINT codeNotify){
	switch(id){
	case IDCANCEL:
		EndDialog(hwnd, id);
		break;
	case IDC_SEND:
		AddText("test");
		Edit_SetText(INFO_CTRL, "tese");
		break;
	case IDC_START:
		StartServer();
		break;
	}
	return FALSE;
}

INT_PTR WINAPI Dlg_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg){
		HANDLE_MSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, Dlg_OnCommand);
	default:
		return DefWindowProc(hwnd,uMsg, wParam, lParam);
	}
}

int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ){
	g_hInstance = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, Dlg_proc);
	return 0;
}