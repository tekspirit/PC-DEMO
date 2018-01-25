// blockchain_mfc.h : main header file for the BLOCKCHAIN_MFC application
//

#if !defined(AFX_BLOCKCHAIN_MFC_H__77B158C8_8A1B_4507_912E_E4E8F87E9452__INCLUDED_)
#define AFX_BLOCKCHAIN_MFC_H__77B158C8_8A1B_4507_912E_E4E8F87E9452__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CBlockchain_mfcApp:
// See blockchain_mfc.cpp for the implementation of this class
//

class CBlockchain_mfcApp : public CWinApp
{
public:
	CBlockchain_mfcApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockchain_mfcApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CBlockchain_mfcApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKCHAIN_MFC_H__77B158C8_8A1B_4507_912E_E4E8F87E9452__INCLUDED_)
