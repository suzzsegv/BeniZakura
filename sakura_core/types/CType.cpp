#include "StdAfx.h"
#include "CType.h"
#include "view/Colors/EColorIndexType.h"
#include "env/CDocTypeManager.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

void _DefaultConfig(STypeConfig* pType);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CTypeConfig                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
STypeConfig* CTypeConfig::GetTypeConfig()
{
	return &CDocTypeManager().GetTypeSetting(*this);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          CType                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CType::InitTypeConfig(int nIdx)
{
	//規定値をコピー
	static STypeConfig sDefault;
	static bool bLoadedDefault = false;
	if(!bLoadedDefault){
		_DefaultConfig(&sDefault);
		bLoadedDefault=true;
	}
	CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx)) = sDefault;

	//インデックスを設定
	CTypeConfig(nIdx)->m_nIdx = nIdx;

	//個別設定
	InitTypeConfigImp(CTypeConfig(nIdx).GetTypeConfig());
}

/*!
 *	キーワードセットのデフォルト値を追加する
 *
 *	@return CKeyWordSetMgr に登録したキーワードセット番号
 */
int CType::AddDefaultKeywordSet(
	const wchar_t*	pSetName,			//!< [in] セット名
	bool			bCaseSensitive,		//!< [in] 大文字小文字の区別．true:あり, false:無し
	int				numOfKeyWords,		//!< [in] 登録するキーワード数
	const wchar_t*	pKeyWordSet[]		//!< [in] 登録するキーワードの配列
)
{
	DLLSHAREDATA* pShareData = &GetDllShareData();
	CKeyWordSetMgr* pKeyWordSetMgr = &pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	int keyWordSetIdx;

	keyWordSetIdx = pKeyWordSetMgr->AddKeyWordSet( pSetName, bCaseSensitive );
	pKeyWordSetMgr->SetKeyWordArr( keyWordSetIdx, numOfKeyWords, pKeyWordSet );

	return keyWordSetIdx;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        デフォルト                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DefaultConfig(STypeConfig* pType)
{
//キーワード：デフォルトカラー設定
/************************/
/* タイプ別設定の規定値 */
/************************/

	pType->m_nTextWrapMethod = WRAP_SETTING_WIDTH;	// テキストの折り返し方法		// 2008.05.30 nasukoji
	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);	/* 折り返し桁数 */
	pType->m_nColmSpace = 0;					/* 文字と文字の隙間 */
	pType->m_nLineSpace = 0;					/* 行間のすきま */
	pType->m_nTabSpace = CLayoutInt(4);					/* TABの文字数 */
	for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		pType->m_nKeyWordSetIdx[i] = -1;
	}
	wcscpy( pType->m_szTabViewString, _EDITL("o.......") );	/* TAB表示文字列 */
	pType->m_bTabArrow = false;				/* タブ矢印表示 */	// 2001.12.03 hor
	pType->m_bInsSpace = FALSE;				/* スペースの挿入 */	// 2001.12.03 hor
	
	//@@@ 2002.09.22 YAZAKI 以下、m_cLineCommentとm_cBlockCommentsを使うように修正
	pType->m_cLineComment.CopyTo(0, L"", -1);	/* 行コメントデリミタ */
	pType->m_cLineComment.CopyTo(1, L"", -1);	/* 行コメントデリミタ2 */
	pType->m_cLineComment.CopyTo(2, L"", -1);	/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	pType->m_cBlockComments[0].SetBlockCommentRule(L"", L"");	/* ブロックコメントデリミタ */
	pType->m_cBlockComments[1].SetBlockCommentRule(L"", L"");	/* ブロックコメントデリミタ2 */

	pType->m_nStringType = 0;					/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"" );		/* その他のインデント対象文字 */

	pType->m_nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	_tcscpy( pType->m_szHokanFile, _T("") );		/* 入力補完 単語ファイル */
	// 2001/06/14 End

	pType->m_nHokanType = 0;

	// 2001/06/19 asa-o
	pType->m_bHokanLoHiCase = false;			// 入力補完機能：英大文字小文字を同一視する

	//	2003.06.23 Moca ファイル内からの入力補完機能
	pType->m_bUseHokanByFile = true;			//! 入力補完 開いているファイル内から候補を探す
	pType->m_bUseHokanByKeyword = true;			// 強調キーワードから入力補完

	// 文字コード設定
	pType->m_encoding.m_bPriorCesu8 = false;
	pType->m_encoding.m_eDefaultCodetype = CODE_SJIS;
	pType->m_encoding.m_eDefaultEoltype = EOL_CRLF;
	pType->m_encoding.m_bDefaultBom = false;

	//@@@2002.2.4 YAZAKI
	pType->m_szExtHelp[0] = L'\0';
	pType->m_szExtHtmlHelp[0] = L'\0';
	pType->m_bHtmlHelpIsSingle = true;

	pType->m_bAutoIndent = true;			/* オートインデント */
	pType->m_bAutoIndent_ZENSPACE = true;	/* 日本語空白もインデント */
	pType->m_bRTrimPrevLine = false;		// 2005.10.11 ryoji 改行時に末尾の空白を削除

	pType->m_nIndentLayout = 0;	/* 折り返しは2行目以降を字下げ表示 */


	for( int i = 0; i < COLORIDX_LAST; ++i ){
		GetDefaultColorInfo(&pType->m_ColorInfoArr[i],i);
	}
	pType->m_szBackImgPath[0] = '\0';
	pType->m_backImgPos = BGIMAGE_TOP_LEFT;
	pType->m_backImgRepeatX = true;
	pType->m_backImgRepeatY = true;
	pType->m_backImgScrollX = true;
	pType->m_backImgScrollY = true;
	{
		POINT pt ={0,0};
		pType->m_backImgPosOffset = pt;
	}
	pType->m_bLineNumIsCRLF = true;					// 行番号の表示 FALSE=折り返し単位／TRUE=改行単位
	pType->m_nLineTermType = 1;						// 行番号区切り 0=なし 1=縦線 2=任意
	pType->m_cLineTermChar = L':';					// 行番号区切り文字
	pType->m_bWordWrap = false;						// 英文ワードラップをする
	pType->m_nCurrentPrintSetting = 0;				// 現在選択している印刷設定
	pType->m_bOutlineDockDisp = false;				// アウトライン解析表示の有無
	pType->m_eOutlineDockSide = DOCKSIDE_FLOAT;		// アウトライン解析ドッキング配置
	pType->m_cxOutlineDockLeft = 0;					// アウトラインの左ドッキング幅
	pType->m_cyOutlineDockTop = 0;					// アウトラインの上ドッキング高
	pType->m_cxOutlineDockRight = 0;				// アウトラインの右ドッキング幅
	pType->m_cyOutlineDockBottom = 0;				// アウトラインの下ドッキング高
	pType->m_eDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	pType->m_nOutlineSortCol = 0;					/* アウトライン解析ソート列番号 */
	pType->m_nOutlineSortType = 0;					/* アウトライン解析ソート基準 */
	pType->m_eSmartIndent = SMARTINDENT_NONE;		/* スマートインデント種別 */
	pType->m_nImeState = IME_CMODE_NOCONVERSION;	/* IME入力 */

	pType->m_szOutlineRuleFilename[0] = L'\0';		//Dec. 4, 2000 MIK
	pType->m_bKinsokuHead = false;					// 行頭禁則				//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = false;					// 行末禁則				//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = false;					// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = false;					// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"" );			// 行頭禁則				//@@@ 2002.04.08 MIK
	wcscpy( pType->m_szKinsokuTail, L"" );			// 行末禁則				//@@@ 2002.04.08 MIK
	wcscpy( pType->m_szKinsokuKuto, L"、。，．､｡,." );	// 句読点ぶら下げ文字	// 2009.08.07 ryoji

	pType->m_bUseDocumentIcon = false;				// 文書に関連づけられたアイコンを使う

//@@@ 2001.11.17 add start MIK
	for(int i = 0; i < _countof(pType->m_RegexKeywordArr); i++)
	{
		pType->m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
	}
	pType->m_RegexKeywordList[0] = L'\0';
	pType->m_bUseRegexKeyword = false;
//		pType->m_nRegexKeyMagicNumber = 1;
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	for(int i = 0; i < MAX_KEYHELP_FILE; i++){
		pType->m_KeyHelpArr[i].m_bUse = false;
		pType->m_KeyHelpArr[i].m_szAbout[0] = _T('\0');
		pType->m_KeyHelpArr[i].m_szPath[0] = _T('\0');
	}
	pType->m_bUseKeyWordHelp = false;		// 辞書選択機能の使用可否
	pType->m_nKeyHelpNum = 0;				// 登録辞書数
	pType->m_bUseKeyHelpAllSearch = false;	// ヒットした次の辞書も検索(&A)
	pType->m_bUseKeyHelpKeyDisp = false;	// 1行目にキーワードも表示する(&W)
	pType->m_bUseKeyHelpPrefix = false;		// 選択範囲で前方一致検索(&P)
//@@@ 2006.04.10 fon ADD-end

	// 2005.11.08 Moca 指定位置縦線の設定
	for(int i = 0; i < MAX_VERTLINES; i++ ){
		pType->m_nVertLineIdx[i] = CLayoutInt(0);
	}
}
