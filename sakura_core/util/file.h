/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_FILE_2813BD8E_F6B9_400F_AA27_A6DDC372D6B89_H_
#define SAKURA_FILE_2813BD8E_F6B9_400F_AA27_A6DDC372D6B89_H_

bool fexist(LPCWSTR pszPath); //!< ファイルまたはディレクトリが存在すればtrue
bool GetVcsRepositoryRootDir(
	WCHAR*	pVcsRepoRootDir,	//!< [out] VCSルートディレクトリのパス
	const WCHAR* pSearchPath	//!< [in] 検索対象パス
);

bool GetGitRepositoryRootDir(
	WCHAR*	pszRootDir,			//!< [out] ルートフォルダのパス (末尾に '\\' を付けない)
	size_t	nRootDirCount,		//!< [in] pszRootDir のバッファサイズ (文字数)
	const WCHAR* pszDirPath		//!< [in] 検索を始めるフォルダのパス
);
bool GetHgRepositoryRootDir(
	WCHAR*	pszRootDir,			//!< [out] ルートフォルダのパス (末尾に '\\' を付けない)
	size_t	nRootDirCount,		//!< [in] pszRootDir のバッファサイズ (文字数)
	const WCHAR* pszDirPath		//!< [in] 検索を始めるフォルダのパス
);

bool IsFilePath( const wchar_t*, int*, int*, bool = true );
bool IsUncPath( const WCHAR* pszPath );	//!< UNC パス (ネットワーク上のパス) かどうか
bool IsFileExists(const WCHAR* path, bool bFileOnly = false);
bool IsBinaryFile(
	const WCHAR* path	//!< [in] 調べるパス名
);
bool IsDirectoryExists(const WCHAR* path);
bool IsDirectory(LPCWSTR pszPath);	// 2009.08.20 ryoji

//	Apr. 30, 2003 genta
//	ディレクトリの深さを調べる
int CalcDirectoryDepth(const WCHAR* path);

// 2005.11.26 aroka
bool IsLocalDrive( const WCHAR* pszDrive );

//※サクラ依存
FILE *_wfopen_absexe(LPCWSTR fname, LPCWSTR mode); // 2003.06.23 Moca
FILE *_wfopen_absini(LPCWSTR fname, LPCWSTR mode, BOOL bOrExedir = TRUE); // 2007.05.19 ryoji

//パス文字列処理
void CutLastYenFromDirectoryPath( WCHAR* );						/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない*/
bool AddLastYenFromDirectoryPath( WCHAR* pszFolder, size_t nBufCount );	/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
void SplitPath_FolderAndFile( const WCHAR*, WCHAR*, WCHAR* );	/* ファイルのフルパスを、フォルダとファイル名に分割 */
void Concat_FolderAndFile( const WCHAR*, const WCHAR*, WCHAR* );/* フォルダ、ファイル名から、結合したパスを作成 */
BOOL GetLongFileName( const WCHAR*, WCHAR* );					/* ロングファイル名を取得する */
BOOL CheckEXT( const WCHAR*, const WCHAR* );					/* 拡張子を調べる */
const WCHAR* GetFileTitlePointer(const WCHAR* tszPath);							//!< ファイルフルパス内のファイル名を指すポインタを取得。2007.09.20 kobake 作成
bool _IS_REL_PATH(const WCHAR* path);											//!< 相対パスか判定する。2003.06.23 Moca

//ファイル時刻
class CFileTime{
public:
	CFileTime(){ ClearFILETIME(); }
	CFileTime(const FILETIME& ftime){ SetFILETIME(ftime); }
	//設定
	void ClearFILETIME(){ m_ftime.dwLowDateTime = m_ftime.dwHighDateTime = 0; m_bModified = true; }
	void SetFILETIME(const FILETIME& ftime){ m_ftime = ftime; m_bModified = true; }
	//取得
	const FILETIME& GetFILETIME() const{ return m_ftime; }
	const SYSTEMTIME& GetSYSTEMTIME() const
	{
		//キャッシュ更新 -> m_systime, m_bModified
		if(m_bModified){
			m_bModified = false;
			FILETIME ftimeLocal;
			if(!::FileTimeToLocalFileTime( &m_ftime, &ftimeLocal ) || !::FileTimeToSystemTime( &ftimeLocal, &m_systime )){
				memset(&m_systime,0,sizeof(m_systime)); //失敗時ゼロクリア
			}
		}
		return m_systime;
	}
	const SYSTEMTIME* operator->() const{ return &GetSYSTEMTIME(); }
	//判定
	bool IsZero() const
	{
		return m_ftime.dwLowDateTime == 0 && m_ftime.dwHighDateTime == 0;
	}
protected:
private:
	FILETIME m_ftime;
	//キャッシュ
	mutable SYSTEMTIME	m_systime;
	mutable bool		m_bModified;
};
bool GetLastWriteTimestamp( const WCHAR* filename, CFileTime* pcFileTime ); //	Oct. 22, 2005 genta

//文字列分割
void my_splitpath_w ( const wchar_t *comln , wchar_t *drv,wchar_t *dir,wchar_t *fnm,wchar_t *ext );

#endif /* SAKURA_FILE_2813BD8E_F6B9_400F_AA27_A6DDC372D6B89_H_ */
/*[EOF]*/
