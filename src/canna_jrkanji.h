/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// High level Canna API support.

#ifndef __CANNA_JRKANJI_H__
#define __CANNA_JRKANJI_H__

#define Uses_SCIM_ICONV

#include <scim.h>

using namespace scim;

class CannaInstance;

class CannaJRKanji
{
public:
    CannaJRKanji           (CannaInstance *ci);
    virtual ~CannaJRKanji  (void);

public:
    int  translate_key_event (const KeyEvent &key);
    bool process_key_event   (const KeyEvent &key);

    CannaInstance *m_ci;

private:
    unsigned int convert_string (WideString &dest,
                                 AttributeList &attr_list,
                                 const char *str,
                                 unsigned int len,
                                 unsigned int cur_pos,
                                 unsigned int cur_len);

private:
    IConvert m_iconv;
    bool     m_preediting;
};

#endif /* __CANNA_JRKANJI_H__ */
