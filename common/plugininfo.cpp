/*
  plugininfo.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014-2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "plugininfo.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QSettings>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QPluginLoader>
#endif

using namespace GammaRay;

PluginInfo::PluginInfo() :
    m_remoteSupport(true),
    m_hidden(false)
{
}

PluginInfo::PluginInfo(const QString& path) :
    m_remoteSupport(true),
    m_hidden(false)
{
    if (QLibrary::isLibrary(path)) {
        initFromJSON(path);
    } else if (path.endsWith(QLatin1String(".desktop"))) {
        initFromDesktopFile(path);
    }
}

QString PluginInfo::path() const
{
    return m_path;
}

QString PluginInfo::id() const
{
    return m_id;
}

QString PluginInfo::interface() const
{
    return m_interface;
}

QStringList PluginInfo::supportedTypes() const
{
    return m_supportedTypes;
}

bool PluginInfo::remoteSupport() const
{
    return m_remoteSupport;
}

QString PluginInfo::name() const
{
    return m_name;
}

bool PluginInfo::isHidden() const
{
    return m_hidden;
}

bool PluginInfo::isValid() const
{
    return !m_path.isEmpty() && !m_interface.isEmpty();
}

void PluginInfo::initFromJSON(const QString& path)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPluginLoader loader(path);
    const QJsonObject metaData = loader.metaData();

    m_interface = metaData.value("IID").toString();
    const QJsonObject customData = metaData.value("MetaData").toObject();

    m_id = customData.value("id").toString();
    m_name = customData.value("name").toString();
    m_remoteSupport = customData.value("remoteSupport").toBool(true);
    m_hidden = customData.value("hidden").toBool(false);

    const QJsonArray types = customData.value("types").toArray();
    m_supportedTypes.reserve(types.size());
    for (auto it = types.constBegin(); it != types.constEnd(); ++it)
      m_supportedTypes.push_back((*it).toString());

    m_path = path;
#else
    Q_UNUSED(path);
#endif
}

void PluginInfo::initFromDesktopFile(const QString& path)
{
    const QFileInfo fi(path);
    QSettings desktopFile(path, QSettings::IniFormat);
    desktopFile.beginGroup(QLatin1String("Desktop Entry"));

    m_id = desktopFile.value("X-GammaRay-Id", fi.baseName()).toString();
    m_interface = desktopFile.value("X-GammaRay-ServiceTypes", QString()).toString();
    m_supportedTypes = desktopFile.value(QLatin1String("X-GammaRay-Types")).toString().split(QLatin1Char(';'), QString::SkipEmptyParts);
    m_name = desktopFile.value(QLatin1String("Name")).toString();
    m_remoteSupport = desktopFile.value(QLatin1String("X-GammaRay-Remote"), true).toBool();
    m_hidden = desktopFile.value(QLatin1String("Hidden"), false).toBool();

    const QString dllBaseName = desktopFile.value(QLatin1String("Exec")).toString();
    if (dllBaseName.isEmpty())
      return;

    foreach (const QString &entry, fi.dir().entryList(QStringList(dllBaseName + QLatin1Char('*')), QDir::Files)) {
        const QString pluginPath = fi.dir().absoluteFilePath(entry);
        if (QLibrary::isLibrary(pluginPath)) {
            m_path = pluginPath;
            break;
        }
    }
}
