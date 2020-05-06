/*!	@file	SessionStore.h
	@brief	終了時に編集中のファイルに関する情報を記録し、再起動後に復元する.

	@author	Suzuki Satoshi
 */
/*
	Copyright (C) 2015, Suzuki Satoshi

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

#ifndef _SessionStore_h_
#define _SessionStore_h_

#include "stdio.h"

class CEditWnd;
class CMutex;
struct EditInfo;

class SessionStore {
private:

	SessionStore() {}
	static SessionStore* pUniqInstance;

public:

	static SessionStore* getInstance() {
		return pUniqInstance;
	}

	static void createUniqInstance();
	static void destroyUniqInstance();

	void save(void);
	void restore(CEditWnd* pEditWnd, EditInfo* pEditInfo);

private:
	CMutex* pMutex;
	FILE* fp;
	void fileOpenForSave(void);
	void fileOneRecordWrite(const wchar_t* pRecord);
	bool fileOpenForRestore(void);
	bool fileOneRecordRead(wchar_t* rRecord);
	void fileClose(void);

};

#endif /* _SessionStore_h_ */

