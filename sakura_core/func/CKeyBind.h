/*!	@file
	@brief キー割り当てに関するクラス

	@author Norio Nakatani
	@date 1998/03/25 新規作成
	@date 1998/05/16 クラス内にデータを持たないように変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CKEYBIND_H_
#define _CKEYBIND_H_

#include <Windows.h>
#include <Funccode_enum.h>

class CFuncLookup;

extern "C" {

//! キー情報を保持する
struct KeyData {
	/*! キーコード	*/
	short			m_nKeyCode;
	
	/*!	キーの名前	*/
	TCHAR			m_szKeyName[64];
	
	/*!	対応する機能番号

		SHIFT, CTRL, ALTの３つのシフト状態のそれぞれに対して
		機能を割り当てるため、配列になっている。
	*/
	EFunctionCode	m_nFuncCodeArr[8];
};

}	// extern "C"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief キー割り当て関連ルーチン
	
	すべての関数はstaticで保持するデータはない。
*/
class CKeyBind
{
public:
	/*
	||  Constructors
	*/
	CKeyBind();
	~CKeyBind();

	/*
	||  参照系メンバ関数
	*/
	static HACCEL CreateAccerelator( int, KeyData* );
	static EFunctionCode GetFuncCode( WORD nAccelCmd, int nKeyNameArrNum, KeyData* pKeyNameArr, BOOL bGetDefFuncCode = TRUE );
	static EFunctionCode GetFuncCodeAt( KeyData& rKeyData, int nState, BOOL bGetDefFuncCode = TRUE );	/* 特定のキー情報から機能コードを取得する */	// 2007.02.24 ryoji
	static EFunctionCode GetDefFuncCode( int nKeyCode, int nState );	/* キーのデフォルト機能を取得する */	// 2007.02.22 ryoji

	//! キー割り当て一覧を作成する
	static int CreateKeyBindList( HINSTANCE hInstance, int nKeyNameArrNum, KeyData* pKeyNameArr, CNativeW& cMemList, CFuncLookup* pcFuncLookup, BOOL bGetDefFuncCode = TRUE );
	static int GetKeyStr( HINSTANCE hInstance, int nKeyNameArrNum, KeyData* pKeyNameArr, CNativeT& cMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* 機能に対応するキー名の取得 */
	static int GetKeyStrList( HINSTANCE	hInstance, int nKeyNameArrNum,KeyData* pKeyNameArr, CNativeT*** pppcMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* 機能に対応するキー名の取得(複数) */
	static TCHAR* GetMenuLabel( HINSTANCE hInstance, int nKeyNameArrNum, KeyData* pKeyNameArr, int nFuncId, TCHAR* pszLabel, const TCHAR* pszKey, BOOL bKeyStr, BOOL bGetDefFuncCode = TRUE );	/* メニューラベルの作成 */	// add pszKey	2010/5/17 Uchi

	static TCHAR* MakeMenuLabel(const TCHAR* sName, const TCHAR* sKey);
	static bool CKeyBind::InitKeyAssign( DLLSHAREDATA* pShareData );
	static void SetDefaultKeyBindToBenizakura(struct CommonSetting_KeyBind* pKeyBind);
	static void SetDefaultKeyBindToSakura(struct CommonSetting_KeyBind* pKeyBind);

private:
	static void SetDefaultKeyBind(struct CommonSetting_KeyBind* pKeyBind, const KeyData* pKeyData);

protected:
	/*
	||  実装ヘルパ関数
	*/
	static bool GetKeyStrSub(int& nKeyNameArrBegin, int nKeyNameArrEnd, KeyData* pKeyNameArr,
			int nShiftState, CNativeT& cMemList, int nFuncId, BOOL bGetDefFuncCode );
};

///////////////////////////////////////////////////////////////////////
#endif /* _CKEYBIND_H_ */



