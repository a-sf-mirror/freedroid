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

#include "integernode.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

IntegerDataNode::IntegerDataNode(const IntegerNodeType* type, int datum, QObject* parent) :
	SingleLineDataNode<int>(type, datum, parent)
{}

const IntegerNodeType* IntegerDataNode::typedType() const {
	const IntegerNodeType* type = dynamic_cast<const IntegerNodeType*>(AbstractDataNode::type());
	Q_ASSERT(type != 0);
	return type;
}

IntegerDataNode* IntegerDataNode::clone(QObject* parent) {
	return new IntegerDataNode(typedType(), datum(), parent);
}

void IntegerDataNode::setDatum(int datum) {
	if (datum < typedType()->minDatum()) {
		qDebug("%s: Specified datum (%i) smaller than minimum datum (%i)", typedType()->name().toLocal8Bit().data(), datum, typedType()->minDatum());
		datum = typedType()->minDatum();
	} else if (datum > typedType()->maxDatum()) {
		qDebug("%s: Specified datum (%i) larger than maximum datum (%i)", typedType()->name().toLocal8Bit().data(), datum, typedType()->maxDatum());
		datum = typedType()->maxDatum();
	}
	if (datum == SingleLineDataNode<int>::datum())
		return;
	SingleLineDataNode<int>::setDatum(datum);
	emit datumChanged(datum);
}

void IntegerDataNode::writeNodeData(QTextStream& out) const {
	out << datum();
}

QWidget* IntegerDataNode::createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) {
	Q_UNUSED(container);
	Q_UNUSED(tabs);
	QWidget* widget = new QWidget(parent);
	QHBoxLayout* layout = new QHBoxLayout(widget);
	QSpinBox* input = new QSpinBox(widget);
	QLabel* label = 0;
	if (mode == PROPERTY) {
		label = new QLabel(widget);
		label->setText(type()->name());
		label->setMinimumWidth(200);
	}
	widget->setToolTip(type()->description());
	input->setMinimum(typedType()->minDatum());
	input->setMaximum(typedType()->maxDatum());
	input->setValue(datum());
	input->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	input->setMaximumWidth(300);
	if (mode == PROPERTY)
		layout->addWidget(label, 0);
	layout->addWidget(input, QWIDGETSIZE_MAX);
	layout->addStretch(1);
	layout->setMargin(2);
	widget->setLayout(layout);
	widget->setFocusProxy(input);
	connect(input, SIGNAL(valueChanged(int)),
			this, SLOT(setDatum(int)));
	connect(this, SIGNAL(datumChanged(int)),
			input, SLOT(setValue(int)));
	connect(this, SIGNAL(destroyed(QObject*)),
			widget, SLOT(deleteLater()));
	return widget;
}



IntegerNodeType::IntegerNodeType(const QString& name, const QString& description, const QString& tag, int minDatum, int maxDatum, int defaultDatum, bool isWriteCompulsory, QObject* parent) :
	SingleLineNodeType<int>(name, description, tag, defaultDatum, isWriteCompulsory, parent),
	m_min_datum(minDatum),
	m_max_datum(maxDatum)
{
	Q_ASSERT(minDatum <= defaultDatum);
	Q_ASSERT(defaultDatum <= maxDatum);
}

IntegerNodeType::IntegerNodeType(const QString& name, const QString& description, const QString& tag, int minDatum, int maxDatum, int defaultDatum, QObject* parent) :
	SingleLineNodeType<int>(name, description, tag, defaultDatum, false, parent),
	m_min_datum(minDatum),
	m_max_datum(maxDatum)
{
	Q_ASSERT(minDatum <= defaultDatum);
	Q_ASSERT(defaultDatum <= maxDatum);
}

IntegerNodeType::IntegerNodeType(const QString& name, const QString& description, const QString& tag, int minDatum, int maxDatum, QObject* parent) :
	SingleLineNodeType<int>(name, description, tag, (minDatum > 0 ? minDatum : maxDatum < 0 ? maxDatum : 0), false, parent),
	m_min_datum(minDatum),
	m_max_datum(maxDatum)
{
	Q_ASSERT(minDatum <= maxDatum);
}

IntegerNodeType::IntegerNodeType(const QString& name, const QString& description, const QString& tag, int defaultDatum, bool isWriteCompulsory, QObject* parent) :
	SingleLineNodeType<int>(name, description, tag, defaultDatum, isWriteCompulsory, parent),
	m_min_datum(-32767),
	m_max_datum(32767)
{
	Q_ASSERT(-32767 <= defaultDatum);
	Q_ASSERT(defaultDatum <= 32767);
}

IntegerNodeType::IntegerNodeType(const QString& name, const QString& description, const QString& tag, int defaultDatum, QObject* parent) :
	SingleLineNodeType<int>(name, description, tag, defaultDatum, false, parent),
	m_min_datum(-32767),
	m_max_datum(32767)
{
	Q_ASSERT(-32767 <= defaultDatum);
	Q_ASSERT(defaultDatum <= 32767);
}

IntegerNodeType::IntegerNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent) :
	SingleLineNodeType<int>(name, description, tag, 0, false, parent),
	m_min_datum(-32767),
	m_max_datum(32767)
{}

IntegerDataNode* IntegerNodeType::createNewDataNode(const int& data, QObject* parent) const {
	return new IntegerDataNode(this, data, parent);
}

int IntegerNodeType::parseNodeData(const QString& input) const {
	bool ok;
	int data = input.toInt(&ok);
	if (!ok) qDebug("Could not parse \"%s\", using default \"%i\" instead.", input.toLocal8Bit().data(), (data = defaultDatum()));
	return data;
}
