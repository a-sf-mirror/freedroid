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

#include "singlechoicenode.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>

SingleChoiceList::SingleChoiceList(QObject* parent) : QObject(parent), m_by_key(), m_by_name(), m_by_value() {}

SingleChoice* SingleChoiceList::addChoice(const QString& name, const QString& value) {
	Q_ASSERT(!name.isEmpty());
	Q_ASSERT(!value.isEmpty());
	Q_ASSERT(!m_by_name.contains(name));
	Q_ASSERT(!m_by_value.contains(value));
	SingleChoice* choice = new SingleChoice(m_by_key.size(), name, value, this);
	m_by_key += choice;
	m_by_name[choice->name()] = choice;
	m_by_value[choice->value()] = choice;
	return choice;
}

bool SingleChoiceList::contains(const SingleChoice* choice) const {
	Q_ASSERT(choice != 0);
	return m_by_key.value(choice->key()) == choice;
}



SingleChoiceDataNode::SingleChoiceDataNode(const SingleChoiceNodeType* type, SingleChoice* datum, QObject* parent) :
	SingleLineDataNode<SingleChoice*>(type, datum, parent)
{}

const SingleChoiceNodeType* SingleChoiceDataNode::typedType() const {
	const SingleChoiceNodeType* type = dynamic_cast<const SingleChoiceNodeType*>(AbstractDataNode::type());
	Q_ASSERT(type != 0);
	return type;
}

SingleChoiceDataNode* SingleChoiceDataNode::clone(QObject* parent) {
	return new SingleChoiceDataNode(typedType(), datum(), parent);
}

void SingleChoiceDataNode::setDatum(SingleChoice* datum) {
	Q_ASSERT(datum != 0);
	Q_ASSERT(typedType()->choices()->contains(datum));
	if (datum == SingleLineDataNode<SingleChoice*>::datum())
		return;
	SingleLineDataNode<SingleChoice*>::setDatum(datum);
	emit datumChanged(datum);
	emit keyChanged(datum->key());
	emit nameChanged(datum->name());
}

void SingleChoiceDataNode::setDatumByKey(int key) {
	setDatum(typedType()->choices()->byKey(key));
}
void SingleChoiceDataNode::setDatumByName(const QString &name) {
	setDatum(typedType()->choices()->byName(name));
}

void SingleChoiceDataNode::writeNodeData(QTextStream& out) const {
	out << '"' << datum()->value() << '"';
}

QWidget* SingleChoiceDataNode::createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) {
	Q_UNUSED(container);
	Q_UNUSED(tabs);
	QWidget *widget = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(widget);
	QComboBox *input = new QComboBox(widget);
	QLabel *label = 0;
	if (mode == PROPERTY) {
		label = new QLabel(widget);
		label->setText(typedType()->name());
		label->setMinimumWidth(200);
	}
	widget->setToolTip(typedType()->description());

	SingleChoice* choice = 0;
	foreach(choice, typedType()->choices()->list())
		input->addItem(choice->name());

	input->setCurrentIndex(typedType()->defaultDatum()->key());
	input->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	input->setMaximumWidth(300);
	if (mode == PROPERTY)
		layout->addWidget(label, 0);
	layout->addWidget(input, QWIDGETSIZE_MAX);
	layout->addStretch(1);
	layout->setMargin(2);
	widget->setLayout(layout);
	widget->setFocusProxy(input);
	connect(input, SIGNAL(currentIndexChanged(int)),
			this, SLOT(setDatumByKey(int)));
	connect(this, SIGNAL(keyChanged(int)),
			input, SLOT(setCurrentIndex(int)));
	connect(this, SIGNAL(destroyed(QObject*)),
			widget, SLOT(deleteLater()));
	return widget;
}



SingleChoiceNodeType::SingleChoiceNodeType(const QString& name, const QString& description, const QString& tag, const SingleChoiceList* choices, SingleChoice* defaultDatum, bool isWriteCompulsory, QObject* parent) :
	SingleLineNodeType<SingleChoice*>(name, description, tag, defaultDatum, isWriteCompulsory, parent),
	m_choices(choices)
{
	Q_ASSERT(defaultDatum != 0);
	Q_ASSERT(choices->contains(defaultDatum));
}

SingleChoiceNodeType::SingleChoiceNodeType(const QString& name, const QString& description, const QString& tag, const SingleChoiceList* choices, SingleChoice* defaultDatum, QObject* parent) :
	SingleLineNodeType<SingleChoice*>(name, description, tag, defaultDatum, false, parent),
	m_choices(choices)
{
	Q_ASSERT(defaultDatum != 0);
	Q_ASSERT(choices->contains(defaultDatum));
}

SingleChoiceDataNode* SingleChoiceNodeType::createNewDataNode(SingleChoice* const& datum, QObject* parent) const {
	return new SingleChoiceDataNode(this, datum, parent);
}

SingleChoice* SingleChoiceNodeType::parseNodeData(const QString& input) const {
	QString unquoted = input.trimmed();
	if (!input.isEmpty() && unquoted[0] == '"' && unquoted[unquoted.length() - 1] == '"')
		unquoted = unquoted.mid(1, unquoted.length() - 2);
	SingleChoice* choice = choices()->byValue(unquoted);
	if (choice != 0)
		return choice;
	qDebug("Could not parse \"%s\", using default \"%s\" instead.", input.toLocal8Bit().data(), defaultDatum()->value().toLocal8Bit().data());
	return defaultDatum();
}
