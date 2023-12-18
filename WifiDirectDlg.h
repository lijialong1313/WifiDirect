
// WifiDirectDlg.h: 头文件
//

#pragma once
#include "WlanHostedNetworkWinRT.h"
#include "DeviceItem.h"
#include <vector>
using namespace std;
#define WM_USER_NOTIFYICON WM_USER+1

// CWifiDirectDlg 对话框
class CWifiDirectDlg : public CDialogEx , public IWlanHostedNetworkListener, public IWlanHostedNetworkPrompt
{
// 构造
public:
	CWifiDirectDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WIFIDIRECT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// Wifi的SSID
	CEdit SSID;
	// Wifi的密码
	CEdit PASS;
	// 显示密码按钮
	CButton ShowPasswordButton;
	afx_msg void OnShowPasswordClick();
	bool isShowPassword = false;
	bool isRunning = false;
	WlanHostedNetworkHelper _hostedNetwork;
	Microsoft::WRL::Wrappers::Event _apEvent;
	afx_msg void OnWifiClick();
	CButton WifiButton;
	bool m_bAddShellIcon;
	NOTIFYICONDATA m_notify;
	vector<Device> connectList;
public:
	virtual void OnDeviceConnected(std::wstring deviceId, std::wstring remoteHostAddress) override;
	virtual void OnDeviceDisconnected(std::wstring deviceId) override;

	virtual void OnAdvertisementStarted() override;
	virtual void OnAdvertisementStopped(std::wstring message) override;
	virtual void OnAdvertisementAborted(std::wstring message) override;

	virtual void OnAsyncException(std::wstring message) override;

	virtual void LogMessage(std::wstring message) override;

	// IWlanHostedNetworkPrompt Implementation

	virtual bool AcceptIncommingConnection() override;
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void AddShellIcon();

	afx_msg LRESULT OnNotifyMsg(WPARAM wparam, LPARAM lparam);
	afx_msg void OnClose();
	void ShowMessage(LPCTSTR message);
	// 连接显示列表
	CListCtrl listCtrl;
};
