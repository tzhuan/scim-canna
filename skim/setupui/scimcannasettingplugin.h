/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/

/*
 *  2005-07-10 Takuro Ashie <ashie@homa.ne.jp>
 *
 *    * Adapt to Canna IMEngine.
 */

#ifndef SCIMCANNASETTINGPLUGIN_H
#define SCIMCANNASETTINGPLUGIN_H

#include "utils/kautocmodule.h"

class ScimCannaSettingPlugin : public KAutoCModule
{
Q_OBJECT
public:
    ScimCannaSettingPlugin(QWidget *parent, 
			   const char */*name*/,
			   const QStringList &args);

    ~ScimCannaSettingPlugin();
private:
    class ScimCannaSettingPluginPrivate;
    ScimCannaSettingPluginPrivate * d;
};

#endif
