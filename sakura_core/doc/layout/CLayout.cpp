/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CLayout.h"
#include "CLayoutMgr.h"
#include "charset/charcode.h"
#include "extmodule/CBregexp.h" // CLayoutMgr�̒�`�ŕK�v



CLayout::~CLayout()
{
	return;
}

void CLayout::DUMP( void )
{
	DEBUG_TRACE( _T("\n\n��CLayout::DUMP()======================\n") );
	DEBUG_TRACE( _T("m_ptLogicPos.y=%d\t\t�Ή�����_���s�ԍ�\n"), m_ptLogicPos.y );
	DEBUG_TRACE( _T("m_ptLogicPos.x=%d\t\t�Ή�����_���s�̐擪����̃I�t�Z�b�g\n"), m_ptLogicPos.x );
	DEBUG_TRACE( _T("m_nLength=%d\t\t�Ή�����_���s�̃n�C�g��\n"), (int)m_nLength );
	DEBUG_TRACE( _T("m_colorIndexPrev=%d\t\tColorIndexType\n"), m_colorIndexPrev );
	DEBUG_TRACE( _T("======================\n") );
	return;
}

//!���C�A�E�g�����v�Z�B���s�͊܂܂Ȃ��B
//2007.10.11 kobake �쐬
//2007.11.29 kobake �^�u�����v�Z����Ă��Ȃ������̂��C��
//2011.12.26 Moca �C���f���g�͊܂ނ悤�ɕύX(���W�ϊ��o�O�C��)
CLayoutInt CLayout::CalcLayoutWidth(const CLayoutMgr& cLayoutMgr) const
{
	//�\�[�X
	const wchar_t* pText    = m_pCDocLine->GetPtr();
	CLogicInt      nTextLen = m_pCDocLine->GetLengthWithoutEOL();

	//�v�Z
	CLayoutInt nWidth = GetIndent();
	for(CLogicInt i=m_ptLogicPos.GetX2();i<m_ptLogicPos.GetX2()+m_nLength;i++){
		if(pText[i]==WCODE::TAB){
			nWidth += cLayoutMgr.GetActualTabSpace(nWidth);
		}
		else{
			nWidth += CNativeW::GetKetaOfChar(pText,nTextLen,i);
		}
	}
	return nWidth;
}

//! �I�t�Z�b�g�l�����C�A�E�g�P�ʂɕϊ����Ď擾�B2007.10.17 kobake
CLayoutInt CLayout::CalcLayoutOffset(const CLayoutMgr& cLayoutMgr) const
{
	CLayoutInt nRet(0);
	if(this->GetLogicOffset()){
		const wchar_t* pLine = this->m_pCDocLine->GetPtr();
		int nLineLen = this->m_pCDocLine->GetLengthWithEOL();
		for(int i=0;i<GetLogicOffset();i++){
			if(pLine[i]==WCODE::TAB){
				nRet+=cLayoutMgr.GetActualTabSpace(nRet);
			}
			else{
				nRet+=CNativeW::GetKetaOfChar(pLine,nLineLen,i);
			}
		}
	}
	return nRet;
}


