#if !defined(AFX_COMMANDVIEW_H__04A7A69B_D43D_47A5_99BB_C9230DAF91BD__INCLUDED_)
#define AFX_COMMANDVIEW_H__04A7A69B_D43D_47A5_99BB_C9230DAF91BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommandView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommandView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

//include
#include "MainFrm.h"

class CCommandView : public CFormView
{
protected:
	CCommandView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCommandView)

// Form Data
public:
	//{{AFX_DATA(CCommandView)
	enum { IDD = IDD_FORMVIEW };
	CListCtrl	m_list;
	//}}AFX_DATA
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommandView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
protected:
	virtual ~CCommandView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CCommandView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMANDVIEW_H__04A7A69B_D43D_47A5_99BB_C9230DAF91BD__INCLUDED_)
