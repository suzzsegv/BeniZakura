/*!	@file
	@brief 指定行へのジャンプダイアログボックス

	@author Norio Nakatani
	@date 1998/05/31 作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgJump;

#ifndef _CDLGJUMP_H_
#define _CDLGJUMP_H_

#include "dlg/CDialog.h"
//! 指定行へのジャンプダイアログボックス
class SAKURA_CORE_API CDlgJump : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgJump();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM/* , BOOL */);	/* モーダルダイアログの表示 */

	int			m_nLineNum;		/*!< 行番号 */
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnNotify( WPARAM,  LPARAM );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);
	void SetData( void );		/* ダイアログデータの設定 */
	int GetData( void );		/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGJUMP_H_ */



