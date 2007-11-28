/***************************************************************************
 *   Copyright (C) 2007 by Jon Severinsson (jon@severinsson.net)           *
 *                                                                         *
 *   This file is part of the FreeDroid Archetype Editor                   *
 *                                                                         *
 *   FreeDroid Archetype Editor is free software; you can redistribute     *
 *   it and/or modify it under the terms of the GNU General Public License *
 *   Version 2, as published by the Free Software Foundation.              *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 *   In addition, as a special exception, the copyright holders give       *
 *   permission to link the code of this program with any edition of       *
 *   the Qt library by Trolltech AS, Norway (or with modified versions     *
 *   of Qt that use the same license as Qt), and distribute linked         *
 *   combinations including the two.  You must obey the GNU General        *
 *   Public License in all respects for all of the code used other than    *
 *   Qt.  If you modify this file, you may extend this exception to        *
 *   your version of the file, but you are not obligated to do so.  If     *
 *   you do not wish to do so, delete this exception statement from        *
 *   your version.                                                         *
 ***************************************************************************/

#include "booleannode.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>

BooleanDataNode::BooleanDataNode(const BooleanNodeType* type, bool datum, QObject* parent) :
	SingleLineDataNode<bool>(type, datum, parent)
{}

const BooleanNodeType* BooleanDataNode::typedType() const {
	const BooleanNodeType* type = dynamic_cast<const BooleanNodeType*>(AbstractDataNode::type());
	Q_ASSERT(type != 0);
	return type;
}

BooleanDataNode* BooleanDataNode::clone(QObject* parent) {
	return new BooleanDataNode(typedType(), datum(), parent);
}

void BooleanDataNode::setDatum(bool datum) {
	if (datum == SingleLineDataNode<bool>::datum())
		return;
	SingleLineDataNode<bool>::setDatum(datum);
	emit datumChanged(datum);
}

void BooleanDataNode::setDatum(int datum) {
	setDatum(datum != 0);
}

void BooleanDataNode::writeNodeData(QTextStream& out) const {
	out << (datum() ? "\"yes\"" : "\"no\"");
}

QWidget* BooleanDataNode::createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) {
	Q_UNUSED(container);
	Q_UNUSED(tabs);
	QWidget* widget = new QWidget(parent);
	QHBoxLayout* layout = new QHBoxLayout(widget);
	QCheckBox* input = new QCheckBox(widget);
	QLabel* label = 0;
	if (mode == PROPERTY) {
		label = new QLabel(widget);
		label->setText(type()->name());
		label->setMinimumWidth(200);
	}
	widget->setToolTip(type()->description());
	input->setChecked(datum());
	if (mode == PROPERTY)
		layout->addWidget(label);
	layout->addWidget(input);
	layout->addStretch(1);
	layout->setMargin(2);
	widget->setLayout(layout);
	widget->setFocusProxy(input);
	connect(input, SIGNAL(stateChanged(int)),
			this, SLOT(setDatum(int)));
	connect(this, SIGNAL(datumChanged(bool)),
			input, SLOT(setChecked(bool)));
	connect(this, SIGNAL(destroyed(QObject*)),
			widget, SLOT(deleteLater()));
	return widget;
}



BooleanNodeType::BooleanNodeType(const QString& name, const QString& description, const QString& tag, bool defaultDatum, bool isWriteCompulsory, QObject* parent) :
	SingleLineNodeType<bool>(name, description, tag, defaultDatum, isWriteCompulsory, parent)
{}

BooleanNodeType::BooleanNodeType(const QString& name, const QString& description, const QString& tag, bool defaultDatum, QObject* parent) :
	SingleLineNodeType<bool>(name, description, tag, defaultDatum, false, parent)
{}

BooleanNodeType::BooleanNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent) :
	SingleLineNodeType<bool>(name, description, tag, false, false, parent)
{}

BooleanDataNode* BooleanNodeType::createNewDataNode(const bool& data, QObject* parent) const {
	return new BooleanDataNode(this, data, parent);
}

bool BooleanNodeType::parseNodeData(const QString& input) const {
	QString unquoted = input.trimmed();
	if (!input.isEmpty() && unquoted[0] == '"' && unquoted[unquoted.length() - 1] == '"')
		unquoted = unquoted.mid(1, unquoted.length() - 2);
	if (unquoted == "yes")
		return true;
	if (unquoted == "no")
		return false;
	qDebug("Could not parse \"%s\", using default \"%s\" instead.", input.toLocal8Bit().data(), defaultDatum() ? "\"yes\"" : "\"no\"");
	return defaultDatum();
}
