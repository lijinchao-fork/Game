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
//�ṹ��
typedef struct                        //���������ǹ����ݣ�ȡ���ݵ�һ���Զ������ݽṹ
	//���Ǹ�wm_data��˶��٣�����������Ҫ��һ��OverLapped�ṹ�� 
{ 
	OVERLAPPED Overlapped; 
	WSABUF DataBuf; 
	CHAR Buffer[DATA_BUFSIZE];                     
	DWORD BytesSEND;                                 //�����ֽ��� 
	DWORD BytesRECV;                                 
} PER_IO_OPERATION_DATA, * LPPER_IO_OPERATION_DATA;

typedef struct 
{ 
	SOCKET Socket; 
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;
/////////////////////////////
/////////////////////////
//ȫ�ֱ���

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
		//����������ɶ˿ڲ��ֵ�����buf�������������� 
		// �����������Ҫ��˵���� 
		// PerIoData ���Ǵӹ���������������, 
		//PerHandleData Ҳ�Ǵӹ�����ȡ���ģ��Ǻ�ʱ�������ģ� 
		//�����ڽ�����2��createIocompletionPortʱ 
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
			// ���б��Ӧ�����ȥ��

			continue;
			//return 0; 
		}
		AddText("end GetQueuedCompletionStatus \r\n");
		// ������ݴ��������� (����Ϊ0,ֻ�е��ýӿڽ�����,�����Ĳ����������)
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
		//������������������������=0�����Ǹ��յ����� 
		// 
		if (PerIoData->BytesRECV == 0) 
		{ 
			PerIoData->BytesRECV = BytesTransferred; 
			PerIoData->BytesSEND = 0; 
		} 
		else   //���ˣ� 
		{ 
			PerIoData->BytesSEND += BytesTransferred; 
		} 
		AddText("print rev data: %d, end %d , BytesTransferred %d \r\n", PerIoData->BytesRECV, PerIoData->BytesSEND, BytesTransferred); 

		// 
		// ����û���ꣿ����send��ȥ 
		// 
		if (PerIoData->BytesRECV > PerIoData->BytesSEND) 
		{
			ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED)); //��0Ϊ����׼�� 

			//��������,��ȡת���Ŀͻ���
			CHAR* dataStart = strchr(PerIoData->Buffer, int(':'));
			if(dataStart == NULL)
			{
				AddText("socket from %d, send data error \r\n ", PerHandleData->Socket);
				//�����ȴ�
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
				//�ҵ��˿�
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
		//���߳��ܵ�����͵Ȱ��Ȱ��������߳�ȴ�����ˣ� 
		if ((Accept =  (g_Listen, NULL, NULL, NULL, 0)) == SOCKET_ERROR) 
		{ 
			AddText("WSAAccept() failed with error %d\r\n", WSAGetLastError()); 
			return -1; 
		} 
		AddText("accept \r\n");
		//�ú����Ӷ��з���һ����Ŀ���ֽ���.Win32�ڴ�����������ṩ�໥�ֿ��ľֲ���ȫ�ֶ�.�ṩ�������ֻ��Ϊ����16λ��Windows�����
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
		//����ͷ����ɶ˿�completionPort������ 
		//�������©���ӵ����ӿ���,��ʼҪ�������� 
		// 
		if (CreateIoCompletionPort((HANDLE) Accept, CompletionPort, (DWORD) PerHandleData, 0) == NULL) 
		{ 
			AddText("CreateIoCompletionPort failed with error %d\r\n", GetLastError()); 
			return -1; 
		} 

		// 
		//����ӵ����ݽṹ,׼������������� 
		// 
		if ((PerIoData = (LPPER_IO_OPERATION_DATA) GlobalAlloc(GPTR,sizeof(PER_IO_OPERATION_DATA))) == NULL) 
		{ 
			AddText("GlobalAlloc() failed with error %d\r\n", GetLastError()); 
			return -1; 
		}
		//��0���һ���ڴ�����
		ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED)); 
		PerIoData->BytesSEND = 0; 
		PerIoData->BytesRECV = 0; 
		PerIoData->DataBuf.len = DATA_BUFSIZE; 
		PerIoData->DataBuf.buf = PerIoData->Buffer;
		Flags = 0; 

		// 
		// accept�ӵ������ݣ��ͷŵ�PerIoData��,��perIoData��ͨ���߳��еĺ���ȡ��, 
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
	//��ɶ˿ڵĽ����ø�2�Σ����ǵ�һ�ε��ã�����Ϊʲô���������� 
	// 
	if ((CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) 
	{ 
		AddText( "CreateIoCompletionPort failed with error: %d\r\n", GetLastError()); 
		return;
	} 
	//������api����̸Ҳ�� 
	GetSystemInfo(&SystemInfo); 

	//����2��CPU���ǾͿ���˫�����߳��ܰ� 
	for(i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++) 
	{ 
		// 
		//��ɶ˿ڹҵ��߳��������ˣ�������Ӱѹ����ݵĺͶ����ݵ���ͷ��������
		// 
		if ((ThreadHandle = CreateThread(NULL, 0, ServerWorkerThread, CompletionPort, 0, &ThreadID)) == NULL)
		{ 
			AddText("CreateThread() failed with error %d\r\n", GetLastError()); 
			return; 
		}      
		CloseHandle(ThreadHandle);
	}
	// 
	//����һ������socket �����¶��ǳ����Ľ��� 
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