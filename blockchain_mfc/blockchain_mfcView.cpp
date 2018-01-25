// blockchain_mfcView.cpp : implementation of the CBlockchain_mfcView class
//

#include "stdafx.h"
#include "blockchain_mfc.h"

#include "blockchain_mfcDoc.h"
#include "blockchain_mfcView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcView

IMPLEMENT_DYNCREATE(CBlockchain_mfcView, CView)

BEGIN_MESSAGE_MAP(CBlockchain_mfcView, CView)
	//{{AFX_MSG_MAP(CBlockchain_mfcView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcView construction/destruction

CBlockchain_mfcView::CBlockchain_mfcView()
{
	// TODO: add construction code here

}

CBlockchain_mfcView::~CBlockchain_mfcView()
{
}

BOOL CBlockchain_mfcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcView drawing

void CBlockchain_mfcView::OnDraw(CDC* pDC)
{
	CBlockchain_mfcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcView printing

BOOL CBlockchain_mfcView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CBlockchain_mfcView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CBlockchain_mfcView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcView diagnostics

#ifdef _DEBUG
void CBlockchain_mfcView::AssertValid() const
{
	CView::AssertValid();
}

void CBlockchain_mfcView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBlockchain_mfcDoc* CBlockchain_mfcView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBlockchain_mfcDoc)));
	return (CBlockchain_mfcDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcView message handlers
