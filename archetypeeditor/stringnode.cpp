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

#include "stringnode.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

StringDataNode::StringDataNode(const StringNodeType* type, const QString& datum, QObject* parent) :
	SingleLineDataNode<QString>(type, datum, parent)
{}

const StringNodeType* StringDataNode::typedType() const {
	const StringNodeType* type = dynamic_cast<const StringNodeType*>(AbstractDataNode::type());
	Q_ASSERT(type != 0);
	return type;
}

StringDataNode* StringDataNode::clone(QObject* parent) {
	return new StringDataNode(typedType(), datum(), parent);
}

void StringDataNode::setDatum(const QString& datum) {
	if (datum == SingleLineDataNode<QString>::datum())
		return;
	SingleLineDataNode<QString>::setDatum(datum);
	emit datumChanged(datum);
}

void StringDataNode::writeNodeData(QTextStream& out) const { 
	QString str = datum();
	str.replace('"', "'");
	str.replace('\n', " ");
	str.replace('\r', " ");
	out << '"' << str << '"';
}

QWidget* StringDataNode::createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) {
	Q_UNUSED(container);
	Q_UNUSED(tabs);
	QWidget* widget = new QWidget(parent);
	QHBoxLayout* layout = new QHBoxLayout(widget);
	QLineEdit* input = new QLineEdit(widget);
	QLabel* label = 0;
	if (mode == PROPERTY) {
		label = new QLabel(widget);
		label->setText(type()->name());
		label->setMinimumWidth(200);
	}
	widget->setToolTip(type()->description());
	input->setText(datum());
	if (mode == PROPERTY)
		layout->addWidget(label);
	layout->addWidget(input);
	layout->setMargin(2);
	widget->setLayout(layout);
	widget->setFocusProxy(input);
	connect(input, SIGNAL(textEdited(const QString&)),
			this, SLOT(setDatum(const QString&)));
	connect(this, SIGNAL(datumChanged(const QString&)),
			input, SLOT(setText(const QString&)));
	connect(this, SIGNAL(destroyed(QObject*)),
			widget, SLOT(deleteLater()));
	return widget;
}



StringNodeType::StringNodeType(const QString& name, const QString& description, const QString& tag, const QString& defaultDatum, bool isWriteCompulsory, QObject* parent) :
	SingleLineNodeType<QString>(name, description, tag, defaultDatum, isWriteCompulsory, parent)
{
	Q_ASSERT(!defaultDatum.isNull());
}

StringNodeType::StringNodeType(const QString& name, const QString& description, const QString& tag, const QString& defaultDatum, QObject* parent) :
	SingleLineNodeType<QString>(name, description, tag, defaultDatum, false, parent)
{
	Q_ASSERT(!defaultDatum.isNull());
}

StringNodeType::StringNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent) :
	SingleLineNodeType<QString>(name, description, tag, "", false, parent)
{}

StringDataNode* StringNodeType::createNewDataNode(const QString& data, QObject* parent) const {
	return new StringDataNode(this, data, parent);
}

QString StringNodeType::parseNodeData(const QString& input) const {
	QString data = input.trimmed();
	if (!data.isEmpty() && data[0] == '"' && data[data.length() - 1] == '"')
		return data.mid(1, data.length() - 2);
	return data;
}
