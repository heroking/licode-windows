
#include "stdafx.h"
#include "MainFrame.h"
#include "VideoDialog.h"
#include "defaults.h"
#include "resource.h"
#include "func.h"
#include "VideoDialog.h"
#include "aboutMe.h"
#include "JoinRoom.h"

const TCHAR* const kLabel_title = _T("Label_Title");
const TCHAR* const kBtn_Mini = _T("minbtn");
const TCHAR* const kBtn_Max = _T("maxbtn");
const TCHAR* const kBtn_Restore = _T("restorebtn");
const TCHAR* const kBtn_Close = _T("closebtn");
const TCHAR* const kLabel_User = _T("Label_User");
const TCHAR* const kLabel_Tip = _T("Label_Tip");
const TCHAR* const kBtn_Join = _T("joinChannel");
const TCHAR* const kBtn_Left= _T("leftChannel");
const TCHAR* const kBtn_AboutMe = _T("aboutSystem");
const TCHAR* const kLayout_Local = _T("Layout_Local");
const TCHAR* const kLayout_Remote = _T("Layout_Remote");


MainFrame::MainFrame()
	: callback_(nullptr)
{
}

MainFrame::~MainFrame()
{
	PostQuitMessage(0);
}

HWND MainFrame::handle()
{
	return *this;
}

void MainFrame::RegisterObserver(MainWndCallback* callback)
{
	ASSERT(callback);
	callback_ = callback;
}

bool MainFrame::IsWindow()
{
	return ::IsWindow(*this);
}

void MainFrame::OnJoinChannel(const AString &room, UInt32 id)
{
	roomName_ = room;

#ifdef _UNICODE
	CString channelName = CA2W(room.c_str());
#else
	CString channelName = room.c_str();
#endif // _UNICODE

	PostNotification( _T("成功加入房间:") + channelName);

	cVideoDialog *dlg = new cVideoDialog(id);

	CVerticalLayoutUI *layout = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(kLayout_Local));
	if (layout)
	{
		layout->Add(dlg);
	}

	callback_->SetLocalRender(dlg);
}

void MainFrame::OnLeftChannel(const AString &room)
{
	CVerticalLayoutUI *layout = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(kLayout_Local));
	if (layout)
	{
		layout->RemoveAll();
	}

#ifdef _UNICODE
	CString channelName = CA2W(room.c_str());
#else
	CString channelName = room.c_str();
#endif // _UNICODE

	PostNotification(_T("成功离开房间:") + channelName);

	roomName_ = "";
}

void MainFrame::OnUserJoinChannel(const AString &room, UInt32 id)
{
#ifdef _UNICODE
	CString channelName = CA2W(room.c_str());
#else
	CString channelName = room.c_str();
#endif // _UNICODE

	CString strID; 
	strID.Format(_T("%d"), id);

	PostNotification(strID + _T(" 加入房间:") + channelName);

	cVideoDialog *dlg = new cVideoDialog(id);

	CTileLayoutUI *layout = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(kLayout_Remote));
	if (layout)
	{
		layout->Add(dlg);
	}

	m_Dlgs[id] = dlg;

	callback_->SetRemoteRender(id,dlg);
}

void MainFrame::OnUserLeftChannel(const AString &room, UInt32 id)
{
#ifdef _UNICODE
	CString channelName = CA2W(room.c_str());
#else
	CString channelName = room.c_str();
#endif // _UNICODE

	CString strID;
	strID.Format(_T("%d"), id);

	PostNotification(strID + _T(" 离开房间:") + channelName);

	callback_->SetRemoteRender(id, nullptr);

	CTileLayoutUI *layout = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(kLayout_Remote));
	if (layout)
	{
		layout->Remove(m_Dlgs[id]);
	}

	m_Dlgs.erase(id);
}

void MainFrame::MessageBox(const char* caption, const char* text, bool is_error)
{

}

void MainFrame::QueueUIThreadCallback(int msg_id, void* data)
{
	::PostMessage(*this, UI_THREAD_CALLBACK, static_cast<WPARAM>(msg_id), reinterpret_cast<LPARAM>(data));
// 	::PostThreadMessage(ui_thread_id_, UI_THREAD_CALLBACK,
// 		static_cast<WPARAM>(msg_id), reinterpret_cast<LPARAM>(data));

}

LPCTSTR MainFrame::GetWindowClassName() const
{
	return _T("launcher");
}

CControlUI* MainFrame::CreateControl(LPCTSTR pstrClass)
{
	return NULL;
}

void MainFrame::OnFinalMessage(HWND hWnd)
{
	if (callback_) {
		callback_->DisconnectFromServer();
	}

	if (callback_)
		callback_->Close();

	m_TrayIcon.RemoveIcon();

	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString MainFrame::GetSkinFile()
{
	return _T("UI.xml");
}

CDuiString MainFrame::GetSkinFolder()
{
	return  _T("skin\\");
}

LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if (wParam == SC_CLOSE) {
		::PostQuitMessage(0L);
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if (::IsZoomed(*this) != bZoomed)
	{
		if (!bZoomed) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(kBtn_Max));
			if (pControl) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(kBtn_Restore));
			if (pControl) pControl->SetVisible(true);
		}
		else 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(kBtn_Max));
			if (pControl) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(kBtn_Restore));
			if (pControl) pControl->SetVisible(false);
		}
	}
	return lRes;
}

// LRESULT MainFrame::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
// {
// 	MONITORINFO oMonitor = {};
// 	oMonitor.cbSize = sizeof(oMonitor);
// 	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
// 	CDuiRect rcWork = oMonitor.rcWork;
// 	rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);
// 
// 	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
// 	lpMMI->ptMaxPosition.x = rcWork.left;
// 	lpMMI->ptMaxPosition.y = rcWork.top;
// 	lpMMI->ptMaxSize.x = rcWork.right;
// 	lpMMI->ptMaxSize.y = rcWork.bottom;
// 
// 	bHandled = FALSE;
// 	return 0;
// }
// 
// 
// LRESULT MainFrame::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
// {
// 	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
// 	::ScreenToClient(*this, &pt);
// 
// 	RECT rcClient;
// 	::GetClientRect(*this, &rcClient);
// 
// 	// 		if( !::IsZoomed(*this) ) {
// 	// 			RECT rcSizeBox = m_pm.GetSizeBox();
// 	// 			if( pt.y < rcClient.top + rcSizeBox.top ) {
// 	// 				if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
// 	// 				if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
// 	// 				return HTTOP;
// 	// 			}
// 	// 			else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
// 	// 				if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
// 	// 				if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
// 	// 				return HTBOTTOM;
// 	// 			}
// 	// 			if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
// 	// 			if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
// 	// 		}
// 
// 	RECT rcCaption = m_PaintManager.GetCaptionRect();
// 	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
// 		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
// 		CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(pt));
// 		if (pControl && _tcscmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0 &&
// 			_tcscmp(pControl->GetClass(), DUI_CTR_OPTION) != 0 &&
// 			_tcscmp(pControl->GetClass(), DUI_CTR_TEXT) != 0)
// 			return HTCAPTION;
// 	}
// 
// 	return HTCLIENT;
// }

LRESULT MainFrame::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return FALSE;
}

void MainFrame::OnTimer(TNotifyUI& msg)
{
}

void MainFrame::OnExit(TNotifyUI& msg)
{
	Close();
}

void MainFrame::InitWindow()
{
	ui_thread_id_ = ::GetCurrentThreadId();

	CLabelUI *label_User = static_cast<CLabelUI*>(m_PaintManager.FindControl(kLabel_User));
	if (label_User)
	{
		CString strTitle;
#ifndef _UNICODE
		strTitle += GetPeerName();
#else
		strTitle += CA2W(GetPeerName().c_str());
#endif
		label_User->SetText(strTitle);
	}

	pBtnJoin = static_cast<CButtonUI*>(m_PaintManager.FindControl(kBtn_Join));

	CTileLayoutUI *layout = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(kLayout_Remote));
	if (layout)
	{
		layout->SetItemSize(CSize(242, 210));
	}
	

	HICON hAppIcon = ::LoadIcon(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
	m_TrayIcon.AddIcon(m_hWnd, WM_TRAYICON_NOTIFY, 1, hAppIcon, _T("一对多音视频"));

	//::SetWindowLong(*this, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
}

void MainFrame::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, _T("windowinit")) == 0)
	{

	}
	else if (_tcsicmp(msg.sType, _T("killfocus")) == 0)
	{

	}
	else if (_tcsicmp(msg.sType, _T("click")) == 0)
	{
		CDuiString name = msg.pSender->GetName();
		if (_tcsicmp(name, kBtn_Close) == 0)
		{
			OnExit(msg);
		}
		else if (_tcsicmp(name, kBtn_Join) == 0)
		{
			doJoinRoom();
		}
		else if(_tcsicmp(name, kBtn_Left) == 0)
		{
			doLeftRoom();
		}
		else if (_tcsicmp(name, kBtn_AboutMe) == 0)
		{
			doAboutMe();
		}
		else if (name == kBtn_Mini) 
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if (name == kBtn_Max) 
		{
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
		else if (name == kBtn_Restore)
		{
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		else if (name == _T("Android")|| name == _T("iPhone") || name == _T("Mac") || name == _T("Web") )
		{
			::MessageBox(*this, msg.pSender->GetText()+_T("正在开发中，敬请期待！"), _T("提示"), MB_OK);
		}
	}
	else if (msg.sType == _T("setfocus"))
	{
		
	}
	else if (msg.sType == _T("itemclick"))
	{		
		//int id = msg.pSender->GetTag();
	}
	else if (msg.sType == _T("itemactivate"))
	{
		//int id = msg.pSender->GetTag();
	}
}

LRESULT MainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (uMsg)
	{
	case WM_TRAYICON_NOTIFY:
	{
		m_TrayIcon.OnTrayIconNotify(wParam, lParam);
		OnTrayMessage(wParam, lParam);
		bHandled = TRUE;
		break;
	}	
	case WM_NETWORK:
	{
		callback_->OnNetwork(wParam, lParam);
		bHandled = TRUE;
		break;
	}
	case WM_TIMER:
	{
		OnTimer(wParam, lParam);
		bHandled = TRUE;
		break;
	}	
	case WM_COMMAND:
	{
		OnCommand(wParam, lParam);
		bHandled = false;
		break;
	}
	case  UI_THREAD_CALLBACK:
	{
		callback_->UIThreadCallback(static_cast<int>(wParam),
			reinterpret_cast<void*>(lParam));
		bHandled = false;
		break;
	}
	default:
		break;
	}
	return 0;
}

UILIB_RESOURCETYPE MainFrame::GetResourceType() const
{
	return UILIB_ZIPRESOURCE;
}

LPCTSTR MainFrame::GetResourceID() const
{
	return MAKEINTRESOURCE(IDR_ZIPRES);
}

LRESULT MainFrame::OnTimer(WPARAM wParam, LPARAM lParam)
{
	m_TrayIcon.OnTimer(wParam);

	return S_OK;
}

//托盘菜单函数
LRESULT MainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_TRAYCLOSE)
	{
		::PostMessage(*this, WM_CLOSE, NULL, NULL);
	}
	else if (wParam == ID_OPENWEB)
	{
		::ShellExecute(*this, _T("open"), WEBURL, NULL, NULL, SW_SHOW);
	}
	return 0;
}

//系统托盘函数
LRESULT MainFrame::OnTrayMessage(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONDBLCLK)	//双击事件
	{
		::ShowWindow(*this, SW_SHOW);
		::SetWindowPos(*this, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	else if (lParam == WM_RBUTTONDOWN)	//右键单击事件
	{
		HMENU hMenu = ::LoadMenu(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(IDR_TRAYMENU));
		hMenu = ::GetSubMenu(hMenu, 0);
		CPoint Curpt;
		::GetCursorPos(&Curpt);			//获取鼠标指针位置
		::SetForegroundWindow(*this);	// 修正当用户按下ESCAPE 键或者在菜单之外单击鼠标时菜单不会消失的情况
		::TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, Curpt.x, Curpt.y, 0, this->handle(), NULL);
	}
	return 0;
}

void MainFrame::doJoinRoom()
{
	JoinRoom *joinDlg = new JoinRoom;
	joinDlg->Create(this->m_hWnd, _T("AboutMe"), UI_WNDSTYLE_DIALOG, UI_WNDSTYLE_EX_FRAME);
	joinDlg->CenterWindow();
	if (joinDlg->ShowModal() == IDOK)
	{
		AString server = joinDlg->getServer();
		AString roomName = joinDlg->getRoomName();
		
		if (!roomName.empty() && !server.empty())
		{
			callback_->JoinChannel(server,roomName);
		}
	}

	delete joinDlg;
	joinDlg = nullptr;

	PostNotification(_T(""));
}

void MainFrame::doLeftRoom()
{
	if (!roomName_.empty())
	{
		callback_->LeaveChannel(roomName_);
	}	
}

void MainFrame::doAboutMe()
{
	AboutMe *me = new AboutMe;
	me->Create(this->m_hWnd, _T("AboutMe"), UI_WNDSTYLE_DIALOG, UI_WNDSTYLE_EX_FRAME);
	me->CenterWindow();
	me->ShowModal();
}

void MainFrame::PostNotification(CString msg, COLORREF color/* = RGB(0, 0, 0)*/)
{
	CLabelUI *label_tip = static_cast<CLabelUI*>(m_PaintManager.FindControl(kLabel_Tip));
	if (label_tip)
	{
		label_tip->SetText(msg);
		label_tip->SetTextColor(color);
	}
}

void MainFrame::OnDisconnected()
{

}

void MainFrame::PacketBufferFull()
{
	PostNotification("缓冲区已满！");
}

void MainFrame::PacketError(UInt32)
{
	PostNotification("数据包异常！");
}

void MainFrame::PacketUnknown(UInt32)
{
	PostNotification("未知数据！");
}

void MainFrame::OnSignedIn()
{

}
