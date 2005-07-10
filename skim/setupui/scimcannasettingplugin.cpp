/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/*
 *  2005-06-26 Takuro Ashie <ashie@homa.ne.jp>
 *
 *    * Adapt to Canna IMEngine.
 */

#include "scimcannasettingplugin.h"

#include "canna.h"
#include "cannaui.h"

#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimCannaSettingPlugin> ScimCannaSettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_canna, 
    ScimCannaSettingLoaderFactory( "kcm_skimplugin_scim_canna" ) )

class ScimCannaSettingPlugin::ScimCannaSettingPluginPrivate {
public:
    CannaSettingUI * ui;
};

ScimCannaSettingPlugin::ScimCannaSettingPlugin(QWidget *parent, 
					       const char */*name*/,
					       const QStringList &args)
 : KAutoCModule( ScimCannaSettingLoaderFactory::instance(), 
     parent, args, CannaConfig::self() ),
   d(new ScimCannaSettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-pinyin");
    d->ui = new CannaSettingUI(this);
    setMainWidget(d->ui);
    
    //FIXME: emit toggle signals to update corresponding connected widgets
    d->ui->advancedCBox->setChecked(true);
    d->ui->advancedCBox->toggle();
    d->ui->kcfg__IMEngine_Pinyin_Ambiguities->toggle();
    d->ui->kcfg__IMEngine_Pinyin_Ambiguities->toggle();
}

ScimCannaSettingPlugin::~ScimCannaSettingPlugin() 
{
    KGlobal::locale()->removeCatalogue("skim-scim-pinyin");
}


#include "scimcannasettingplugin.moc"
