
// WifiDirectDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "WifiDirect.h"
#include "WifiDirectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWifiDirectDlg 对话框



CWifiDirectDlg::CWifiDirectDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WIFIDIRECT_DIALOG, pParent), _apEvent(CreateEventEx(nullptr, nullptr, 0, WRITE_OWNER | EVENT_ALL_ACCESS))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	HRESULT hr = _apEvent.IsValid() ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	if (FAILED(hr))
	{
		std::wcout << "Failed to create AP event: " << hr << std::endl;
		throw WlanHostedNetworkException("Create event failed", hr);
	}

	_hostedNetwork.RegisterListener(this);
	_hostedNetwork.RegisterPrompt(this);
}

void CWifiDirectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, SSID);
	DDX_Control(pDX, IDC_EDIT2, PASS);
	DDX_Control(pDX, IDC_BUTTON2, ShowPasswordButton);
	DDX_Control(pDX, IDC_BUTTON1, WifiButton);
}

BEGIN_MESSAGE_MAP(CWifiDirectDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CWifiDirectDlg::OnShowPasswordClick)
	ON_COMMAND(IDCLOSE, &CWifiDirectDlg::OnClose)
	ON_BN_CLICKED(IDC_BUTTON1, &CWifiDirectDlg::OnWifiClick)
END_MESSAGE_MAP()


// CWifiDirectDlg 消息处理程序

BOOL CWifiDirectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWifiDirectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CWifiDirectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWifiDirectDlg::OnShowPasswordClick()
{
	CEdit* password = (CEdit*)GetDlgItem(IDC_EDIT2);
	if (isShowPassword)
	{
		// TODO: 在此添加控件通知处理程序代码
		ShowPasswordButton.SetWindowTextW(TEXT("显示密码"));
		isShowPassword = false;
		PASS.SetPasswordChar('*');
	}
	else {
		ShowPasswordButton.SetWindowTextW(TEXT("隐藏密码"));
		isShowPassword = true;
		PASS.SetPasswordChar(0);
		
	}
	UpdateData(FALSE);
	PASS.Invalidate();
}


void CWifiDirectDlg::OnClose()
{
	// TODO: 在此添加命令处理程序代码
	_hostedNetwork.Stop();
	EndDialog(IDCANCEL);
}


void CWifiDirectDlg::OnWifiClick()
{
	// TODO: 在此添加控件通知处理程序代码
	if (isRunning)
	{
		WifiButton.SetWindowTextW(TEXT("开启热点"));
		SSID.SetReadOnly(false);
		PASS.SetReadOnly(false);
		isRunning = false;
	}
	else {
		WifiButton.SetWindowTextW(TEXT("关闭热点"));
		SSID.SetReadOnly(true);
		PASS.SetReadOnly(true);
		isRunning = true;
	}
}

void CWifiDirectDlg::OnDeviceConnected(std::wstring remoteHostName)
{
	std::wcout << std::endl << "Peer connected: " << remoteHostName << std::endl;
}

void CWifiDirectDlg::OnDeviceDisconnected(std::wstring deviceId)
{
	std::wcout << std::endl << "Peer disconnected: " << deviceId << std::endl;
}

void CWifiDirectDlg::OnAdvertisementStarted()
{
	std::wcout << "Soft AP started!" << std::endl
		<< "Peers can connect to: " << _hostedNetwork.GetSSID() << std::endl
		<< "Passphrase: " << _hostedNetwork.GetPassphrase() << std::endl;
	SetEvent(_apEvent.Get());
}

void CWifiDirectDlg::OnAdvertisementStopped(std::wstring message)
{
	std::wcout << "Soft AP stopped." << std::endl;
	SetEvent(_apEvent.Get());
}

void CWifiDirectDlg::OnAdvertisementAborted(std::wstring message)
{
	std::wcout << "Soft AP aborted: " << message << std::endl;
	SetEvent(_apEvent.Get());
}

void CWifiDirectDlg::OnAsyncException(std::wstring message)
{
	std::wcout << std::endl << "Caught exception in asynchronous method: " << message << std::endl;
}

void CWifiDirectDlg::LogMessage(std::wstring message)
{
	std::wcout << std::endl << message << std::endl;
}

bool CWifiDirectDlg::AcceptIncommingConnection()
{
	return true;
	//std::wcout << std::endl << "Accept peer connection? (y/n)" << std::endl;

	//std::wstring response;
	//getline(std::wcin, response);

	//if (response.length() > 0 &&
	//	(response[0] == 'y' || response[0] == 'Y'))
	//{
	//	return true;
	//}

	//return false;
}