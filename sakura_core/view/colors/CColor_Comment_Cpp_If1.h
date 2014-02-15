/*!	@file	CColor_Comment_Cpp_If1.h
	@brief	C++ #if 1 ～ #else ～ #endif コメント判定クラス

	@author	Suzuki Satoshi
*/
/*
	Copyright (C) 2014, Suzuki Satoshi

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

#ifndef _CColor_Comment_Cpp_If1_h_
#define _CColor_Comment_Cpp_If1_h_

#include "CColorStrategy.h"

class CColor_Comment_Cpp_If1 : public CColorStrategy
{
public:

	CColor_Comment_Cpp_If1() : commentEndPos(0) { }
	~CColor_Comment_Cpp_If1() {	}
	void InitStrategyStatus()
	{
		commentEndPos = 0;
	}

	EColorIndexType GetStrategyColor() const
	{
		return COLORIDX_COMMENT_CPP_IF1;
	}

	bool Match_CommentFrom(int pos, const CStringRef& rStr, ColorStrategyState& rColorStrategyState);
	int Match_CommentTo(int pos, const CStringRef& rStr, ColorStrategyState& rColorStrategyState);
	bool BeginColor(const CStringRef& cStr, int pos, ColorStrategyState& rColorStrategyState);
	bool EndColor(const CStringRef& cStr, int pos, ColorStrategyState& rColorStrategyState);

	int commentEndPos;
};

#endif /* _CColor_Comment_Cpp_If1_h_ */

