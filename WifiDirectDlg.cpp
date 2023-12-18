
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
	: CDialogEx(IDD_WIFIDIRECT_DIALOG, pParent) , _apEvent(CreateEventEx(nullptr, nullptr, 0, WRITE_OWNER | EVENT_ALL_ACCESS))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	//初始化数据
	HRESULT hr = _apEvent.IsValid() ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	if (FAILED(hr))
	{
		MessageBox(LPCTSTR(_T("Failed to create AP event")));
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
	//  DDX_Control(pDX, IDC_LIST3, connectList);
	DDX_Control(pDX, IDC_LIST3, listCtrl);
}

BEGIN_MESSAGE_MAP(CWifiDirectDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CWifiDirectDlg::OnShowPasswordClick)
	ON_COMMAND(IDCLOSE, &CWifiDirectDlg::OnClose)
	ON_BN_CLICKED(IDC_BUTTON1, &CWifiDirectDlg::OnWifiClick)
	ON_MESSAGE(WM_USER_NOTIFYICON, &CWifiDirectDlg::OnNotifyMsg)
	ON_WM_CLOSE()
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
	AddShellIcon();

	//设置列表
	CRect rect;
	listCtrl.GetWindowRect(&rect);
	int width = ((int)rect.right - (int)rect.left) / 2;
	listCtrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	listCtrl.InsertColumn(0, _T("设备id"), LVCFMT_CENTER,width);
	listCtrl.InsertColumn(1, _T("地址"), LVCFMT_CENTER, width);

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
	//CEdit* password = (CEdit*)GetDlgItem(IDC_EDIT2);
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

void CWifiDirectDlg::OnWifiClick()
{
	//点击热点按钮
	if (isRunning)
	{
		WifiButton.SetWindowTextW(TEXT("开启热点"));
		SSID.SetReadOnly(false);
		PASS.SetReadOnly(false);
		_hostedNetwork.Stop();
		//不知道为什么加了个弹框就可以了
		MessageBox(LPCTSTR(_T("热点已关闭")));
		WaitForSingleObjectEx(_apEvent.Get(), INFINITE, FALSE);
		isRunning = false;
	}
	else {
		CString ssid;
		CString pass;
		SSID.GetWindowTextW(ssid);
		PASS.GetWindowTextW(pass);
		if (ssid.IsEmpty() || pass.IsEmpty())
		{
			MessageBox(LPCTSTR(_T("请输入WIFI名和密码")));
		}
		else if (pass.GetLength() < 8)
		{
			MessageBox(LPCTSTR(_T("WIFI密码不得少于8位")));
		}
		else {
			WifiButton.SetWindowTextW(TEXT("关闭热点"));
			SSID.SetReadOnly(true);
			PASS.SetReadOnly(true);

			//设置SSID和PASS
			_hostedNetwork.SetSSID(ssid.GetString());
			_hostedNetwork.SetPassphrase(pass.GetString());

			_hostedNetwork.Start();
			//不知道为什么加了个弹框就可以了
			MessageBox(LPCTSTR(_T("热点已开启")));
			WaitForSingleObjectEx(_apEvent.Get(), INFINITE, FALSE);
			isRunning = true;
		}
	}
	
	
}

void CWifiDirectDlg::OnDeviceConnected(std::wstring deviceId,std::wstring remoteHostAddress)
{
	//std::wcout << std::endl << "Peer connected: " << remoteHostAddress << std::endl;
}

void CWifiDirectDlg::OnDeviceDisconnected(std::wstring deviceId)
{
	//std::wcout << std::endl << "Peer disconnected: " << deviceId << std::endl;
}

void CWifiDirectDlg::OnAdvertisementStarted()
{
	CString c;
	c.Append(_T("热点已启动"));
	ShowMessage(LPCTSTR(c.GetString()));
	/*std::wcout << "Soft AP started!" << std::endl
		<< "Peers can connect to: " << _hostedNetwork.GetSSID() << std::endl
		<< "Passphrase: " << _hostedNetwork.GetPassphrase() << std::endl;
	*/
	SetEvent(_apEvent.Get());
}

void CWifiDirectDlg::OnAdvertisementStopped(std::wstring message)
{
	CString c;
	c.Append(_T("热点已停止"));
	ShowMessage(LPCTSTR(c.GetString()));
	//std::wcout << "Soft AP stopped." << std::endl;
	SetEvent(_apEvent.Get());
}

void CWifiDirectDlg::OnAdvertisementAborted(std::wstring message)
{
	CString c;
	c.Append(_T("热点已退出"));
	ShowMessage(LPCTSTR(c.GetString()));
	//std::wcout << "Soft AP aborted: " << message << std::endl;
	SetEvent(_apEvent.Get());
}

void CWifiDirectDlg::OnAsyncException(std::wstring message)
{
	CString c;
	c.Append(_T("出现错误："));
	c.Append(message.c_str());
	ShowMessage(LPCTSTR(c.GetString()));
	//std::wcout << std::endl << "Caught exception in asynchronous method: " << message << std::endl;
}

void CWifiDirectDlg::LogMessage(std::wstring message)
{
	CString c;
	c.Append(message.c_str());

	ShowMessage(LPCTSTR(c.GetString()));
	//std::wcout << std::endl << message << std::endl;
}

bool CWifiDirectDlg::AcceptIncommingConnection()
{
	//询问是否要同意加入的
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

LRESULT CWifiDirectDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch (message)
	{
	case WM_SYSCOMMAND:
		//收到最小化，添加到系统托盘
		if (SC_MINIMIZE == wParam)
		{
			AddShellIcon(); // 添加系统托盘图标
			ModifyStyleEx(0, WS_EX_TOPMOST); // 去除任务栏图标
			ShowWindow(SW_HIDE); // 窗体隐藏
			return 0;
		}
		break;
	case WM_USER_NOTIFYICON:
		if (lParam == WM_LBUTTONDBLCLK)
		{
			//鼠标双击时主窗口出现 
			if (AfxGetApp()->m_pMainWnd->IsWindowVisible()) //判断窗口当前状态
			{
				AfxGetApp()->m_pMainWnd->ShowWindow(SW_HIDE); //隐藏窗口
			}
			else
			{
				AfxGetApp()->m_pMainWnd->ShowWindow(SW_SHOW); //显示窗口
			}
		}
		break;
	default:
		break;
	}
	return __super::WindowProc(message, wParam, lParam);
}


void CWifiDirectDlg::AddShellIcon()
{
	// TODO:  在此添加控件通知处理程序代码
	if (m_bAddShellIcon) return; // 标志位，用于标识是否已经添加托盘图标，防止重复操作。


	m_notify.cbSize = sizeof(NOTIFYICONDATA);//结构体的大小
	m_notify.hWnd = m_hWnd;//接收托盘消息的窗口句柄
	m_notify.uID = IDR_MAINFRAME;//定义的托盘图标ID
	m_notify.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;//设置属性，下面的三个属性是否有效
	m_notify.uCallbackMessage = WM_USER_NOTIFYICON;//自定义的消息，使用#define WM_ONTRAY WM_USER + 1
	m_notify.hIcon = m_hIcon;//显示在系统托盘上的图标

	TCHAR lpszTip[] = _T("shellicon");
	lstrcpyn(m_notify.szTip, lpszTip, sizeof(lpszTip));//图标上的提示字符串

	Shell_NotifyIcon(NIM_ADD, &m_notify);// 向任务栏的状态栏发送添加托盘图标的消息

	m_bAddShellIcon = TRUE;
}

LRESULT CWifiDirectDlg::OnNotifyMsg(WPARAM wparam, LPARAM lparam)
{
	//wParam接收的是图标的ID，而lParam接收的是鼠标的行为   
	if (wparam != IDR_MAINFRAME)
		return    1;
	switch (lparam)
	{
	case  WM_RBUTTONUP://右键起来时弹出快捷菜单，这里只有一个“关闭”   
	{
		LPPOINT    lpoint = new    tagPOINT;
		::GetCursorPos(lpoint);//得到鼠标位置   
		CMenu menu;
		menu.CreatePopupMenu();//声明一个弹出式菜单   
		//增加菜单项“关闭”，点击则发送消息WM_DESTROY给主窗口（已   
		//隐藏），将程序结束
		//menu.AppendMenuW(MF_STRING, WM_USER_NOTIFYICON, LPCTSTR(_T("还原")));
		menu.AppendMenuW(MF_STRING, WM_DESTROY, LPCTSTR(_T("关闭")));
		//确定弹出式菜单的位置   
		menu.TrackPopupMenu(TPM_LEFTALIGN, lpoint->x, lpoint->y, this);
		//资源回收   
		HMENU    hmenu = menu.Detach();
		menu.DestroyMenu();
		delete lpoint;
	}
	break;
	case WM_LBUTTONDBLCLK://双击左键的处理   
	{
		this->ShowWindow(SW_SHOW);//简单的显示主窗口完事儿   
	}
	break;
	}
	return 0;
}


void CWifiDirectDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		// TODO: 在此添加命令处理程序代码
	try {
		if (isRunning)
		{
			_hostedNetwork.Stop();
			//WaitForSingleObjectEx(_apEvent.Get(), INFINITE, FALSE);
		}
	}
	catch (WlanHostedNetworkException exception)
	{
		const char* name = exception.what();

		//先获取转换成宽字符后的长度（一定要通过这种方式，先回去字符长度，因为有指定编码为CP_ACP，编码不同，长度也会不同）
		int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name, -1, NULL, 0);
		//声明一个宽字符类型变量，用于存放转换后的字符
		wchar_t* wname = new wchar_t[nLen];
		//利用微软ANSI转宽字符的函数（name:ANSI字符，wname:宽字符）
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name, -1, wname, nLen);
		MessageBox(wname);
	}
	
	//_hostedNetwork.~WlanHostedNetworkHelper();
	Shell_NotifyIcon(NIM_DELETE, &m_notify); //关闭时删除系统托盘图标
	//清理vector的内存
	connectList.clear();
	//不知道为啥关不掉，直接结束进程
	HANDLE MyProcess = GetCurrentProcess();
	TerminateProcess(MyProcess, 0);
	//MessageBox(LPCTSTR(_T("触发OnClose")));
	__super::OnClose();
}


void CWifiDirectDlg::ShowMessage(LPCTSTR message)
{
	// 显示消息
	GetDlgItem(IDC_STATIC4)->SetWindowTextW(message);
	//GetDlgItem(IDC_STATIC)->SetWindowTextW(LPCTSTR(_T("测试")));
	UpdateData(FALSE);
	GetDlgItem(IDC_STATIC4)->Invalidate();

}
