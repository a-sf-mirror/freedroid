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

/*!
 * \file choicenode.h
 * \brief The building blocks to store single-choice data in datafiles
 * \author Jon Severinsson (jon@severinsson.net)
 */

#ifndef FDAE_SINGLE_CHOICE_NODE_H
#define FDAE_SINGLE_CHOICE_NODE_H

#include "singlelinenode.h"
#include <QList>
#include <QHash>

class SingleChoice;
class SingleChoiceNodeType;

/*!
 * \class SingleChoiceList
 * \brief A collection of posible choices.
 */
class SingleChoiceList : public QObject  {
	Q_OBJECT;

public:
	/*!
	 * \brief Creates a new list of choices.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	SingleChoiceList(QObject* parent = 0);

	/*!
	 * \brief Adds a new posible choice to this list
	 * \param name The name of this choice, as present in the UI.
	 * \param value The value of this choice, as present in the datafile.
	 */
	SingleChoice* addChoice(const QString& name, const QString& value);

	/*!
	 * \brief Retrieve a choice based on its key.
	 * If a choice with the specfied key exists in this list, return it, otherwise return \c null.
	 */
	inline SingleChoice* byKey(int key) const {
		return m_by_key.value(key);
	}

	/*!
	 * \brief Retrieve a choice based on its name.
	 * If a choice with the specfied name exists in this list, return it, otherwise return \c null.
	 */
	inline SingleChoice* byName(const QString& name) const {
		return m_by_name.value(name);
	}

	/*!
	 * \brief Retrieve a choice based on its value.
	 * If a choice with the specfied value exists in this list, return it, otherwise return \c null.
	 */
	inline SingleChoice* byValue(const QString& value) const {
		return m_by_value.value(value);
	}

	/*!
	 * \brief returns a \c QList containing all valid choices.
	 */
	inline const QList<SingleChoice*> list() const { return m_by_key; }

	/*!
	 * \brief Whether this list contains the specified choice.
	 */
	bool contains(const SingleChoice* choice) const;

private:
	SingleChoiceList(const SingleChoiceList&);
	SingleChoiceList& operator=(const SingleChoiceList&);

	QList<SingleChoice*> m_by_key;
	QHash<QString, SingleChoice*> m_by_name;
	QHash<QString, SingleChoice*> m_by_value;
};


/*!
 * \class SingleChoice
 * \brief A posible choice.
 *
 * This class represents one posible choice in a choice data node.
 */
class SingleChoice : public QObject {
	Q_OBJECT;

	/*!
	 * \brief A numeric key used to reprecent the choice in some places.
	 */
	Q_PROPERTY(int key READ key);

	/*!
	 * \brief The name of this choice
	 * This property contains the name of this choice, as it will be presented in the UI.
	 * It has no effect on reading or writing the datafile.
	 */
	Q_PROPERTY(QString name READ name);

	/*!
	 * \brief The value of this choice
	 * This property contains the value of this choice, as present in the datafile.
	 * The value will be sorounded by citation character in the actual datafile.
	 */
	Q_PROPERTY(QString value READ value);

private:
	friend SingleChoice* SingleChoiceList::addChoice(const QString& name, const QString& value);
	inline SingleChoice(int key, const QString& name, const QString& value, QObject* parent = 0) : QObject(parent), m_key(key), m_name(name), m_value(value) {}

public:
	// Qt property access functions
	inline int key() const { return m_key; }
	inline const QString& name() const { return m_name; }
	inline const QString& value() { return m_value; }

private:
	SingleChoice(const SingleChoice&);
	SingleChoice& operator=(const SingleChoice&);

	const int m_key;
	const QString m_name;
	const QString m_value;
};



/*!
 * \class SingleChoiceDataNode
 * \brief A single single-choice datum from a datafile.
 *
 * \sa SingleChoiceNodeType
 * \sa SingleLineDataNode
 */
class SingleChoiceDataNode : public SingleLineDataNode<SingleChoice*> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_DATA_NODE;

public:
	//! \copydoc SingleLineDataNode::SingleLineDataNode(const SingleLineNodeType, const T&, QObject*)
	SingleChoiceDataNode(const SingleChoiceNodeType* type, SingleChoice* choice, QObject* parent = 0);

	//! \copydoc IntegerDataNode::typedType()
	const SingleChoiceNodeType* typedType() const;

signals:
	//! \copydoc IntegerDataNode::typedType(int)
	void datumChanged(SingleChoice* datum);
	
	/*!
	 * \brief Indicates that the datum stored in this data node has changed.
	 * \param key the key of the new datum
	 */
	void keyChanged(int key);
	
	/*!
	 * \brief Indicates that the datum stored in this data node has changed.
	 * \param name the name of the new datum
	 */
	void nameChanged(const QString &name);

public:
	virtual SingleChoiceDataNode* clone(QObject* parent = 0);

public slots:
	// Qt property access functions
	virtual void setDatum(SingleChoice* datum);

private slots:
	void setDatumByKey(int key);
	void setDatumByName(const QString &name);

protected:
	virtual void writeNodeData(QTextStream& out) const;

	virtual QWidget* createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent);

private:
	SingleChoiceDataNode(const SingleChoiceDataNode&);
	SingleChoiceDataNode& operator=(const SingleChoiceDataNode&);
};


/*!
 * \class SingleChoiceNodeType
 * \brief A single-line node type for storing single-choice data
 *
 * \sa SingleChoiceDataNode
 * \sa SingleLineNodeType
 */
class SingleChoiceNodeType : public SingleLineNodeType<SingleChoice*> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_NODE_TYPE;

	/*!
	 * \brief A list containing all valid choices.
	 */
	Q_PROPERTY(SingleChoiceList choices READ choices);

public:
	/*!
	 * \brief Creates a new single-line node type for storing single-choice data
	 * \param name The name of the node type.
	 * \param description A one line description of the node type.
	 * \param tag The datafile tag of the node type.
	 * \param choices A list containing all valid choices.
	 * \param defaultDatum The default datum for new data nodes of this type.
	 * \param isWriteCompulsory Whether a property with this node type must be written to the datafile even if it only contains the default datum.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	SingleChoiceNodeType(const QString& name, const QString& description, const QString& tag, const SingleChoiceList* choices, SingleChoice* defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c SingleChoiceNodeType(name, description, tag, choices, defaultDatum, false, parent)
	 * \sa BooleanNodeType(const QString&, const QString&, const QString&, bool, bool, QObject*)
	 */
	SingleChoiceNodeType(const QString& name, const QString& description, const QString& tag, const SingleChoiceList* choices, SingleChoice* defaultDatum, QObject* parent = 0);

	//! \copydoc SingleLineNodeType::createNewDataNode(const T&, QObject*) const
	virtual SingleChoiceDataNode* createNewDataNode(SingleChoice* const& datum, QObject* parent = 0) const;

public:
	// Qt property access functions
	inline const SingleChoiceList* choices() const { return m_choices; }

protected:
	virtual SingleChoice* parseNodeData(const QString& input) const;

private:
	SingleChoiceNodeType(const SingleChoiceNodeType&);
	SingleChoiceNodeType& operator=(const SingleChoiceNodeType&);
	
	const SingleChoiceList* m_choices;
};

#endif
