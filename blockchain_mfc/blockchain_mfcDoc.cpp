// blockchain_mfcDoc.cpp : implementation of the CBlockchain_mfcDoc class
//

#include "stdafx.h"
#include "blockchain_mfc.h"

#include "blockchain_mfcDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcDoc

IMPLEMENT_DYNCREATE(CBlockchain_mfcDoc, CDocument)

BEGIN_MESSAGE_MAP(CBlockchain_mfcDoc, CDocument)
	//{{AFX_MSG_MAP(CBlockchain_mfcDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcDoc construction/destruction

CBlockchain_mfcDoc::CBlockchain_mfcDoc()
{
	// TODO: add one-time construction code here

}

CBlockchain_mfcDoc::~CBlockchain_mfcDoc()
{
}

BOOL CBlockchain_mfcDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcDoc serialization

void CBlockchain_mfcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcDoc diagnostics

#ifdef _DEBUG
void CBlockchain_mfcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBlockchain_mfcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcDoc commands
