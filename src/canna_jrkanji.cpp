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

#define Uses_SCIM_IMENGINE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <canna/jrkanji.h>
#include "canna_jrkanji.h"
#include "scim_canna_imengine.h"

static unsigned int n_instance = 0;

#define MAX_SIZE 1024

CannaJRKanji::CannaJRKanji (CannaInstance *ci)
    : m_ci (ci),
      m_preediting (false)
{
    char **warn = NULL, **p;
    jrKanjiStatus ks;

    m_iconv.set_encoding ("EUC-JP");

    jrKanjiControl (0, KC_INITIALIZE, (char *) &warn);

    {
        jrKanjiStatusWithValue ksv;
        unsigned char workbuf[MAX_SIZE];
        ksv.ks = &ks;
        ksv.buffer = workbuf;
        ksv.bytes_buffer = MAX_SIZE;
        ksv.val = CANNA_MODE_HenkanMode;
        jrKanjiControl (0, KC_CHANGEMODE, (char *) &ksv);
    }

    for (p = warn; warn && *p; p++) {
        // error
    }

    n_instance++;
}

CannaJRKanji::~CannaJRKanji ()
{
if (n_instance != 0) {
        n_instance--;
        if (n_instance == 0)
            jrKanjiControl (0, KC_FINALIZE, 0);
    }
}

int
CannaJRKanji::translate_key_event (const KeyEvent &key)
{
    switch (key.code) {
    case SCIM_KEY_Return:
        return 0x0d;

    case SCIM_KEY_BackSpace:
        return 0x08;

    case SCIM_KEY_Insert:
        return CANNA_KEY_Insert;

    case SCIM_KEY_Up:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Up;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Up;
        }
        return  CANNA_KEY_Up;

    case SCIM_KEY_Down:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Down;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Down;
        }
        return  CANNA_KEY_Down;

    case SCIM_KEY_Left:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Left;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Left;
        }
        return  CANNA_KEY_Left;

    case SCIM_KEY_Right:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Right;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Right;
        }
        return  CANNA_KEY_Right;

    case SCIM_KEY_Page_Up:
        return CANNA_KEY_Rolldown;

    case SCIM_KEY_Page_Down:
        return CANNA_KEY_Rollup;

    case SCIM_KEY_Home:
        return CANNA_KEY_Home;

    case SCIM_KEY_Help:
        return CANNA_KEY_Help;

    case SCIM_KEY_F1:
    case SCIM_KEY_F2:
    case SCIM_KEY_F3:
    case SCIM_KEY_F4:
    case SCIM_KEY_F5:
    case SCIM_KEY_F6:
    case SCIM_KEY_F7:
    case SCIM_KEY_F8:
    case SCIM_KEY_F9:
    case SCIM_KEY_F10:
    case SCIM_KEY_F11:
    case SCIM_KEY_F12:
        return CANNA_KEY_F1 + key.code - SCIM_KEY_F1;

    case SCIM_KEY_Henkan_Mode:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Xfer;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Xfer;
        }
        return CANNA_KEY_Xfer;

    case SCIM_KEY_Muhenkan:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Nfer;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Nfer;
        }
        return CANNA_KEY_Nfer;

    default:
        if (key.code >= SCIM_KEY_a && key.code <= SCIM_KEY_z){
            if (key.mask & SCIM_KEY_ControlMask)
                return key.code - SCIM_KEY_a + 1;
        }

        if (key.code == SCIM_KEY_at) {
            if (key.code & SCIM_KEY_ControlMask)
                return 0;
        }

        return key.get_ascii_code ();
    }

    return 0xffff;
}

unsigned int
CannaJRKanji::convert_string (WideString &dest,
                              AttributeList &attr_list,
                              const char *str,
                              unsigned int len,
                              unsigned int cur_pos,
                              unsigned int cur_len)
{
    // cut the string
    char left_str[cur_pos + 1];
    char cur_str[cur_len + 1];
    char right_str[len - cur_pos - cur_len + 1];
    strncpy (left_str, str, cur_pos);
    left_str[cur_pos] = '\0';
    strncpy (cur_str, (const char *) (str + cur_pos), cur_len);
    cur_str[cur_len] = '\0';
    strncpy (right_str, (const char *) (str + cur_pos + cur_len),
             len - cur_pos - cur_len);
    right_str[len - cur_pos - cur_len] = '\0';

    // convert
    WideString left, cur, right;
    m_iconv.convert (left,  left_str);
    m_iconv.convert (cur,   cur_str);
    m_iconv.convert (right, right_str);

    // join all string
    dest = left + cur + right;

    // set attributes
    Attribute attr (left.length (), cur.length (), SCIM_ATTR_DECORATE);
    attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
    attr_list.push_back (attr);

    return left.length ();
}

bool
CannaJRKanji::process_key_event (const KeyEvent &key)
{
    int size = 1024, n, ch;
    char buf[1024 + 1];
    jrKanjiStatus ks;

    ch = translate_key_event (key);
    if (ch == 0xffff)
        return false;

    n = jrKanjiString (0, ch, buf, size, &ks);


    // commit string
    if (m_preediting && n > 0) {
        buf[n] = '\0';
        WideString dest;
        m_iconv.convert (dest, buf);
        m_ci->commit_string (dest);
    }


    // guide line string
    if (ks.info & KanjiGLineInfo) {
        WideString dest;
        AttributeList attrs;
        convert_string (dest, attrs,
                        (const char *) ks.gline.line,
                        ks.gline.length,
                        ks.gline.revPos,
                        ks.gline.revLen);
        m_ci->update_aux_string (dest, attrs);
        if (dest.length () > 0)
            m_ci->show_aux_string ();
        else
            m_ci->hide_aux_string ();

    } else {
        m_ci->hide_aux_string ();
        m_ci->update_aux_string (utf8_mbstowcs (""));
    }


    // preedit string
    if (ks.length > 0) {
        WideString dest;
        AttributeList attrs;
        unsigned int pos;
        pos = convert_string (dest, attrs,
                              (const char *) ks.echoStr,
                              ks.length,
                              ks.revPos,
                              ks.revLen);

        m_ci->update_preedit_string (dest, attrs);
        m_ci->update_preedit_caret (pos);

        if (!m_preediting && dest.length () <= 0) {
            return false;
        } else {
            m_preediting = true;
            return true;
        }

    } else if (ks.length == 0) {
        m_ci->update_preedit_string (utf8_mbstowcs (""));

        if (m_preediting) {
            m_preediting = false;
            return true;
        } else {
            return false;
        }

    } else {
        return false;
    }

    return false;
}
