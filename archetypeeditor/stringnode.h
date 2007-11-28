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
 * \file stringnode.h
 * \brief The building blocks to store textual data in datafiles
 * \author Jon Severinsson (jon@severinsson.net)
 */

#ifndef FDAE_STRING_NODE_H
#define FDAE_STRING_NODE_H

#include "singlelinenode.h"

class StringNodeType;

/*!
 * \class StringDataNode
 * \brief A single text string datum from a datafile.
 * 
 * \sa StringNodeType
 * \sa SingleLineDataNode
 */
class StringDataNode : public SingleLineDataNode<QString> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_DATA_NODE;

public:
	//! \copydoc SingleLineDataNode::SingleLineDataNode(const SingleLineNodeType, const T&, QObject*)
	StringDataNode(const StringNodeType* type, const QString& datum, QObject* parent = 0);

	//! \copydoc IntegerDataNode::typedType()
	const StringNodeType* typedType() const;

signals:
	//! \copydoc IntegerDataNode::typedType(int)
	void datumChanged(const QString& datum);

public:
	virtual StringDataNode* clone(QObject* parent = 0);

public slots:
	// Qt property access functions
	virtual void setDatum(const QString& datum);

protected:
	virtual void writeNodeData(QTextStream& out) const;

	virtual QWidget* createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent);

private:
	StringDataNode(const StringDataNode&);
	StringDataNode& operator=(const StringDataNode&);
};



/*!
 * \class StringNodeType
 * \brief A single-line node type for storing storing single-line text strings
 *
 * \sa StringDataNode
 * \sa SingleLineNodeType
 */
class StringNodeType : public SingleLineNodeType<QString> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_NODE_TYPE;

public:
	/*!
	 * \brief Creates a new single-line node type for storing single-line text strings
	 * \param name The name of the node type.
	 * \param description A one line description of the node type.
	 * \param tag The datafile tag of the node type.
	 * \param defaultDatum The default datum for new data nodes of this type.
	 * \param isWriteCompulsory Whether a property with this node type must be written to the datafile even if it only contains the default datum.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	StringNodeType(const QString& name, const QString& description, const QString& tag, const QString& defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c BooleanNodeType(name, description, tag, defaultDatum, false, parent)
	 * \sa StringNodeType(const QString&, const QString&, const QString&, const QString&, bool, QObject*)
	 */
	StringNodeType(const QString& name, const QString& description, const QString& tag, const QString& defaultDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c BooleanNodeType(name, description, tag, "", false, parent)
	 * \sa StringNodeType(const QString&, const QString&, const QString&, const QString&, bool, QObject*)
	 */
	StringNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent = 0);

	//! \copydoc SingleLineNodeType::createNewDataNode(const T&, QObject*) const
	virtual StringDataNode* createNewDataNode(const QString& datum, QObject* parent = 0) const;

protected:
	virtual QString parseNodeData(const QString& input) const;

private:
	StringNodeType(const StringNodeType&);
	StringNodeType& operator=(const StringNodeType&);
};

#endif
