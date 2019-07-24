
// ChangeFileTimeDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CChangeFileTimeDlg dialog
class CChangeFileTimeDlg : public CDialogEx
{
// Construction
public:
	CChangeFileTimeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CHANGEFILETIME_DIALOG };

	protected:
	
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnDropFiles(HDROP hDropInfo);

	void ProcessFolder (CString strFolderName, bool change_modified, bool change_created, int &count_total, int &count_changed);
	void ProcessFolderCnt(CString strFolderName, int &count);
	void parseTime(CString sVal2, CTimeSpan& dtTime);

	bool ParseFixedDateTime(CString sTime, CTime& dtTime);
	bool ParseRelativeTime(CString sTime, CTime& dtTime);
	bool isEqualCertainDate(CTime dtTime1, CTime dtTime2);

	void InitControl_start(BOOL bInit );
	void InitControl_undo(BOOL bInit );
	void ProcessFile  (CString strName, bool change_modified, bool change_created, int &count_total, int &count_changed);
	void initLogView();
	BOOL checkInputParam();
	
	BOOL GetStatus(CString sFile, CFileStatus& status);
	BOOL SetStatus(CString sFile, CFileStatus& status);

	void processXap(CString strName, CTime dtModify);
	BOOL getTakenXap(CString strName, CStringA& sTaken);
	
	BOOL getTakenNormal(CString sFile, CStringA& sTaken );
	BOOL setTakenNormal(CString sFile, CStringA sTaken );
	
	void parseXapTime(CStringA sOrgTaken, CTime& dtTaken);
	void parseNormalTime(CStringA sOrgTaken, CTime& dtTaken);

	int findTag(byte* buf, int nLen, CStringA sTag, int& nPos);
	BOOL IsPhoto(CString sFile);

	void doEvent();
	void waitForResume();
	bool parseLog(CString sLine, CString& sFile, LONGLONG& lCreate, LONGLONG& lModify, LONGLONG& lAccess, CString& sXapTaken, CString& sDateTaken);
	void UnixTimeToFileTime(time_t t, LPFILETIME pft);

	CString getLogFileName();
	int getUndoCnt();

	CString m_sLogFile;
	CString m_sPath;
	CString m_sDiff;
	CString m_sDiffHour;
	bool m_bPause;
	CProgressCtrl m_ctlProgress;
	CTime m_dtCertainDate;
	CTime m_dtFixDate;
	CTimeSpan m_dtRelative;

	CStdioFile m_logFile;

	afx_msg void OnBnClickedBtnBrowser();
	CString m_sQuery;
	CComboBox m_cmbQuery;
	CComboBox m_cmbChange;
	CString m_sChangeTime;
	CString m_sFileType;
	CString m_sChangeType;
	afx_msg void OnBnClickedBtnPuse();
	afx_msg void OnBnClickedBtnUndo();
	CListCtrl m_lstLog;
	BOOL m_bChangeAccess;
	BOOL m_bChangeModify;
	BOOL m_bChangeCreate;
	BOOL m_bChangeTaken;
};
