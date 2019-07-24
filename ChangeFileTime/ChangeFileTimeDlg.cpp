
// Date & time batch changer for photos and other files
// Developed for a Code Project article
// ©2014 Michael Haephrati, haephrati@gmail.com


#include "stdafx.h"
#include <share.h>
#include <shlwapi.h>
#include "ChangeFileTime.h"
#include "ChangeFileTimeDlg.h"
#include "EXIF.h"

#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define TITLE L"Date & Time Batch Changer ©2014 Michael Haephrati"

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChangeFileTimeDlg dialog



CChangeFileTimeDlg::CChangeFileTimeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChangeFileTimeDlg::IDD, pParent)
	, m_sPath(_T(""))
	, m_sDiff(_T(""))
	, m_sDiffHour(_T(""))
	, m_sQuery(_T(""))
	, m_sChangeTime(_T(""))
	, m_sFileType(_T(""))
	, m_sChangeType(_T(""))
	, m_bChangeAccess(TRUE)
	, m_bChangeModify(TRUE)
	, m_bChangeCreate(TRUE)
	, m_bChangeTaken(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChangeFileTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_PATH, m_sPath);
	DDX_Control(pDX, IDC_PROGRESS, m_ctlProgress);
	DDX_Text(pDX, IDC_EDT_QUERY, m_sQuery);
	DDX_Control(pDX, IDC_CMB_QUERY, m_cmbQuery);
	DDX_Control(pDX, IDC_CMB_CHANGE, m_cmbChange);
	DDX_Text(pDX, IDC_EDT_TIME, m_sChangeTime);
	DDX_CBString(pDX, IDC_CMB_QUERY, m_sFileType);
	DDX_CBString(pDX, IDC_CMB_CHANGE, m_sChangeType);
	DDX_Control(pDX, IDC_LST_LOG, m_lstLog);
	DDX_Check(pDX, IDC_CHEK_ACC, m_bChangeAccess);
	DDX_Check(pDX, IDC_CHK_MODIFY, m_bChangeModify);
	DDX_Check(pDX, IDC_CHK_CREATE, m_bChangeCreate);
	DDX_Check(pDX, IDC_CHK_TAKEN, m_bChangeTaken);
}

BEGIN_MESSAGE_MAP(CChangeFileTimeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, &CChangeFileTimeDlg::OnBnClickedBtnStart)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BTN_BROWSER, &CChangeFileTimeDlg::OnBnClickedBtnBrowser)
	ON_BN_CLICKED(IDC_BTN_PUSE, &CChangeFileTimeDlg::OnBnClickedBtnPuse)
	ON_BN_CLICKED(IDC_BTN_UNDO, &CChangeFileTimeDlg::OnBnClickedBtnUndo)
END_MESSAGE_MAP()


#define TYPE_CERTAIN_DATE	_T("Certain Date")
#define TYPE_CONTAIN_STRING _T("Contain string")
#define TYPE_FILE_TYPE		_T("File Type")
#define TYPE_ALL_FILES		_T("All Files")

#define TIME_FIX			_T("Fixed Date")
#define TIME_RELATIVE		_T("Relative Date")


// CChangeFileTimeDlg message handlers
BOOL CChangeFileTimeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	DragAcceptFiles(TRUE);

	// Initilize combo box for search query
	m_cmbQuery.AddString(TYPE_CERTAIN_DATE);
	m_cmbQuery.AddString(TYPE_CONTAIN_STRING);
	m_cmbQuery.AddString(TYPE_FILE_TYPE);
	m_cmbQuery.AddString(TYPE_ALL_FILES);
	m_cmbQuery.SetCurSel(0);

	//Initilize combo box for types of date/time values, which are either fixed or relative
	m_cmbChange.AddString(TIME_FIX);
	m_cmbChange.AddString(TIME_RELATIVE);
	m_cmbChange.SetCurSel(0);

	m_sChangeType = TIME_RELATIVE;
	m_sFileType = TYPE_ALL_FILES;
	UpdateData(FALSE);
	InitControl_start(TRUE);

	m_sLogFile = getLogFileName();

	initLogView();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChangeFileTimeDlg::initLogView()
{
	
	m_lstLog.InsertColumn(0, _T("File"),0, 230);
	m_lstLog.InsertColumn(1, _T("old date/time"),0, 100);
	m_lstLog.InsertColumn(2, _T("new date/time"),0, 100);
	m_lstLog.InsertColumn(3, _T("result"),0, 50);
}

CString CChangeFileTimeDlg::getLogFileName()
{

	TCHAR buff[MAX_PATH];
	memset(buff, 0, MAX_PATH);
	::GetModuleFileName(NULL,buff,sizeof(buff));    
	CString sFile = buff;
	sFile = sFile + _T(".log");
	return sFile;
}

void CChangeFileTimeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChangeFileTimeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChangeFileTimeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
// ParseRelativeTime - Parses the requested relative change from user's input
bool CChangeFileTimeDlg::ParseRelativeTime(CString sTime, CTime& dtTime)
{
	CString sVal = sTime;
	sVal.Replace(_T("/"), _T(" "));
	sVal.Replace(_T(":"), _T(" "));
	CArray<CString,CString> v;
	CString field;
	int index = 0;
	while (AfxExtractSubString(field,sVal,index,_T(' ')))
	{
		v.Add(field);
		++index;
	}

	if ( v.GetCount() != 3 )
	{
		return false;
	}

	CTime dtCertain(_ttoi(v[0]), _ttoi(v[1]), _ttoi(v[2]), 0, 0, 0);
	dtTime = dtCertain;
}
// ParseRelativeTime - Parses the requested fixed date and time from user's input
bool CChangeFileTimeDlg::ParseFixedDateTime(CString sTime, CTime& dtTime)
{
	CString sVal = sTime;
	sVal.Replace(_T("/"), _T(" "));
	sVal.Replace(_T(":"), _T(" "));
	CArray<CString,CString> v;
	CString field;
	int index = 0;
	while (AfxExtractSubString(field,sVal,index,_T(' ')))
	{
		v.Add(field);
		++index;
	}
	// This part is to make things easier and a bit more user friendly
	if (v.GetCount() == 5) // Only date was entered with an hour and minutes but without seconds
	{
		v.Add((CString)(L"00"));	// Seconds
		++index;
	}
	else
	if (v.GetCount() == 4) // Only date was entered with an hour but without minutes and seconds
	{
		v.Add((CString)(L"00"));	// Minutes
		v.Add((CString)(L"00"));	// Seconds
		index += 2;
	}
	else
	if (v.GetCount() == 3) // Only date was entered without any time
	{
		v.Add((CString)(L"00"));	// Hours
		v.Add((CString)(L"00"));	// Minutes
		v.Add((CString)(L"00"));	// Seconds
		index += 3;
	}
	if ( v.GetCount() != 6 ) // I give up, date/time entered incorrectly
	{
		return false;
	}

	CTime dtCertain(_ttoi(v[0]), _ttoi(v[1]), _ttoi(v[2]), _ttoi(v[3]), _ttoi(v[4]), _ttoi(v[5]));
	dtTime = dtCertain;
}

void CChangeFileTimeDlg::InitControl_start(BOOL bInit)
{
	GetDlgItem(IDC_BTN_START)->EnableWindow(bInit);
	GetDlgItem(IDC_BTN_PUSE)->EnableWindow(!bInit);
}

void CChangeFileTimeDlg::InitControl_undo(BOOL bInit)
{
	GetDlgItem(IDC_BTN_UNDO)->EnableWindow(bInit);
	GetDlgItem(IDC_BTN_PUSE)->EnableWindow(!bInit);
}

BOOL CChangeFileTimeDlg::checkInputParam()
{

	m_sQuery.MakeLower();
	
	if ( m_sPath == "" )
	{
		MessageBox(_T("Please input Folder Path"), TITLE);
		InitControl_start(TRUE);
		m_logFile.Close();
		return FALSE;
	}

	if ( m_sChangeTime == "" )
	{
		MessageBox(_T("Please input Change Time"), TITLE);
		InitControl_start(TRUE);
		m_logFile.Close();
		return FALSE;
	}

	if ( m_sQuery == "" && m_sFileType != TYPE_ALL_FILES)
	{
		MessageBox(_T("Please input Query"), TITLE);
		InitControl_start(TRUE);
		m_logFile.Close();
		return FALSE;
	}

	if ( m_sFileType == TYPE_CERTAIN_DATE )
	{
		if ( !ParseRelativeTime(m_sQuery, m_dtCertainDate) )
		{
			MessageBox(_T("Incorrect fixed date time. See example : 2013/10/05"), TITLE);
			InitControl_start(TRUE);
			m_logFile.Close();
			return FALSE;
		}
	}

	if ( m_sChangeType == TIME_FIX )
	{
		if ( !ParseFixedDateTime(m_sChangeTime, m_dtFixDate) )
		{
			MessageBox(_T("Incorrect relative requested time change See example : 2013/10/05 10:59:04"), TITLE);
			InitControl_start(TRUE);
			m_logFile.Close();
			return FALSE;
		}
	}

	if ( !m_bChangeAccess && !m_bChangeCreate && !m_bChangeModify && !m_bChangeTaken )
	{
		MessageBox(_T("Please check date type"), TITLE);
			InitControl_start(TRUE);
			m_logFile.Close();
			return FALSE;
	}
	
	if ( m_sChangeType == TIME_RELATIVE )
	{
		CTimeSpan dtDiff(0, 0, _ttof(m_sChangeTime) * 60, 0);
		parseTime(m_sChangeTime, dtDiff);

		m_dtRelative = dtDiff;
	}


	return TRUE;
}

void CChangeFileTimeDlg::parseTime(CString sVal2, CTimeSpan& dtTime)
{
	CString sVal = sVal2;

	CArray<CString,CString> v;
	CString field;
	int index = 0;
	
	while (AfxExtractSubString(field,sVal,index,_T(':')))
	{
		v.Add(field);
		++index;
	}
	int nH = 0;
	int nM = 0;
	int nS = 0;
	if ( v.GetCount() > 0 )
		nH = _ttoi(v[0]);
	if ( v.GetCount() > 1 )
		nM = _ttoi(v[1]);
	if ( v.GetCount() > 2 )
		nS = _ttoi(v[2]);

	CTimeSpan dtSpan(0, nH, nM, nS);
	dtTime = dtSpan;
}

void CChangeFileTimeDlg::OnBnClickedBtnStart()
{
	//////////////////////////////////////////////////////////////////////////
	//Init log file1
	
	if (::PathFileExists(m_sLogFile)  && !::DeleteFile(m_sLogFile) )
	{
		MessageBox(_T("Please close log file"), TITLE);
		return;
	}

	m_logFile.Open(m_sLogFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
	
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//Check input value
	int count_total=0; // if a folder was dropped or selected
	int count_changed = 0;
	UpdateData();

	if ( ! checkInputParam() )
		return ;

	//////////////////////////////////////////////////////////////////////////
	//Process
	m_bPause = false;
	InitControl_start(FALSE);
	m_lstLog.DeleteAllItems();

	ProcessFolderCnt(m_sPath, count_total); // Get total count of files.
	m_ctlProgress.SetRange(0, count_total); // set progress

	count_total = 0;
	WIN32_FILE_ATTRIBUTE_DATA w32fad;
	ZeroMemory (&w32fad, sizeof(w32fad));

	if (::GetFileAttributesEx (m_sPath, GetFileExInfoStandard, &w32fad))
	{
		// **BG** Changed to '&' instead of '==' because of possible FILE_ATTRIBUTE_NOT_CONTENT_INDEXED attribute
		if (w32fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			ProcessFolder(m_sPath, true, true, count_total, count_changed);
		}
		else
		{
			ProcessFile(m_sPath, true, true, count_total, count_changed);
		}

		CString sMsg;
		sMsg.Format(_T("%d files changed!"), count_changed);
		
	}

	InitControl_start(TRUE);
	m_logFile.Close();
}

void CChangeFileTimeDlg::ProcessFolderCnt(CString strFolderName, int &count)
{
	strFolderName += _T("\\*");
	CStringArray filenames;

	CFileFind files;
	if(files.FindFile(strFolderName)) 
	{
		while (true)
		{
			BOOL bMoreFiles = files.FindNextFile ();
			if (!files.IsDirectory ())
			{
				count++;
			}
			else{
				if (!files.IsDots()){
					ProcessFolderCnt(files.GetFilePath(), count);
				}
			}
			if (! bMoreFiles)
				break;
		}
	}
}

void CChangeFileTimeDlg::ProcessFolder (CString strFolderName, bool change_modified, bool change_created, int &count_total, int &count_changed)
{

	strFolderName += _T("\\*");
	CStringArray filenames;

	CFileFind files;

	ProcessFile(strFolderName, change_modified, change_created, count_total, count_changed);

	if(files.FindFile(strFolderName)) 
	{
		while (true)
		{
			BOOL bMoreFiles = files.FindNextFile ();
			if (!files.IsDirectory ())
			{
				ProcessFile(files.GetFilePath(), change_modified, change_created, count_total, count_changed);
			}
			else{
				if (!files.IsDots()){
					ProcessFolder(files.GetFilePath(), change_modified, change_created, count_total, count_changed);
				}
			}
			if (! bMoreFiles)
				break;
		}
	}
}


void CChangeFileTimeDlg::doEvent()
{
	MSG msg;
	if(PeekMessage(&msg, AfxGetMainWnd()->m_hWnd,
		0, 0, PM_REMOVE))
	{ /* handle it*/
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} /* handle it */
}

void CChangeFileTimeDlg::waitForResume()
{
	while (m_bPause)
	{
		Sleep(100);
		doEvent();
	}
}

//return 1 : find tag
//return 2 : find tag but require data
//return 0 : not find.
int CChangeFileTimeDlg::findTag(byte* buf, int nLen, CStringA sTag, int& nPos)
{
	int nCnt = 0;
	for ( int i = 0; i < nLen; i ++ )
	{
		if ( nCnt == sTag.GetLength() )
		{
			if ( i < nLen - 20 )
			{
				if ( buf[i] == _T('=') )
					nPos = i+1;
				else
					nPos = i;
				return 1;//find
			}
			return 2;
		}
		if ( sTag.GetAt(nCnt) == _T('?'))
		{
			nCnt++;
			continue;
		}

		if ( buf[i] == sTag.GetAt(nCnt) )
		{
			nCnt ++;
			continue;
		}
		else
			nCnt = 0;
	}

	if ( nCnt > 0 )
		return 2; // require data

	return 0; //not find
}

BOOL CChangeFileTimeDlg::getTakenXap(CString strName, CStringA& sTaken)
{
	CFile file (strName, CFile::modeReadWrite);
	
	byte buf[4096];
	bool bEnd = false;
	int nRet;
	int nRead;
	int nPos = 0;
	char szTaken[20];
	while (!bEnd)
	{
		memset(buf, 0, sizeof(buf));
		nRead = file.Read(buf, sizeof(buf));
		if ( nRead != 4096 )
			bEnd = true;
		nRet = findTag(buf,nRead,"x?p:CreateDate", nPos);
		if ( nRet == 0 ) //not found
		{
		}

		if ( nRet == 1 )// find tag
			break;
		
		if ( nRet == 2 )// find tag and require 
		{
			file.Seek(-100, CFile::current);
		}
	}

	if ( nRet == 1 )
	{
		
		file.Seek(nPos - nRead+1, CFile::current);
		file.Read(szTaken, 20);
		szTaken[19] = 0;
		sTaken= szTaken;
		file.Close();
		return TRUE;
	}
	
	file.Close();
	return FALSE;
}

void CChangeFileTimeDlg::processXap(CString strName, CTime dtTaken)
{
	CFile file (strName, CFile::modeReadWrite);
	
	byte buf[4096];
	bool bEnd = false;
	int nRet;
	int nRead;
	int nPos = 0;
	while (!bEnd)
	{
		memset(buf, 0, sizeof(buf));
		nRead = file.Read(buf, sizeof(buf));
		if ( nRead != 4096 )
			bEnd = true;
		nRet = findTag(buf,nRead,"x?p:CreateDate", nPos);
		if ( nRet == 0 ) //not found
		{
		}

		if ( nRet == 1 )// find tag
			break;
		
		if ( nRet == 2 )// find tag and require 
		{
			file.Seek(-100, CFile::current);
		}
	}

	if ( nRet == 1 )
	{
		file.Seek(nPos - nRead+1, CFile::current);
		CStringA sTaken;
		sTaken.Format("%.4d-%.2d-%.2dT%.2d:%.2d:%.2d", dtTaken.GetYear(), dtTaken.GetMonth(), dtTaken.GetDay(), dtTaken.GetHour(), dtTaken.GetMinute(), dtTaken.GetSecond());
		file.Write(sTaken.GetBuffer(), sTaken.GetLength());
	}

	file.Close();
}
// IsPhoto - returns TRUE if the file extention indicates it is a photo file.
BOOL CChangeFileTimeDlg::IsPhoto(CString sFile)
{
	int len_fname=sFile.GetLength();
	
	return ((len_fname > 3 && (_T(".jpg") == sFile.MakeLower().Right(4)))
		|| (len_fname > 4 && (_T(".jpeg") == sFile.MakeLower().Right(5)))
		|| (len_fname > 3 && (_T(".nef") == sFile.MakeLower().Right(4))));

	
}

BOOL CChangeFileTimeDlg::getTakenNormal(CString sFile, CStringA& sTaken )
{
#ifdef _UNICODE
		FILE* hFile=_wfsopen(sFile,_T("r+b"), _SH_DENYNO);
#else
		FILE* hFile=_fsopen(sFile,"r+b", _SH_DENYNO);
#endif
		if (hFile){
			EXIFINFO exifinfo;
			memset(&exifinfo,0,sizeof(EXIFINFO));
			Cexif exif(&exifinfo);
			bool decode_success = false;
			decode_success = exif.DecodeExif(hFile, NULL, false);
			fclose(hFile);
			if ( decode_success )
				sTaken = exifinfo.DateTime;
			return TRUE;
		}
		return FALSE;
}

BOOL CChangeFileTimeDlg::setTakenNormal(CString sFile, CStringA sTaken )
{
#ifdef _UNICODE
		FILE* hFile=_wfsopen(sFile,_T("r+b"), _SH_DENYNO);
#else
		FILE* hFile=_fsopen(sFile,"r+b", _SH_DENYNO);
#endif
		if (hFile){
			EXIFINFO exifinfo;
			memset(&exifinfo,0,sizeof(EXIFINFO));
			Cexif exif(&exifinfo);
			bool decode_success = false;
			decode_success = exif.DecodeExif(hFile, sTaken.GetBuffer(), true);
			fclose(hFile);
			if ( decode_success )
				return TRUE;
		}
		return FALSE;
}

void CChangeFileTimeDlg::parseXapTime(CStringA sOrgTaken, CTime& dtTaken)
{
	CString sVal = CA2W(sOrgTaken);
	sVal.Replace(_T("-"), _T(" "));
	sVal.Replace(_T(":"), _T(" "));
	sVal.Replace(_T("T"), _T(" "));

	CArray<CString,CString> v;
	CString field;
	int index = 0;
	
	while (AfxExtractSubString(field,sVal,index,_T(' ')))
	{
		v.Add(field);
		++index;
	}
	CTime dtTime(_ttoi(v[0]), _ttoi(v[1]), _ttoi(v[2]), _ttoi(v[3]), _ttoi(v[4]), _ttoi(v[5]) );
	dtTaken = dtTime;
}

void CChangeFileTimeDlg::parseNormalTime(CStringA sOrgTaken, CTime& dtTaken)
{

}

BOOL CChangeFileTimeDlg::GetStatus(CString sFile, CFileStatus& status)
{
	HANDLE hFile;
	CString sFilePath = sFile;
	if ( sFile.Right(2) == _T("\\*"))
		sFilePath.Delete(sFilePath.GetLength()-1, 1);

	hFile = CreateFile(sFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	 if(hFile == INVALID_HANDLE_VALUE)
		 return FALSE;
	 FILETIME ftCreate, ftModify, ftAccess;
	 if ( !GetFileTime(hFile,  &ftCreate, &ftAccess, &ftModify) )
	 {
		 CloseHandle(hFile);
		 return FALSE;
	 }
	 status.m_ctime = CTime(ftCreate);
	 status.m_mtime = CTime(ftModify);
	 status.m_atime = CTime(ftAccess);

	 CloseHandle(hFile);
	return TRUE;
}

void CChangeFileTimeDlg::UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}
	
BOOL CChangeFileTimeDlg::SetStatus(CString sFile, CFileStatus& status)
{
	HANDLE hFile;		// File name without full path
	CString sFilePath = sFile;
	if ( sFile.Right(2) == _T("\\*"))
		sFilePath.Delete(sFilePath.GetLength()-1, 1); // Generate file name from full path

	// Open file 
	hFile = CreateFile(sFilePath, GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	 if(hFile == INVALID_HANDLE_VALUE)
		 return FALSE;


	 SYSTEMTIME stCreate, stModify, stAccess;
	 FILETIME ftCreate, ftModify, ftAccess;
	 FILETIME ftCreate2, ftModify2, ftAccess2;

	 // Taking into account the time zone and day light saving
	 TIME_ZONE_INFORMATION tzi;
	 DWORD dwRet = GetTimeZoneInformation(&tzi);
	 
	 CTime ctCreate(status.m_ctime.GetTime() - tzi.DaylightBias*60);
	 CTime ctModify(status.m_mtime.GetTime() - tzi.DaylightBias*60);
	 CTime ctAccess(status.m_atime.GetTime() - tzi.DaylightBias*60);

	 ctCreate.GetAsSystemTime(stCreate);
	 ctModify.GetAsSystemTime(stModify);
	 ctAccess.GetAsSystemTime(stAccess);
	 
	 SystemTimeToFileTime(&stCreate, &ftCreate);
	 SystemTimeToFileTime(&stModify, &ftModify);
	 SystemTimeToFileTime(&stAccess, &ftAccess);
	 

	 LocalFileTimeToFileTime(&ftCreate, &ftCreate2);
	 LocalFileTimeToFileTime(&ftModify, &ftModify2);
	 LocalFileTimeToFileTime(&ftAccess, &ftAccess2);

     if ( !SetFileTime(hFile,&ftCreate2,&ftAccess2,&ftModify2) )
	 {
		 // Failed to change date/time for file
		 CloseHandle(hFile);
		 return FALSE;
	 }
	 // Success
	 CloseHandle(hFile);
	return TRUE;
}

bool CChangeFileTimeDlg::isEqualCertainDate(CTime dtTime1, CTime dtTime2)
{
	if ( dtTime1.GetYear() == dtTime2.GetYear() && dtTime1.GetMonth() == dtTime2.GetMonth() && dtTime1.GetDay() == dtTime2.GetDay())
		return true;
	return false;
}

void CChangeFileTimeDlg::ProcessFile  (CString strName, bool change_modified, bool change_created, int &count_total, int &count_changed)
{
	doEvent();

	if ( m_bPause )
	{
		waitForResume();
	}

	m_ctlProgress.SetPos(m_ctlProgress.GetPos() +1);
	strName.MakeLower();


	
	CFileStatus status;
	CTime dtCreate;
	CTime dtModify;
	CTime dtAccess;
	CTime dtTaken;
	CTime dtShowOld;
	CTime dtShowNew;

	if ( !GetStatus(strName, status) )
		return;


	//Check File Type
	if ( m_sFileType == TYPE_CONTAIN_STRING || m_sFileType == TYPE_FILE_TYPE)
	{
		if ( strName.Find(m_sQuery) == -1 )
			return;
	}

	//Check CertainDate
	if ( m_sFileType == TYPE_CERTAIN_DATE &&  !isEqualCertainDate(status.m_ctime , m_dtCertainDate) )
	{
		return;
	}

	dtShowOld = status.m_ctime;
	
	if ( m_bChangeAccess )
		dtShowOld = status.m_atime;
	if ( m_bChangeModify )
		dtShowOld = status.m_mtime;
	if ( m_bChangeCreate )
		dtShowOld = status.m_ctime;
	
	
	//Get Date Taken ORG
	CStringA sOrgTaken = "";
	bool bJpgXap = true;
	bool bRet = true;
	if (IsPhoto(strName))
	{
		if ( getTakenXap(strName, sOrgTaken) )
			parseXapTime(sOrgTaken, dtTaken);
		else
		{
			bJpgXap = false;
			if ( !getTakenNormal(strName, sOrgTaken) )
				return;
			parseNormalTime(sOrgTaken, dtTaken);
		}
		dtShowOld = dtTaken;
	}
	
	//For Debug
	//CTime dtT = status.m_ctime.GetTime();
	//int nY = dtT.GetYear();
	//int nM = dtT.GetMonth();



	//For Log
	CString sLog = _T("");
	sLog.Format(_T("%s,%d,%d,%d,"),strName, status.m_ctime.GetTime(), status.m_mtime.GetTime(), status.m_atime.GetTime());
	int nID = m_lstLog.InsertItem(m_lstLog.GetItemCount(), strName);
	//m_lstLog.SetItemText(nID, 1, status.m_ctime.Format("%Y/%m/%d %H:%M"));
	m_lstLog.SetItemText(nID, 1, dtShowOld.Format("%Y/%m/%d %H:%M"));

	//Check Time

	if ( m_sChangeType == TIME_RELATIVE)
	{
		//If checked CreateDate
		if ( m_bChangeCreate )
			dtCreate = status.m_ctime + m_dtRelative;
		else
			dtCreate = status.m_ctime;
		//If checked ModifyDate
		if ( m_bChangeModify)
		{
			if ( status.m_mtime == 0 )
				dtModify = status.m_ctime + m_dtRelative;
			else
				dtModify = status.m_mtime + m_dtRelative;
		}
		else
		{
			if ( status.m_mtime == 0 )
				dtModify = status.m_ctime;
			else
				dtModify = status.m_mtime;
		}

		//If checked AccessDate
		if ( m_bChangeAccess)
			dtAccess = status.m_atime + m_dtRelative;
		else
			dtAccess = status.m_atime;

		//If checked DateTaken
		if ( m_bChangeTaken)
			dtTaken = dtTaken + m_dtRelative;

		dtShowNew = dtShowOld + m_dtRelative;
	}

	if ( m_sChangeType == TIME_FIX )
	{
		//If checked CreateDate
		if ( m_bChangeCreate )
			dtCreate = m_dtFixDate;
		else
			dtCreate = status.m_ctime;
		
		//If checked ModifyDate
		if ( m_bChangeModify)
			dtModify = m_dtFixDate;
		else
			dtModify = status.m_mtime;

		//If checked AccessDate
		if ( m_bChangeAccess )
			dtAccess = m_dtFixDate;
		else
			dtAccess = status.m_atime;

		if ( m_bChangeTaken)
			dtTaken = m_dtFixDate;
		
		dtShowNew = m_dtFixDate;
	}


	status.m_ctime = dtCreate;
	status.m_mtime = dtModify;
	status.m_atime = dtAccess;

	//For Log
	m_lstLog.SetItemText(nID, 2, dtShowNew.Format("%Y/%m/%d %H:%M"));

	CStringA sDateTaken = "";

	sDateTaken.Format("%.4d %.2d %.2d %.2d %.2d %.2d.%.4d:%.2d:%.2d %.2d:%.2d:%.2d", 
		dtModify.GetYear(), dtModify.GetMonth(), dtModify.GetDay(),dtModify.GetHour(),dtModify.GetMinute(), dtModify.GetSecond(),
		dtModify.GetYear(), dtModify.GetMonth(), dtModify.GetDay(),dtModify.GetHour(),dtModify.GetMinute(), dtModify.GetSecond());

	


	//Check jpg, jpeg
	
	if (IsPhoto(strName))
	{
		CString sVal;
		sVal.Format(_T("%d"), dtShowOld.GetTime());
		sLog += sVal;
		sLog += _T(",");
		sLog += sDateTaken;
		count_total++;
		//if ( bJpgXap )
			processXap(strName,dtShowNew);
		//else
			try 
			{
				setTakenNormal(strName, sDateTaken);
			}
			catch(...)
			{
				bRet = false;
			}
	}

	bRet = (bRet & SetStatus(strName, status)); //HSM
	if ( bRet )
		m_lstLog.SetItemText(nID, 3, _T("OK"));
	else
		m_lstLog.SetItemText(nID, 3, _T("FALSE"));

	sLog += _T("\r\n");
	m_logFile.WriteString(sLog);
}


void CChangeFileTimeDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default
	TCHAR fname[_MAX_PATH];
	int count=::DragQueryFile(hDropInfo,0xFFFFFFFF,fname,_MAX_PATH); // count holds the number of files that were dragged and dropped
	m_sPath = fname;
	UpdateData(false);


	for(int i=0;i<count;++i)
	{
		::DragQueryFile(hDropInfo,i,fname,_MAX_PATH);
		m_sPath = fname;
		UpdateData(false);

	}

	CDialogEx::OnDropFiles(hDropInfo);
}


void CChangeFileTimeDlg::OnBnClickedBtnBrowser()
{
	BROWSEINFO   bi; 
	ZeroMemory(&bi,   sizeof(bi)); 
	TCHAR   szDisplayName[MAX_PATH]; 
	szDisplayName[0]    =   ' ';  

	bi.hwndOwner        =   NULL; 
	bi.pidlRoot         =   NULL; 
	bi.pszDisplayName   =   szDisplayName; 
	bi.lpszTitle        =   _T("Please select a folder for storing received files :"); 
	bi.ulFlags          =   BIF_RETURNONLYFSDIRS;
	bi.lParam           =   NULL; 
	bi.iImage           =   0;  

	LPITEMIDLIST   pidl   =   SHBrowseForFolder(&bi);
	TCHAR   szPathName[MAX_PATH]; 
	if   (NULL   !=   pidl)
	{
		BOOL bRet = SHGetPathFromIDList(pidl,szPathName);
		if(FALSE == bRet)
			return;
		m_sPath = szPathName;
		UpdateData(FALSE);
	}
}


void CChangeFileTimeDlg::OnBnClickedBtnPuse()
{
	CString sCaption;
	GetDlgItem(IDC_BTN_PUSE)->GetWindowText(sCaption);
	if ( sCaption == _T("Pause"))
	{
		GetDlgItem(IDC_BTN_PUSE)->SetWindowText(_T("Resume"));
		m_bPause = true;
	}
	else
	{
		GetDlgItem(IDC_BTN_PUSE)->SetWindowText(_T("Pause"));
		m_bPause = false;
	}
}

bool CChangeFileTimeDlg::parseLog(CString sLine, CString& sFile, LONGLONG& lCreate, LONGLONG& lModify, LONGLONG& lAccess, CString& sXapTaken, CString& sDateTaken)
{
	sLine.Replace(_T("\r"), _T(""));

	CString sVal = sLine;
	CArray<CString,CString> v;
	CString field;
	int index = 0;
	

	while (AfxExtractSubString(field,sVal,index,_T(',')))
	{
		v.Add(field);
		++index;
	}

	if ( v.GetCount() < 4 )
		return false;

	sFile = v[0];
	lCreate = _ttol(v[1]);
	lModify = _ttol(v[2]);
	lAccess = _ttol(v[3]);
	
	if ( v.GetCount() > 5 )
	{
		sXapTaken = v[4];
		sDateTaken = v[5];
	}

	return true;
}

int CChangeFileTimeDlg::getUndoCnt()
{
	int nCnt = 0;
	if ( !::PathFileExists(m_sLogFile) )
		return nCnt;
	m_logFile.Open(m_sLogFile, CFile::modeRead | CFile::typeText);
	CString sLine = _T("");

	while(m_logFile.ReadString(sLine)) 
	{
		nCnt++;
	}

	m_logFile.Close();

	return nCnt;
}

void CChangeFileTimeDlg::OnBnClickedBtnUndo()
{
	if ( !::PathFileExists(m_sLogFile) )
		return;
	m_ctlProgress.SetRange(0, getUndoCnt());

	m_logFile.Open(m_sLogFile, CFile::modeRead | CFile::typeText);
	CString sLine = _T("");
	CString sFile, sDateTaken, sXapTaken;
	LONGLONG lCreate, lModify, lAccess;	
	
	CFileStatus status;
	int nPos = 0;
	m_bPause = false;
	TIME_ZONE_INFORMATION tzi;
	 DWORD dwRet = GetTimeZoneInformation(&tzi);

	InitControl_undo(FALSE);
	while(m_logFile.ReadString(sLine)) 
	{
		doEvent();
		if ( m_bPause )
		{
			waitForResume();
		}

		nPos++;
		m_ctlProgress.SetPos(nPos);
		sDateTaken = _T("");
		
		if ( parseLog(sLine, sFile, lCreate, lModify, lAccess, sXapTaken, sDateTaken) )
		{
			int nY = status.m_ctime.GetYear();
			int nM = status.m_ctime.GetMonth();

			status.m_ctime = lCreate + tzi.DaylightBias*60;
			status.m_mtime = lAccess + tzi.DaylightBias*60;
			status.m_atime = lAccess + tzi.DaylightBias*60;
			
			nY = status.m_ctime.GetYear();
			nM = status.m_ctime.GetMonth();
			SetStatus(sFile, status); //HSM

			if ( sDateTaken != _T(""))
			{
				CTime dtTaken(_ttol(sXapTaken));
				processXap(sFile, dtTaken);
				try
				{
					setTakenNormal(sFile, (CStringA)CW2A(sDateTaken));
				}
				catch(...)
				{

				}
			}
		}
	}
	InitControl_undo(TRUE);
	m_logFile.Close();
}
