// TabPageServer.cpp : implementation file
//

#include "stdafx.h"
#include "DeployTool.h"
#include "TabPageServer.h"
#include "SocketHelper.h"
#include "DeployManager.h"
#include <sstream>

#define MAXRECV 247815

#define DBOUT( s )            \
{                             \
   std::stringstream os_;    \
   os_ << s;                  \
   OutputDebugString( os_.str().c_str() );  \
}

// CTabPageServer dialog

unsigned int __stdcall ServerConnectThread_wrapper(void* data);
unsigned int __stdcall ServerUpdateThread_wrapper(void* data);

IMPLEMENT_DYNAMIC(CTabPageServer, CDialog)
CTabPageServer::CTabPageServer(CWnd* pParent /*=NULL*/)
	: CDialog(CTabPageServer::IDD, pParent)
{
	mRecvBuffer = new char[MAXRECV];
	mSendBuffer = new char[MAXRECV];
	mDeployPort = 5000;
}

CTabPageServer::~CTabPageServer()
{
}

void CTabPageServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabPageServer, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CTabPageServer message handlers

void CTabPageServer::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

}

void CTabPageServer::OnShowWindow(BOOL bShow, UINT nStatus)
{
}

void CTabPageServer::Startup()
{
	mfExitThread = false;

	// startup server thread
	SocketHelper::CreateServerSocket(mhServerSocket, mDeployPort);

	// start thread to connect to clients
	HANDLE ServerConnectThreadHandle = (HANDLE)_beginthreadex(0, 0, &ServerConnectThread_wrapper, this, 0, 0);
	SetThreadPriority(ServerConnectThreadHandle, THREAD_PRIORITY_NORMAL);

	// start thread to update
	HANDLE ServerUpdateThreadHandle = (HANDLE)_beginthreadex(0, 0, &ServerUpdateThread_wrapper, this, 0, 0);
	SetThreadPriority(ServerUpdateThreadHandle, THREAD_PRIORITY_NORMAL);

	// create DeployManager and create test app
	DeployManager::instance()->AddDeployApp("C:\\Users\\joel\\Desktop\\12_9_2014", "render_test.exe");
}

void CTabPageServer::Shutdown()
{
	mfExitThread = true;

	// Close the socket and mark as 0 in list for reuse
	closesocket(mhServerSocket);
}

unsigned int __stdcall ServerConnectThread_wrapper(void* data)
{
    static_cast<CTabPageServer*>(data)->ServerConnectThread();
	return 0;
}

void CTabPageServer::ServerConnectThread()
{
	while (!mfExitThread)
	{
		SOCKET hClientSocket;

		// listen for clients
		if (SocketHelper::WaitForClient(mhServerSocket, hClientSocket))
		{
			mhClients.push_back(hClientSocket);

			// send files to client
			DeployManager::instance()->SendToClient(hClientSocket);

			// start up the app
			DeployManager::instance()->StartApp(hClientSocket, "render_test.exe");
		}
		Sleep(100);
	}
}

unsigned int __stdcall ServerUpdateThread_wrapper(void* data)
{
    static_cast<CTabPageServer*>(data)->ServerUpdateThread();
	return 0;
}

void CTabPageServer::ServerUpdateThread()
{
	while (!mfExitThread)
	{
		// update clients
	}
}