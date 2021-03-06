/*
  translatorsmodel.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014-2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Jan Dalheimer <jan.dalheimer@kdab.com>

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

#include "translatorsmodel.h"

#include <core/util.h>

#include "translatorwrapper.h"

using namespace GammaRay;

TranslatorsModel::TranslatorsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}
int TranslatorsModel::columnCount(const QModelIndex &) const
{
  return 3;
}
int TranslatorsModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return m_translators.size();
}
QVariant TranslatorsModel::data(const QModelIndex &index, int role) const
{
  if (role == TranslatorRole) {
    return QVariant::fromValue(m_translators.at(index.row()));
  }
  TranslatorWrapper *trans = m_translators.at(index.row());
  Q_ASSERT(trans);
  if (role == Qt::DisplayRole) {
    if (index.column() == 0) {
      return Util::shortDisplayString(trans->translator());
    } else if (index.column() == 1) {
      return QString(trans->translator()->metaObject()->className());
    } else if (index.column() == 2) {
      return trans->model()->rowCount(QModelIndex());
    }
  } else if (role == Qt::ToolTipRole) {
    return Util::tooltipForObject(trans->translator());
  }
  return QVariant();
}
QVariant TranslatorsModel::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == 0) {
      return tr("Object");
    } else if (section == 1) {
      return tr("Type");
    } else if (section == 2) {
      return tr("Translations");
    }
  }
  return QVariant();
}
Qt::ItemFlags TranslatorsModel::flags(const QModelIndex &index) const
{
  Q_UNUSED(index);
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
TranslatorWrapper *TranslatorsModel::translator(const QModelIndex &index)
    const
{
  return m_translators.at(index.row());
}
void TranslatorsModel::sourceDataChanged()
{
  const int row = m_translators.indexOf(qobject_cast<TranslationsModel *>(sender())->translator());
  if (row == -1) {
    return;
  }
  const QModelIndex index = this->index(row, 2, QModelIndex());
  if (!index.isValid()) {
    return;
  }
  // needed to make sure these things also update
  emit dataChanged(index, index,
                   QVector<int>() << Qt::DisplayRole << Qt::EditRole);
}

void TranslatorsModel::registerTranslator(TranslatorWrapper *translator)
{
  beginInsertRows(QModelIndex(), m_translators.size(), m_translators.size());
  m_translators.append(translator);
  endInsertRows();
  connect(translator->model(), SIGNAL(rowCountChanged()),
          SLOT(sourceDataChanged()));
}
void TranslatorsModel::unregisterTranslator(TranslatorWrapper *translator)
{
  const int index = m_translators.indexOf(translator);
  if (index == -1) {
    qWarning("TranslatorsModel::unregisterTranslator: translator %s is not registered",
             qPrintable(Util::addressToString(translator)));
    return;
  }
  disconnect(translator->model(), 0, this, 0);
  beginRemoveRows(QModelIndex(), index, index);
  m_translators.removeAt(index);
  endRemoveRows();
}
