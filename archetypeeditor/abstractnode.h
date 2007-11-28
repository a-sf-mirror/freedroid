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
 * \file abstractnode.h
 * \brief The basic building blocks for the datafile hierarchy
 * \author Jon Severinsson (jon@severinsson.net)
 *
 * A datafile is abstracted into a hierarchy of nodes by the FreeDroid Archetype Editor.
 * This file contains the basic, most fundamental, pieces of this hierarchy, the \c AbstractNodeType
 * and the \c AbstractDataNode. These are virtual base classes for a bunch of other node type classes
 * and data node classes, specified both in this and other files.
 *
 * The node type hierarchy and the data node hierarchy are separate, but intertwined. Basically the
 * node type hierarchy describes the syntax of a datafile while the node data hierarchy describes the
 * content of the datafile. The object oriented programmer may think of node types as classes and
 * data nodes as objects, though they are both object as far as C++ is conserned. Those of you not
 * accustomed to object oriented programming will have to resort to the compulsory car analogy:
 * Think of node types as car models and data nodes as actal cars.
 *
 * Anyway, there are many kinds of node types and data nodes, the two most important ones beeing
 * \c SingleLineNodeType and its related \c SingleLineDataNode, specified in sinlgelinenode.h file,
 * as well as the \c CompoundNodeType and its related \c CompoundDataNode, specified in compoundnode.h.
 *
 * A \c SingleLineNodeType is the representation of how a single line in the datafile should look
 * like, while the \c CompoundNodeType is the representation of how multiple lines in the datafile
 * should be grouped together. A \c SingleLineDataNode is then the data from a single line in the,
 * datafile while a \c CompoundDataNode is a structured collection of multiple \c SimpleDataNodes.
 *
 * \c SingleLineNodeType and \c SingleLineDataNode are not intended to be used directly, instead use
 * the apropriate sublcass for the particular data on that line:
 * \li \c IntegerNodeType and \c IntegerDataNode specified in integernode.h
 * \li \c FloatNodeType and \c FloatDataNode specified in floatnode.h
 * \li \c BooleanNodeType and \c BooleanDataNode specified in booleannode.h
 * \li \c StringNodeType and \c StringDataNode specified in stringnode.h
 * \li \c SingleChoiceNodeType and \c SingleChoiseDataNode specified in singlechoicenode.h
 */

#ifndef FDAE_ABSTRACT_NODE_H
#define FDAE_ABSTRACT_NODE_H

#include <QObject>
#include <QWidget>
#include <QTabWidget>
class QString;
class QTextStream;


class CompoundDataNode;
class AbstractNodeType;


/*!
 * \class AbstractDataNode
 * \brief Data for a particular node in the datafile.
 * 
 * This virtual class is used as a base class for all other data nodes. It provides some stuff
 * for the GUI, as well as an interface for writing data to a datafile.
 * 
 * \sa AbstractNodeType
 * \sa SingleLineDataNode
 * \sa CompoundDataNode
 */
class AbstractDataNode : public QObject {
	Q_OBJECT;

	/*!
	 * \brief The node type of this data node
	 */
	Q_PROPERTY(const AbstractNodeType* type READ type);

public:
	/*!
	 * \brief Creates a new data node
	 * \param type The node type of the node type.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	explicit AbstractDataNode(const AbstractNodeType* type, QObject* parent = 0);

	/*!
	 * \brief Writes this data node to a datafile
	 * \param out An output stream to a datafile
	 *
	 * This function serializes the data in this data node to the specified datafile in such a way
	 * that #AbstractNodeType::readData(QString&, QTextStream&, QObject*) can reconstruct an
	 * equivalent data node.
	 * 
	 * \b Note: In some cases, such as some one-line data nodes containing its default datum, this
	 * might mean that no text is written to the file.
	 */
	virtual void writeNode(QTextStream& out) const = 0;

	/*!
	 * \brief Returns a clone of this data node
	 */
	virtual AbstractDataNode* clone(QObject* parent = 0) = 0;

	/*!
	 * \brief Creates a widget suitable for inclusion in a property list.
	 * \param parent The parent widget to the returned widget.
	 *
	 * The data in the widget will be two-way-synchronized with the data in this data node.
	 *
	 * \b Note: This method is only intended to be used by #CompundDataNode::createWidget(WidgetMode, QTabWidget*, QWidget*).
	 */
	inline QWidget* createPropertyWidget(CompoundDataNode* container, QWidget* parent = 0) {
		return createWidget(PROPERTY, container, 0, parent);
	}

	/*!
	 * \brief Creates a widget suitable for inclusion in an entry list.
	 * \param tabs A tab widget in which new tabs can be created if made nessesary by user actions.
	 * \param parent The parent widget to the returned widget.
	 *
	 * The data in the widget will be two-way-synchronized with the data in this data node.
	 *
	 * \b Note: This method is only intended to be used by #CompundDataNode::createWidget(WidgetMode, QTabWidget*, QWidget*).
	 */
	inline QWidget* createEntryWidget(CompoundDataNode* container, QTabWidget* tabs, QWidget* parent = 0) {
		Q_ASSERT(tabs != 0);
		return createWidget(ENTRY, container, tabs, parent);
	}

	/*!
	 * \brief Creates a widget suitable for inclusion in an entry list.
	 * \param tabs The tab widget in which the returned widget will live
	 *
	 * The data in the widget will be two-way-synchronized with the data in this data node.
	 *
	 * \b Note: The returned widget will become the active tab in the specified tab widget upon creation.
	 */
	inline QWidget* createStandaloneWidget(QTabWidget* tabs) {
		Q_ASSERT(tabs != 0);
		return createWidget(STANDALONE, 0, tabs, tabs);
	}

protected:
	/*!
	 * \enum WidgetMode
	 * \brief A enumeration used to tell createWidget in what mode the requested widget is to be used.
	 */
	enum WidgetMode {STANDALONE, PROPERTY, ENTRY};

	/*!
	 * \brief Creats a widget representing this data node.
	 * \param mode In what mode the returned widget is to be used.
	 * \param tabs The tab pane the returned node will live in.
	 * \param parent The parent widget to the returned widget.
	 *
	 * This is the core function for generateing the editor GUI. All subclases of \c AbstractDataNode
	 * must implement this function so that it returns a suitable widget.
	 *
	 * \b Note: This function is only intended to be called from it's convienience methods
	 * #createPropertyWidget, #createEntryWidget and #createStandaloneWidget
	 */
	virtual QWidget* createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) = 0;

signals:
	/*!
	 * \brief Indicates that the data stored in this data node has changed.
	 */
	void dataChanged();

	/*!
	 * \brief This signal is emitted immediately before this node is destroyed, and can not be blocked.
	 *
	 * \b Note: This signal has the same purpose as #QObject::destroyed(QObject*), but will be emitted
	 * just before the \c AbstractDataNode destructor is run. The object destruction sequence for objects
	 * of types inheriting AbstractDataNode is as follows:
	 *
	 * \li DeriveredDataNode::~DeriveredDataNode()
	 * \li AbstractDataNode::destroyed(AbstractDataNode*)
	 * \li AbstractDataNode::~AbstractDataNode()
	 * \li QObject::destroyed(QObject*)
	 * \li QObject::~QObject()
	 */
	void destroyed(AbstractDataNode* node);

public:
	virtual ~AbstractDataNode();

	// Qt property access functions
	virtual const AbstractNodeType* type() const { return m_type; }

private:
	AbstractDataNode(const AbstractDataNode& );
	AbstractDataNode& operator=(const AbstractDataNode&);

	const AbstractNodeType* m_type;
};



/*!
 * \class AbstractNodeType
 * \brief The virtual base class of all node types
 *
 * This virtual class is used as a base class for all other nodes types. It provides some stuff
 * for the GUI, as well as an interface for reading data from a datafile.
 * 
 * \sa AbstractDataNode
 * \sa SingleLineNodeType
 * \sa CompundDataNodeType
 */
class AbstractNodeType : public QObject {
	Q_OBJECT;
	
	/*!
	 * \brief The name of this node type.
	 * This property contains the name of this node type, as it will be presented in the UI.
	 * It has no effect on reading or writing the datafile.
	 */
	Q_PROPERTY(QString name READ name);

	/*!
	 * \brief A one-line-description of the node type.
	 * This property contains a short description of this node type, as it will be presented in the UI.
	 * It has no effect on reading or writing the datafile.
	 */
	Q_PROPERTY(QString description READ description);

public:
	/*!
	 * \brief Creates a new node type
	 * \param name The name of the node type.
	 * \param description A one line description of the node type.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	AbstractNodeType(const QString& name, const QString& description, QObject* parent = 0);

	/*!
	 * \brief Check whether the next datum in the datafile is of this node type
	 * \param next The next line of data in the datafile.
	 *
	 * This function will return true if the type information in \a next maches what this
	 * node type expects. It will not check that the actual data is valid.
	 *
	 * \sa #readData(QString&, QTextStream&, QObject*) const
	 */
	virtual bool canReadData(const QString& next) const = 0;

	/*!
	 * \brief Reads a data node from the datafile
	 * \param next The next line of the datafile
	 * \param remainder The rest of the datafile
	 * \param parent The Qt parent object of the new data node.
	 *
	 * Note that this function expects that \a next contains data apropriate for this
	 * node type, eg that \c canReadData(next) would return \c true.
	 *
	 * When this function returns \a next will contain the last line of the datafile that was used
	 * to create the returned data node and \a remainder will contain the remainder of the datafile.
	 * This means that multi-line node types will modify it's arguments, while single-line node
	 * types won't.
	 *
	 * \b Note: The returned ponter will point to an object on the heap, and must be deleted using
	 * #QObject::deleteLater(). If you specified a parent object Qt will do this automatically
	 * when the parent object is deleted.
	 *
	 * \sa #canReadData(const QString& next) const
	 */
	virtual AbstractDataNode* readData(QString& next, QTextStream& remainder, QObject* parent = 0) const = 0;

	/*!
	 * \brief Creates a new data node of this type
	 * \param parent The Qt parent object of the new data node.
	 *
	 * This functions creates a new data node with this node type in a default state (eg with it's
	 * default data, if any).
	 *
	 * \b Note: The returned ponter will point to an object on the heap, and must be deleted using
	 * #QObject::deleteLater(). If you specified a parent object Qt will do this automatically
	 * when the parent object is deleted.
	 */
	virtual AbstractDataNode* createNewDataNode(QObject* parent = 0) const = 0;

public:
	// Qt property access functions
	inline const QString& name() const { return m_name; }
	inline const QString& description() const { return m_description; }

private:
	AbstractNodeType(const AbstractNodeType&);
	AbstractNodeType& operator=(const AbstractNodeType&);

	const QString m_name;
	const QString m_description;
};

#endif
