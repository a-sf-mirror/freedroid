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

#include "floatnode.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>


FloatDataNode::FloatDataNode(const FloatNodeType* type, double datum, QObject* parent) :
	SingleLineDataNode<double>(type, datum, parent)
{}

const FloatNodeType* FloatDataNode::typedType() const {
	const FloatNodeType* type = dynamic_cast<const FloatNodeType*>(AbstractDataNode::type());
	Q_ASSERT(type != 0);
	return type;
}

FloatDataNode* FloatDataNode::clone(QObject* parent) {
	return new FloatDataNode(typedType(), datum(), parent);
}

void FloatDataNode::setDatum(double datum) {
	if (datum < typedType()->minDatum()) {
		qDebug("%s: Specified datum (%f.4) smaller than minimum datum (%f.4)", typedType()->name().toLocal8Bit().data(), datum, typedType()->minDatum());
		datum = typedType()->minDatum();
	} else if (datum > typedType()->maxDatum()) {
		qDebug("%s: Specified datum (%f.4) larger than maximum datum (%f.4)", typedType()->name().toLocal8Bit().data(), datum, typedType()->maxDatum());
		datum = typedType()->maxDatum();
	}
	if (datum == SingleLineDataNode<double>::datum())
		return;
	SingleLineDataNode<double>::setDatum(datum);
	emit datumChanged(datum);
}

void FloatDataNode::writeNodeData(QTextStream& out) const {
	out << datum();
}

QWidget* FloatDataNode::createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) {
	Q_UNUSED(container);
	Q_UNUSED(tabs);
	QWidget* widget = new QWidget(parent);
	QHBoxLayout* layout = new QHBoxLayout(widget);
	QDoubleSpinBox* input = new QDoubleSpinBox(widget);
	QLabel* label = 0;
	if (mode == PROPERTY) {
		label = new QLabel(widget);
		label->setText(type()->name());
		label->setMinimumWidth(200);
	}
	widget->setToolTip(type()->description());
	input->setDecimals(4);
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
	connect(input, SIGNAL(valueChanged(double)),
			this, SLOT(setDatum(double)));
	connect(this, SIGNAL(datumChanged(double)),
			input, SLOT(setValue(double)));
	connect(this, SIGNAL(destroyed(QObject*)),
			widget, SLOT(deleteLater()));
	return widget;
}


FloatNodeType::FloatNodeType(const QString& name, const QString& description, const QString& tag, double minDatum, double maxDatum, double defaultDatum, bool isWriteCompulsory, QObject* parent) :
	SingleLineNodeType<double>(name, description, tag, defaultDatum, isWriteCompulsory, parent),
	m_min_datum(minDatum),
	m_max_datum(maxDatum)
{
	Q_ASSERT(minDatum <= defaultDatum);
	Q_ASSERT(defaultDatum <= maxDatum);
}

FloatNodeType::FloatNodeType(const QString& name, const QString& description, const QString& tag, double minDatum, double maxDatum, double defaultDatum, QObject* parent) :
	SingleLineNodeType<double>(name, description, tag, defaultDatum, false, parent),
	m_min_datum(minDatum),
	m_max_datum(maxDatum)
{
	Q_ASSERT(minDatum <= defaultDatum);
	Q_ASSERT(defaultDatum <= maxDatum);
}

FloatNodeType::FloatNodeType(const QString& name, const QString& description, const QString& tag, double minDatum, double maxDatum, QObject* parent) :
	SingleLineNodeType<double>(name, description, tag, (minDatum > 0.0 ? minDatum : maxDatum < 0.0 ? maxDatum : 0.0), false, parent),
	m_min_datum(minDatum),
	m_max_datum(maxDatum)
{
	Q_ASSERT(minDatum <= maxDatum);
}

FloatNodeType::FloatNodeType(const QString& name, const QString& description, const QString& tag, double defaultDatum, bool isWriteCompulsory, QObject* parent) :
	SingleLineNodeType<double>(name, description, tag, defaultDatum, isWriteCompulsory, parent),
	m_min_datum(-999999999.9999),
	m_max_datum(999999999.9999)
{
	Q_ASSERT(-999999999.9999 <= defaultDatum);
	Q_ASSERT(defaultDatum <= 999999999.9999);
}

FloatNodeType::FloatNodeType(const QString& name, const QString& description, const QString& tag, double defaultDatum, QObject* parent) :
	SingleLineNodeType<double>(name, description, tag, defaultDatum, false, parent),
	m_min_datum(-999999999.9999),
	m_max_datum(999999999.9999)
{
	Q_ASSERT(-999999999.9999 <= defaultDatum);
	Q_ASSERT(defaultDatum <= 999999999.9999);
}

FloatNodeType::FloatNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent) :
	SingleLineNodeType<double>(name, description, tag, 0.0, false, parent),
	m_min_datum(-999999999.9999),
	m_max_datum(999999999.9999)
{}

FloatDataNode* FloatNodeType::createNewDataNode(const double& data, QObject* parent) const {
	return new FloatDataNode(this, data, parent);
}

double FloatNodeType::parseNodeData(const QString& input) const {
	bool ok;
	double data = input.toDouble(&ok);
	if (!ok) qDebug("Could not parse \"%s\", using default \"%f\" instead.", input.toLocal8Bit().data(), (data = defaultDatum()));
	return data;
}
