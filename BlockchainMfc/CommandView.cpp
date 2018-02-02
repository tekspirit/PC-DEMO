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

//extern
extern uint32 g_devicenum[2];//设备个数
extern uint32 g_devicerange;//设备坐标范围
extern uint32 g_devicestep;//设备步进值
extern uint32 g_number;//交易原子列表个数
extern deal_t *g_deal;//交易原子列表
extern CRITICAL_SECTION g_cs;
extern device_t *g_device;//设备数组
extern mainchain_t g_mainchain;//主链
extern volatile uint32 g_index;//临时用来统计交易号码的(以后会用hash_t代替,计数从1开始)
extern volatile uint8 g_flag;//使用timer计时重发
extern volatile uint8 g_run;//0-停止,1-运行
extern uint8 **g_check;//[g_number][5].0-source,1-mc(src),2-mc(dst),3-node(src),4-node(dst)

CCommandView::CCommandView()
	: CFormView(CCommandView::IDD)
{
	m_flag=0;
}

CCommandView::~CCommandView()
{
}

void CCommandView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommandView)
	DDX_Control(pDX, IDC_BUTTON_INFINITE, m_infinite);
	DDX_Control(pDX, IDC_BUTTON_LIMIT, m_limit);
	DDX_Control(pDX, IDC_LIST_TRANSACTION, m_transaction);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_device);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommandView, CFormView)
	//{{AFX_MSG_MAP(CCommandView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_LIMIT, OnButtonLimit)
	ON_BN_CLICKED(IDC_BUTTON_INFINITE, OnButtonInfinite)
	ON_WM_TIMER()
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
	//fill list
	m_transaction.SetExtendedStyle(LVS_EX_GRIDLINES);//设置扩展风格为网格
	m_transaction.InsertColumn(0,"Transaction",LVCFMT_LEFT,80);
	m_transaction.InsertColumn(1,"Source",LVCFMT_LEFT,60);
	m_transaction.InsertColumn(2,"MC(Src)",LVCFMT_LEFT,60);
	m_transaction.InsertColumn(3,"MC(Dst)",LVCFMT_LEFT,60);
	m_transaction.InsertColumn(4,"Node(Src)",LVCFMT_LEFT,70);
	m_transaction.InsertColumn(5,"Node(Dst)",LVCFMT_LEFT,70);
	m_device.SetExtendedStyle(LVS_EX_GRIDLINES);//设置扩展风格为网格
	m_device.InsertColumn(0,"Node",LVCFMT_LEFT,60);
	m_device.InsertColumn(1,"Token(Available)",LVCFMT_LEFT,170);
	m_device.InsertColumn(2,"Token(Frozen)",LVCFMT_LEFT,170);
	//set flag
	m_token=g_device[0].token[0];
	m_flag=1;
}

void CCommandView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	
	if (m_flag)
	{
		CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
		cx=math_max((int)pMain->m_scrollsize[2].cx,cx);
		cy=math_max((int)pMain->m_scrollsize[2].cy,cy);
		//adjust list
		m_transaction.MoveWindow(0,0,cx,cy*7/10);
		m_device.MoveWindow(0,cy*7/10,cx,cy/5);
		//adjust button
		m_limit.MoveWindow(math_max(cx/4-50,0),cy*19/20-14,100,28);
		m_infinite.MoveWindow(math_max(cx*3/4-50,0),cy*19/20-14,100,28);
	}
}

void CCommandView::OnButtonLimit() 
{
	uint32 i;
	CString str;

	//interface
	if (!g_run)
	{
		g_run=1;
		m_limit.SetWindowText("pausing...");
		m_transaction.DeleteAllItems();
		for (i=0;i<g_number;i++)
		{
			str.Format("%ld->%ld:%ld",g_deal[i].device_index[0],g_deal[i].device_index[1],g_deal[i].token);
			m_transaction.InsertItem(i,str);
			memset(g_check[i],0,5);
		}
		m_device.DeleteAllItems();
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
		{
			g_device[i].token[0]=m_token;
			g_device[i].token[1]=0;
			str.Format("%ld",g_device[i].device_index);
			m_device.InsertItem(i,str);
			str.Format("%ld",g_device[i].token[0]);
			m_device.SetItemText(i,1,str);
			str.Format("%ld",g_device[i].token[1]);
			m_device.SetItemText(i,2,str);
		}
		m_infinite.EnableWindow(FALSE);
		SetTimer(0,TIMER_CONNECT*1000,NULL);//原则上时钟线程应该是每个设备线程配备一个,这里简化为共用一个
	}
	else
	{
		g_run=0;
		m_limit.SetWindowText("running...");
		KillTimer(0);
	}
	//process
}

void CCommandView::OnButtonInfinite() 
{
	/*
	uint32 i,j;
	CString str;

	//interface
	if (!g_run[1])
	{
		m_run[1]=1;
		m_infinite.SetWindowText("pause...");
		m_transaction.DeleteAllItems();
		for (i=0;i<g_number;i++)
		{
			str.Format("%ld->%ld:%ld",g_deal[i].device_index[0],g_deal[i].device_index[1],g_deal[i].token);
			m_transaction.InsertItem(i,str);
			for (j=0;j<5;j++)
			{
				if (g_check[i])
					m_transaction.SetItemText(i,j,"√");
			}
		}
		m_device.DeleteAllItems();
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
		{
			str.Format("%ld",g_device[i].device_index);
			m_device.InsertItem(i,str);
			str.Format("%ld",g_device[i].token[0]);
			m_device.SetItemText(i,1,str);
			str.Format("%ld",g_device[i].token[1]);
			m_device.SetItemText(i,2,str);
		}
		m_device.EnableWindow(FALSE);
		SetTimer(0,TIMER_CONNECT*1000,NULL);//原则上时钟线程应该是每个设备线程配备一个,这里简化为共用一个
	}
	else
	{
		m_run[1]=0;
		m_infinite.SetWindowText("infinite case");
		KillTimer(0);
	}
	m_run[0]=0;
	m_limit.SetWindowText("limit case");
	//process
	g_run=m_run[1];*/
}

void CCommandView::OnTimer(UINT nIDEvent) 
{
	uint32 i,j,k;
	CString str;
	static uint32 count=0;

	EnterCriticalSection(&g_cs);
	if (count++==100)
	{
		count=0;
		g_flag=!g_flag;
	}
	//for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	//	move_location(&g_device[i],g_devicestep,g_devicerange);
	//

	//interface
	for (i=0;i<g_number;i++)
		for (j=0;j<5;j++)
		{
			if (g_check[i][j])
				m_transaction.SetItemText(i,j+1,"√");
		}
	m_transaction.UpdateData();
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		str.Format("%ld",g_device[i].token[0]);
		m_device.SetItemText(i,1,str);
		str.Format("%ld",g_device[i].token[1]);
		m_device.SetItemText(i,2,str);
	}
	m_device.UpdateData();
	//check
	k=0;
	for (i=0;i<g_number;i++)
		for (j=0;j<5;j++)
			k+=g_check[i][j];
	if (k>=g_number*5-4)
	{
		KillTimer(0);
		m_limit.SetWindowText("limit case");
		m_infinite.EnableWindow(TRUE);
		//
		count=0;
		g_run=0;
		g_index=0;
		g_flag=0;
		delete[] g_deal;
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
			device_delete(&g_device[i]);
		delete[] g_device;
		mainchain_delete(&g_mainchain);
		//
		for (i=0;i<g_number;i++)
			delete[] g_check[i];
		delete[] g_check;
	}
	LeaveCriticalSection(&g_cs);
	
	CFormView::OnTimer(nIDEvent);
}
