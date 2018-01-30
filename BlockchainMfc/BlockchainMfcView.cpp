// BlockchainMfcView.cpp : implementation of the CBlockchainMfcView class
//

#include "stdafx.h"
#include "BlockchainMfc.h"

#include "BlockchainMfcDoc.h"
#include "BlockchainMfcView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView

IMPLEMENT_DYNCREATE(CBlockchainMfcView, CView)

BEGIN_MESSAGE_MAP(CBlockchainMfcView, CView)
	//{{AFX_MSG_MAP(CBlockchainMfcView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView construction/destruction

CBlockchainMfcView::CBlockchainMfcView()
{
}

CBlockchainMfcView::~CBlockchainMfcView()
{
}

BOOL CBlockchainMfcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView drawing

void CBlockchainMfcView::OnDraw(CDC* pDC)
{
	CBlockchainMfcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView printing

BOOL CBlockchainMfcView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CBlockchainMfcView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CBlockchainMfcView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView diagnostics

#ifdef _DEBUG
void CBlockchainMfcView::AssertValid() const
{
	CView::AssertValid();
}

void CBlockchainMfcView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBlockchainMfcDoc* CBlockchainMfcView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBlockchainMfcDoc)));
	return (CBlockchainMfcDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcView message handlers
