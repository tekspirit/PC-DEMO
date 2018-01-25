// blockchain_mfcDoc.h : interface of the CBlockchain_mfcDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BLOCKCHAIN_MFCDOC_H__6FCC7AFE_D2F6_4519_9775_E44A891AA752__INCLUDED_)
#define AFX_BLOCKCHAIN_MFCDOC_H__6FCC7AFE_D2F6_4519_9775_E44A891AA752__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CBlockchain_mfcDoc : public CDocument
{
protected: // create from serialization only
	CBlockchain_mfcDoc();
	DECLARE_DYNCREATE(CBlockchain_mfcDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockchain_mfcDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBlockchain_mfcDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBlockchain_mfcDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKCHAIN_MFCDOC_H__6FCC7AFE_D2F6_4519_9775_E44A891AA752__INCLUDED_)
