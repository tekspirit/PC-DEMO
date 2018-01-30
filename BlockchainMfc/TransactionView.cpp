// TransactionView.cpp : implementation file
//

#include "stdafx.h"
#include "BlockchainMfc.h"
#include "TransactionView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTransactionView

IMPLEMENT_DYNCREATE(CTransactionView, CScrollView)

CTransactionView::CTransactionView()
{
}

CTransactionView::~CTransactionView()
{
}


BEGIN_MESSAGE_MAP(CTransactionView, CScrollView)
	//{{AFX_MSG_MAP(CTransactionView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransactionView drawing

void CTransactionView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	SetScrollSizes(MM_TEXT,pMain->m_scrollsize[1]);
}

void CTransactionView::OnDraw(CDC* pDC)
{
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	CBlockchainMfcDoc* pDoc = (CBlockchainMfcDoc *)GetDocument();//这里的CDocument要变为CBlockchainMfcDoc
	ASSERT_VALID(pDoc);

	SetScrollPos(SB_HORZ,pMain->m_scrollsize[1].cx);
	SetScrollPos(SB_VERT,pMain->m_scrollsize[1].cy);
}

/////////////////////////////////////////////////////////////////////////////
// CTransactionView diagnostics

#ifdef _DEBUG
void CTransactionView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTransactionView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTransactionView message handlers
