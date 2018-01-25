// blockchain_mfcView.h : interface of the CBlockchain_mfcView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BLOCKCHAIN_MFCVIEW_H__F74292EE_BD64_482B_A059_8511B91A85B4__INCLUDED_)
#define AFX_BLOCKCHAIN_MFCVIEW_H__F74292EE_BD64_482B_A059_8511B91A85B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CBlockchain_mfcView : public CView
{
protected: // create from serialization only
	CBlockchain_mfcView();
	DECLARE_DYNCREATE(CBlockchain_mfcView)

// Attributes
public:
	CBlockchain_mfcDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockchain_mfcView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBlockchain_mfcView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBlockchain_mfcView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in blockchain_mfcView.cpp
inline CBlockchain_mfcDoc* CBlockchain_mfcView::GetDocument()
   { return (CBlockchain_mfcDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKCHAIN_MFCVIEW_H__F74292EE_BD64_482B_A059_8511B91A85B4__INCLUDED_)
