// CommandView.cpp : implementation file
//

#include "stdafx.h"
#include "BlockchainMfc.h"
#include "CommandView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommandView

IMPLEMENT_DYNCREATE(CCommandView, CFormView)

CCommandView::CCommandView()
	: CFormView(CCommandView::IDD)
{
}

CCommandView::~CCommandView()
{
}

void CCommandView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommandView)
	DDX_Control(pDX, IDC_LIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommandView, CFormView)
	//{{AFX_MSG_MAP(CCommandView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommandView diagnostics

#ifdef _DEBUG
void CCommandView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCommandView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCommandView message handlers

void CCommandView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	//expand list
	//m_list.MoveWindow(100,100,200,200);
	m_list.MoveWindow(0,0,pMain->m_scrollsize[2].cx,pMain->m_scrollsize[2].cy);


	//fill list
	m_list.InsertColumn(0,"Transaction",LVCFMT_CENTER,55);
	m_list.InsertColumn(1,"Source",LVCFMT_CENTER,55);
	m_list.InsertColumn(2,"Mainchain(Src)",LVCFMT_CENTER,55);
	m_list.InsertColumn(3,"Mainchain(Dst)",LVCFMT_CENTER,55);
	m_list.InsertColumn(4,"Confirm(Src)",LVCFMT_CENTER,55);
	m_list.InsertColumn(5,"Confirm(Dst)",LVCFMT_CENTER,55);




/*
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	for (int i=0;i<8;i++)
	{
		lvc.iSubItem=i;
		lvc.pszText="a";//_gszColumnLabel[i];
		lvc.cx = 55;//_gnColumnWidth[i];
		lvc.fmt = LVCFMT_LEFT;//_gnColumnFmt[i];
		m_list.InsertColumn(i,&lvc);
	}
*/	






	/*
	m_ListCtrl.InsertColumn(1,"身高"),LVCFMT_CENTER,60);
	m_ListCtrl.InsertColumn(2,_T("体重"),LVCFMT_CENTER,60);
	m_ListCtrl.InsertColumn(3,_T("测量时间"),LVCFMT_CENTER,180);

	m_ListCtrl.InsertItem(0,"张三");
	m_ListCtrl.SetItemText(0,1,"178CM");
	m_ListCtrl.SetItemText(0,2,"70KG");
	m_ListCtrl.SetItemText(0,3,"2009年1月15日23时40分");

	m_ListCtrl.InsertItem(1,"王五");
	m_ListCtrl.SetItemText(1,1,"178cm");
	m_ListCtrl.SetItemText(1,2,"70kg");
	m_ListCtrl.SetItemText(1,3,"2009年1月15日23时40分");

	m_ListCtrl.InsertItem(2,"阿花");
	m_ListCtrl.SetItemText(2,1,"168cm");
	m_ListCtrl.SetItemText(2,2,"60kg");
	m_ListCtrl.SetItemText(2,3,"2009年1月15日23时40分");

	SetWindowLong(m_ListCtrl.m_hWnd ,GWL_EXSTYLE,WS_EX_CLIENTEDGE);
	m_ListCtrl.SetExtendedStyle(LVS_EX_GRIDLINES);                     //设置扩展风格为网格
	::SendMessage(m_ListCtrl.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		*/
}
