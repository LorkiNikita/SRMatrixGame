// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

CForm * g_FormFirst;
CForm * g_FormLast;
CForm * g_FormCur;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CForm::CForm() : CMain(),m_Name()
{
    DTRACE();

	m_FormPrev=nullptr;
	m_FormNext=nullptr;

	LIST_ADD(this,g_FormFirst,g_FormLast,m_FormPrev,m_FormNext);
}

CForm::~CForm()
{
    DTRACE();

	LIST_DEL(this,g_FormFirst,g_FormLast,m_FormPrev,m_FormNext);
}

void CForm::StaticInit(void)
{
    g_FormFirst = nullptr;
    g_FormLast = nullptr;
    g_FormCur = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void FormChange(CForm * form)
{
    DTRACE();

	if(g_FormCur) {
		g_FormCur->Leave();
	}
	g_FormCur=form;
    if(g_FormCur) g_FormCur->Enter();
}
