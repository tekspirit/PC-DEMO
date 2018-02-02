// ConnectView.cpp : implementation file
//

#include "stdafx.h"
#include "BlockchainMfc.h"
#include "ConnectView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConnectView

IMPLEMENT_DYNCREATE(CConnectView, CScrollView)

extern volatile uint8 g_run;//0-停止,1-运行

CConnectView::CConnectView()
{
}

CConnectView::~CConnectView()
{
}


BEGIN_MESSAGE_MAP(CConnectView, CScrollView)
	//{{AFX_MSG_MAP(CConnectView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectView drawing

void CConnectView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	SetScrollSizes(MM_TEXT,pMain->m_scrollsize[0]);
}

void CConnectView::OnDraw(CDC* pDC)
{
	/*
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
*/

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	CString str;

	str.Format("%ld",g_run);
	pDC->TextOut(100,100,str);
	SetScrollPos(SB_HORZ,pMain->m_scrollsize[0].cx);
	SetScrollPos(SB_VERT,pMain->m_scrollsize[0].cy);
/*
	pDC->Rectangle(100,100,200,200);
	pDC->Rectangle(500,500,900,900);
	pDC->Rectangle(1200,1200,1500,1500);


	static i=0;
	CSize sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 1500+(i+=10);
	SetScrollSizes(MM_TEXT, sizeTotal);

	*/
	/*
	if (pDoc->IsNew)                      //对新建文件的操作
	{
		if (pDoc->m_new_style)
		{
            for (int i=1;i<=pDoc->m_xnum;i++)
				for (int j=1;j<=pDoc->m_ynum;j++)
				{
			    	pDC->Rectangle((i-1)*pDoc->m_xvalue,(j-1)*pDoc->m_yvalue,i*pDoc->m_xvalue,j*pDoc->m_yvalue);
				}
		}
		else
		{
			if (pDoc->m_draw_default)
			{
				CRect r;
				GetClientRect(&r);
				pDC->Rectangle(&r);
			}
			else
			{
				pDC->Rectangle(0,0,pDoc->m_draw_width,pDoc->m_draw_height);
			}
		}
	}
	
	if (pDoc->m_pDib)//如果有m_pDib存在的话，开始显示图像
	{
		pDoc->m_set_para.Open();
		if (!pDoc->m_pDib->ShowBmpFile(pDC,pDoc->m_set_para.m_ImageStartX,pDoc->m_set_para.m_ImageStartY,pDoc->m_set_para.m_ImageWidth,pDoc->m_set_para.m_ImageHeight))
		{
	        pDoc->m_set_para.Close();
			return;
		}
		    
		CSize sizeTotal;//滚动条变化
		sizeTotal.cx = pDoc->m_pDib->DibWidth();
	    sizeTotal.cy = pDoc->m_pDib->DibHeight();
		SetScrollSizes(MM_TEXT, sizeTotal);
        
		pDoc->m_set_para.Close();
	}*/
}

/////////////////////////////////////////////////////////////////////////////
// CConnectView diagnostics

#ifdef _DEBUG
void CConnectView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CConnectView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CConnectView message handlers
