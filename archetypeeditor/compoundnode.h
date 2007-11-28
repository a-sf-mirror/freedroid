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

#ifndef FDAE_COMPOUND_NODE_H
#define FDAE_COMPOUND_NODE_H

#include "abstractnode.h"
#include "stringnode.h"

#include <QPointer>
#include <QList>
#include <QHash>
class QGroupBox;
class QScrollArea;

class CompoundNodeType;

class CompoundDataNode : public AbstractDataNode {
	Q_OBJECT;
	Q_PROPERTY(QString name READ name);
	Q_PROPERTY(QString description READ description);

public:
	CompoundDataNode(const CompoundNodeType* type, QObject* parent = 0);
	bool enforcePropertyConsistency();

	//! \copydoc IntegerDataNode::typedType()
	const CompoundNodeType* typedType() const;

	void addEntryNode(AbstractDataNode* node);
	void addPropertyNode(AbstractDataNode* node);

	void removeEntryNode(AbstractDataNode* node);
	void removePropertyNode(AbstractDataNode* node);
	void removeProperty(const AbstractNodeType* property);

	bool containsEntry(const AbstractNodeType* entry) const;
	bool containsEntryNode(AbstractDataNode* node) const;
	bool containsProperty(const AbstractNodeType* property) const;
	bool containsPropertyNode(AbstractDataNode* node) const;

	const QList<QPointer<AbstractDataNode> > entryNodes(const AbstractNodeType* entry) const;
	AbstractDataNode* propertyNode(const AbstractNodeType* property) const;

	//! \copydoc AbstractDataNode::writeNode(QTextStream&) const
	virtual void writeNode(QTextStream& stream) const;

signals:
	void entryNodeAdded(AbstractDataNode* node);
	void propertyStructureChanged();

public:
	virtual CompoundDataNode* clone(QObject* parent = 0);

	// Qt property access functions
	virtual const QString& name() const;
	virtual const QString& description() const;

protected:
	virtual QWidget* createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent);

private:
	bool shouldContainProperty(const AbstractNodeType *property) const;

private slots:
	void entryDestroyedHandler(AbstractDataNode* obj);
	void propertyDestroyedHandler(AbstractDataNode* obj);
	void propertyDataChangedHandler();

private:
	CompoundDataNode(const CompoundDataNode&);
	CompoundDataNode& operator= (const CompoundDataNode&);

	QHash<const AbstractNodeType*, QPointer<AbstractDataNode> > m_property_nodes;
	QHash<const AbstractNodeType*, QList<QPointer<AbstractDataNode> > > m_entry_nodes;
};


class CompoundNodeType : public AbstractNodeType {
	Q_OBJECT;

	Q_PROPERTY(QString startTag READ startTag);
	Q_PROPERTY(QString endTag READ endTag);
	Q_PROPERTY(QList<const AbstractNodeType*> entries READ entries);
	Q_PROPERTY(QList<const AbstractNodeType*> properties READ entries);
	Q_PROPERTY(const StringNodeType* nameProperty READ nameProperty WRITE setNameProperty);
	Q_PROPERTY(const StringNodeType* descriptionProperty READ descriptionProperty WRITE setDescriptionProperty);
	Q_PROPERTY(QString dummyName READ dummyName WRITE setDummyName);
	Q_PROPERTY(QString dummyDescription READ dummyDescription WRITE setDummyDescription);

public:
	CompoundNodeType(const QString& name, const QString& description, const QString& startTag, const QString& endTag, QObject* parent = 0);

	inline void addEntry(const AbstractNodeType* entry) {
		Q_ASSERT(entry != 0);
		Q_ASSERT(!entries().contains(entry));
		Q_ASSERT(!properties().contains(entry));
		m_entries.append(entry);
	}
	inline void addProperty(const AbstractNodeType* property) {
		Q_ASSERT(property != 0);
		Q_ASSERT(!properties().contains(property));
		Q_ASSERT(!entries().contains(property));
		m_properties.append(property);
	}
	inline void setPropertyCondition(const AbstractNodeType* property, const AbstractNodeType* requiredProperty, bool(*conditionSattisfied)(const AbstractDataNode*)) {
		Q_ASSERT(property != 0);
		Q_ASSERT(requiredProperty != 0);
		Q_ASSERT(conditionSattisfied != 0); 
		Q_ASSERT(properties().contains(property));
		Q_ASSERT(properties().contains(requiredProperty));
		Q_ASSERT(!isConditional(property));
		m_property_conditions.insert(property, QPair<const AbstractNodeType*, bool (*)(const AbstractDataNode*)>(requiredProperty, conditionSattisfied));
	}

	inline void addConditionalProperty(const AbstractNodeType* property, const AbstractNodeType* requiredProperty, bool(*conditionSattisfied)(const AbstractDataNode*)) {
		addProperty(property);
		setPropertyCondition(property, requiredProperty, conditionSattisfied);
	}

	inline QPair<const AbstractNodeType*, bool (*)(const AbstractDataNode*)> propertyCondition(const AbstractNodeType* property) const {
		Q_ASSERT(property != 0);
		Q_ASSERT(isConditional(property));
		return m_property_conditions[property];
	}

	inline bool isConditional(const AbstractNodeType* property) const {
		Q_ASSERT(property != 0);
		return m_property_conditions.contains(property);
	}

	//! \copydoc AbstractNodeType::canReadData(const QString&) const
	virtual bool canReadData(const QString& next) const { return next.startsWith(startTag()); }

	//! \copydoc AbstractNodeType::readData(QString&, QTextStream&, QObject*) const
	virtual CompoundDataNode* readData(QString& next, QTextStream& remainder, QObject* parent = 0) const;

	//! \copydoc AbstractNodeType::createNewDataNode(QObject* parent) const
	virtual CompoundDataNode* createNewDataNode(QObject* parent = 0) const;

	CompoundDataNode* createNewDummyDataNode(QObject* parent = 0) const;

public:
	// Qt property access functions
	inline const QString& startTag() const { return m_start_tag; }
	inline const QString& endTag() const { return m_end_tag; }
	inline const QList<const AbstractNodeType*> entries() const { return m_entries; }
	inline const QList<const AbstractNodeType*> properties() const { return m_properties; }
	inline const StringNodeType* nameProperty() const { return m_name_property; }
	inline const StringNodeType* descriptionProperty() const { return m_description_property; }
	inline const QString& dummyName() const { return m_dummy_name; }
	inline const QString& dummyDescription() const { return m_dummy_description; };

	inline void setNameProperty(const StringNodeType* property) {
		Q_ASSERT(property != 0);
		Q_ASSERT(nameProperty() == 0);
		Q_ASSERT(properties().contains(property));
		m_name_property = property;
	}
	inline void setDescriptionProperty(const StringNodeType* property) {
		Q_ASSERT(property != 0);
		Q_ASSERT(descriptionProperty() == 0);
		Q_ASSERT(properties().contains(property));
		m_description_property = property;
	}

	inline void setDummyName(const QString& name) {
		Q_ASSERT(nameProperty() != 0);
		Q_ASSERT(!name.isNull());
		Q_ASSERT(name != nameProperty()->defaultDatum());
		m_dummy_name = name;
	}

	inline void setDummyDescription(const QString& description) {
		Q_ASSERT(!dummyName().isNull());
		Q_ASSERT(descriptionProperty() != 0);
		Q_ASSERT(!description.isNull());
		m_dummy_description = description;
	}

private:
	CompoundNodeType(const CompoundNodeType&);
	CompoundNodeType& operator= (const CompoundNodeType&);

	const QString m_start_tag;
	const QString m_end_tag;

	const StringNodeType* m_name_property;
	const StringNodeType* m_description_property;
	QString m_dummy_name;
	QString m_dummy_description;

	QList<const AbstractNodeType*> m_entries;
	QList<const AbstractNodeType*> m_properties;

	// This holds the mapping  conditional property -> (required property  x  requirement sattisfied predicate)
	QHash<const AbstractNodeType*, QPair<const AbstractNodeType*, bool (*)(const AbstractDataNode*)> > m_property_conditions;
};

#endif
