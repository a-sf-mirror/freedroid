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

#include "compoundnode.h"

#include "compoundwidget.h"
#include <QWidget>
#include <QScrollArea>
#include <QGroupBox>
#include <QVBoxLayout>

CompoundDataNode::CompoundDataNode(const CompoundNodeType* type, QObject* parent) :
	AbstractDataNode(type, parent),
	m_property_nodes(),
	m_entry_nodes()
{
	connect(this, SIGNAL(propertyStructureChanged()),
			this, SIGNAL(dataChanged()));
	connect(this, SIGNAL(entryNodeAdded(AbstractDataNode*)),
			this, SIGNAL(dataChanged()));
}

bool CompoundDataNode::enforcePropertyConsistency() {
	bool modified = false;
	blockSignals(true);
	const AbstractNodeType* property;
	foreach(property, typedType()->properties()) {
		Q_ASSERT(property != 0);
		if (!containsProperty(property) && shouldContainProperty(property)) {
			addPropertyNode(property->createNewDataNode(this));
			modified = true;
		}
		if (containsProperty(property) && !shouldContainProperty(property)) {
			removeProperty(property);
			modified = true;
		}
	}
	blockSignals(false);
	return modified;
}

const CompoundNodeType* CompoundDataNode::typedType() const {
	const CompoundNodeType* type = dynamic_cast<const CompoundNodeType*>(AbstractDataNode::type());
	Q_ASSERT(type != 0);
	return type;
}


CompoundDataNode* CompoundDataNode::clone(QObject* parent) {
	CompoundDataNode* clone = new CompoundDataNode(typedType(), parent);
	
	AbstractDataNode* node = 0;
	foreach(node, m_property_nodes)
		if (node != 0)
			clone->addPropertyNode(node->clone(clone));
	QList<QPointer<AbstractDataNode> > list;
	foreach(list, m_entry_nodes);
		foreach(node, list)
			if (node != 0)
				clone->addEntryNode(node->clone(clone));

	if (typedType()->nameProperty() == 0)
		return clone;

	StringDataNode* nameNode = dynamic_cast<StringDataNode*>(clone->propertyNode(typedType()->nameProperty()));
	nameNode->setDatum(nameNode->datum() + " Copy");
	return clone;
}

void CompoundDataNode::addEntryNode(AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	Q_ASSERT(typedType()->entries().contains(node->type()));
	Q_ASSERT(!containsEntryNode(node));

	// Try to figure out if this is a dummy entry
	CompoundDataNode* cnode = dynamic_cast<CompoundDataNode*>(node);
	if (cnode != 0 && !cnode->typedType()->dummyName().isNull()) {
		StringDataNode* nameNode = dynamic_cast<StringDataNode*>(cnode->propertyNode(cnode->typedType()->nameProperty()));
		Q_ASSERT(nameNode != 0);
		if (nameNode->datum() == cnode->typedType()->dummyName()) {
			m_entry_nodes[cnode->type()] += 0;
			cnode->deleteLater();
			return;
		}
	}

	// This wans't a dummy entry, so add it propperly.
	node->setParent(this);
	m_entry_nodes[node->type()] += node;
	connect(node, SIGNAL(destroyed(AbstractDataNode*)),
			this, SLOT(entryDestroyedHandler(AbstractDataNode*)));
	connect(node, SIGNAL(dataChanged()),
			this, SIGNAL(dataChanged()));
	emit entryNodeAdded(node);
}

void CompoundDataNode::addPropertyNode(AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	Q_ASSERT(typedType()->properties().contains(node->type()));
	Q_ASSERT(!containsProperty(node->type()));

	node->setParent(this);
	m_property_nodes[node->type()] = node;
	connect(node, SIGNAL(destroyed(AbstractDataNode*)),
			this, SLOT(propertyDestroyedHandler(AbstractDataNode*)));
	connect(node, SIGNAL(dataChanged()),
			this, SIGNAL(dataChanged()));
	connect(node, SIGNAL(dataChanged()),
			this, SLOT(propertyDataChangedHandler()));
	emit propertyStructureChanged();
}

void CompoundDataNode::removeEntryNode(AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	Q_ASSERT(typedType()->entries().contains(node->type()));
	Q_ASSERT(containsEntryNode(node));
	node->deleteLater();
}

void CompoundDataNode::removePropertyNode(AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	Q_ASSERT(typedType()->properties().contains(node->type()));
	Q_ASSERT(containsPropertyNode(node));
	node->deleteLater();
}

void CompoundDataNode::removeProperty(const AbstractNodeType* property) {
	Q_ASSERT(property != 0);
	Q_ASSERT(typedType()->properties().contains(property));
	Q_ASSERT(containsProperty(property));
	propertyNode(property)->deleteLater();
}

bool CompoundDataNode::containsEntry(const AbstractNodeType* entry) const {
	Q_ASSERT(entry != 0);
	Q_ASSERT(typedType()->entries().contains(entry));
	return !entryNodes(entry).isEmpty();
}

bool CompoundDataNode::containsEntryNode(AbstractDataNode* node) const {
	Q_ASSERT(node != 0);
	Q_ASSERT(typedType()->entries().contains(node->type()));
	return entryNodes(node->type()).contains(node);
}

bool CompoundDataNode::containsProperty(const AbstractNodeType* property) const {
	Q_ASSERT(property != 0);
	Q_ASSERT(typedType()->properties().contains(property));
	return propertyNode(property) != 0;
}

bool CompoundDataNode::containsPropertyNode(AbstractDataNode* node) const {
	Q_ASSERT(node != 0);
	Q_ASSERT(typedType()->properties().contains(node->type()));
	return propertyNode(node->type()) == node;
}

const QList<QPointer<AbstractDataNode> > CompoundDataNode::entryNodes(const AbstractNodeType* entry) const {
	Q_ASSERT(entry != 0);
	Q_ASSERT(typedType()->entries().contains(entry));
	return m_entry_nodes[entry];
}

AbstractDataNode* CompoundDataNode::propertyNode(const AbstractNodeType* property) const {
	Q_ASSERT(property != 0);
	Q_ASSERT(typedType()->properties().contains(property));
	return m_property_nodes[property];
}

void CompoundDataNode::writeNode(QTextStream& stream) const {
	stream << typedType()->startTag() << "\n";

	const AbstractNodeType* property = 0;
	foreach(property, typedType()->properties()) {
		// ToDo: Replace SingleLineNodeType::isWriteCompulsory with CompoundNodeType::mustWriteDefaultPropertyNodes(const AbstractNodeType*)
	
		if (containsProperty(property))
			propertyNode(property)->writeNode(stream);
		else if (dynamic_cast<const CompoundNodeType*>(property) == 0){
			AbstractDataNode* node = property->createNewDataNode();
			node->writeNode(stream);
			node->deleteLater();
		}
	}

	if (!typedType()->properties().isEmpty() && !typedType()->entries().isEmpty())
		stream << "\n";

	const AbstractNodeType* entry = 0;
	foreach(entry, typedType()->entries()) {
		// ToDo: Introduce the concept of CompoundNodeType::mustWriteDummyEntryNodes(const AbstractNodeType*)
		CompoundDataNode* dummyNode = 0;

		// If dummy entries should be printed, get a template.
		const CompoundNodeType* centry = dynamic_cast<const CompoundNodeType*>(entry);
		if (centry != 0 && !centry->dummyName().isNull()) {
			Q_ASSERT(centry->nameProperty() != 0);
			dummyNode = centry->createNewDummyDataNode();
		}

		int i = 0;
		const AbstractDataNode* node = 0;
		foreach(node, entryNodes(entry)) {
			stream << "\n"; // add some space
			stream << "// " << entry->name() << " No. " << i++ << "\n";
			if (node != 0)
				node->writeNode(stream);
			else if (dummyNode != 0)
				dummyNode->writeNode(stream);
		}

		if (dummyNode != 0)
			dummyNode->deleteLater();
		stream << "\n"; // add some space
	}

	stream << typedType()->endTag() << endl;
}

const QString& CompoundDataNode::name() const {
	if (typedType()->nameProperty() == 0)
		return typedType()->name();
	StringDataNode* nameNode = dynamic_cast<StringDataNode*>(propertyNode(typedType()->nameProperty()));
	Q_ASSERT(nameNode != 0);
	return nameNode->datum();
}

const QString& CompoundDataNode::description() const {
	if (typedType()->descriptionProperty() == 0)
		return typedType()->description();
	StringDataNode* descriptionNode = dynamic_cast<StringDataNode*>(propertyNode(typedType()->descriptionProperty()));
	Q_ASSERT(descriptionNode != 0);
	return descriptionNode->datum();
}

bool CompoundDataNode::shouldContainProperty(const AbstractNodeType* property) const {
	Q_ASSERT(property != 0);
	Q_ASSERT(typedType()->properties().contains(property));
	if (!typedType()->isConditional(property))
		return true;
	return typedType()->propertyCondition(property).second(propertyNode(typedType()->propertyCondition(property).first));
}

void CompoundDataNode::entryDestroyedHandler(AbstractDataNode* obj) {
	Q_UNUSED(obj);
	// Thanks to the Qt object hierarchy and the smart QPointer nothing has to be done atm.
	emit dataChanged();
}

void CompoundDataNode::propertyDestroyedHandler(AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	const AbstractNodeType* property = node->type();
	Q_ASSERT(typedType()->properties().contains(property));
	Q_ASSERT(containsProperty(property));
	Q_ASSERT(containsPropertyNode(node));
	m_property_nodes.remove(property);
	enforcePropertyConsistency();
	emit propertyStructureChanged();
}

void CompoundDataNode::propertyDataChangedHandler() {
	bool moddified = enforcePropertyConsistency();
	if (moddified)
		emit propertyStructureChanged();
}



CompoundNodeType::CompoundNodeType(const QString& name, const QString& description, const QString& startTag, const QString& endTag, QObject* parent) :
	AbstractNodeType(name, description, parent),
	m_start_tag(startTag),
	m_end_tag(endTag)
{
	Q_ASSERT(!startTag.isEmpty());
	Q_ASSERT(!endTag.isEmpty());
	m_name_property = 0;
	m_description_property = 0;
}

CompoundDataNode* CompoundNodeType::readData(QString& next, QTextStream& remainder, QObject* parent) const {
	Q_ASSERT(canReadData(next));
	CompoundDataNode* data = new CompoundDataNode(this, parent);
outer:
	forever {
		next = remainder.readLine();
		if (next.isNull()) {
			qWarning("CompoundNodeType::readData: Premature end of stream, last node might be incomlete");
			data->enforcePropertyConsistency();
			return data;
		}
		if (next.isEmpty() || next.startsWith("//"))
			continue;
		if (next == endTag()) {
			data->enforcePropertyConsistency();
			return data;
		}
		const AbstractNodeType* entry = 0;
		foreach(entry, entries()) {
			Q_ASSERT(entry != 0);
			if (entry->canReadData(next)) {
				AbstractDataNode* node = entry->readData(next, remainder, data);
				data->addEntryNode(node);
				goto outer; //continue the outer loop, and not the inner one.
			}
		}
		const AbstractNodeType* property = 0;
		foreach(property, properties()) {
			Q_ASSERT(property != 0);
			if (property->canReadData(next)) {
				AbstractDataNode* node = property->readData(next, remainder, data);
				data->addPropertyNode(node);
				goto outer; //continue the outer loop, and not the inner one.
			}
		}
		//ToDo: Remove if statement. It's only here to remove noice in debug output...
		if (next.startsWith("Position_"))
			continue;
		qWarning("A problem occured while loading the datafile: A line with unknown syntax found.");
		qDebug("The node currently being read: %s", name().toLocal8Bit().data());
		qDebug("The offending line: %s", next.toLocal8Bit().data());
	}
	// We should return from within the loop, so this should never happen.
	Q_ASSERT(false);
	return data;
}

CompoundDataNode* CompoundNodeType::createNewDataNode(QObject* parent) const {
	CompoundDataNode* node = new CompoundDataNode(this, parent);
	node->enforcePropertyConsistency();
	return node;
}

CompoundDataNode* CompoundNodeType::createNewDummyDataNode(QObject* parent) const {
	Q_ASSERT(nameProperty() != 0);
	Q_ASSERT(!dummyName().isNull());
	CompoundDataNode* node = new CompoundDataNode(this, parent);
	node->addPropertyNode(nameProperty()->createNewDataNode(dummyName()));
	if (descriptionProperty() != 0 && !dummyDescription().isNull())
		node->addPropertyNode(descriptionProperty()->createNewDataNode(dummyDescription()));
	node->enforcePropertyConsistency();
	return node;
}

QWidget* CompoundDataNode::createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) {
	Q_ASSERT(mode == PROPERTY ? tabs == 0 : tabs != 0);
	Q_ASSERT(mode == STANDALONE ? container == 0 : container != 0);
	Q_ASSERT(mode == PROPERTY ? container->containsPropertyNode(this) : true);
	Q_ASSERT(mode == ENTRY ? container->containsEntryNode(this) : true);

	if (mode == ENTRY)
		return new EntryWidget(this, container, tabs, parent);

	QWidget* widget = 0;
	QWidget* surface = 0;

	if (typedType()->entries().isEmpty()) {
		surface = new PropertyListWidget(this);
	} else if (typedType()->properties().isEmpty() && typedType()->entries().size() == 1) {
		surface = new EntryListWidget(this, typedType()->entries().first(), tabs);
	} else {
		surface = new QWidget();
		QVBoxLayout* layout = new QVBoxLayout(surface);
		layout->setMargin(2);
		if (!typedType()->entries().isEmpty())
			layout->addWidget(new PropertyListWidget(this, surface));
		const AbstractNodeType* entry;
		foreach(entry, typedType()->entries()) {
			QGroupBox* box = new QGroupBox(surface);
			box->setTitle(entry->name());
			box->setToolTip(entry->description());
			QVBoxLayout* boxLayout = new QVBoxLayout(box);
			boxLayout->setMargin(0);
			boxLayout->addWidget(new EntryListWidget(this, entry, tabs, box));
			box->setLayout(boxLayout);
			layout->addWidget(box);
		}
		surface->setLayout(layout);
	}

	if (mode == STANDALONE) {
		Q_ASSERT(tabs == parent);
		QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(surface->layout());
		Q_ASSERT(layout != 0);
		layout->addStretch(1);

		QScrollArea* scroll = new QScrollArea(parent);
		scroll->setWidget(surface);
		scroll->setWidgetResizable(true);
		widget = scroll;
	}
	if (mode == PROPERTY) {
		QGroupBox* box = new QGroupBox(parent);
		box->setTitle(name());
		box->setToolTip(description());
		QVBoxLayout* boxLayout = new QVBoxLayout(box);
		boxLayout->setMargin(0);
		surface->setParent(box);
		boxLayout->addWidget(surface);
		box->setLayout(boxLayout);
		widget = box;
	}

	connect(this, SIGNAL(destroyed(QObject*)),
			widget, SLOT(deleteLater()));

	Q_ASSERT(widget != 0);
	return widget;
}
