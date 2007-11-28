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
 * \file booleannode.h
 * \brief The building blocks to store boolean data in datafiles
 * \author Jon Severinsson (jon@severinsson.net)
 */

#ifndef FDAE_BOOLEAN_NODE_H
#define FDAE_BOOLEAN_NODE_H

#include "singlelinenode.h"

class BooleanNodeType;

/*!
 * \class BooleanDataNode
 * \brief A single boolean datum from a datafile.
 *
 * \sa BooleanNodeType
 * \sa SingleLineDataNode
 */
class BooleanDataNode : public SingleLineDataNode<bool> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_DATA_NODE;

public:
	//! \copydoc SingleLineDataNode::SingleLineDataNode(const SingleLineNodeType, const T&, QObject*)
	BooleanDataNode(const BooleanNodeType* type, bool datum, QObject* parent = 0);

	//! \copydoc IntegerDataNode::typedType()
	const BooleanNodeType* typedType() const;

signals:
	//! \copydoc IntegerDataNode::typedType(int)
	void datumChanged(bool datum);

public:
	virtual BooleanDataNode* clone(QObject* parent = 0);

public slots:
	// Qt property access functions
	virtual void setDatum(bool datum);

private slots:
	virtual void setDatum(int datum);

protected:
	virtual void writeNodeData(QTextStream& out) const;

	virtual QWidget* createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent);

private:
	BooleanDataNode(const BooleanDataNode&);
	BooleanDataNode& operator=(const BooleanDataNode&);
};

/*!
 * \class BooleanNodeType
 * \brief A single-line node type for storing storing boolean data
 *
 * \sa BooleanDataNode
 * \sa SingleLineNodeType
 */
class BooleanNodeType : public SingleLineNodeType<bool> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_NODE_TYPE;

public:
	/*!
	 * \brief Creates a new single-line node type for storing boolean data
	 * \param name The name of the node type.
	 * \param description A one line description of the node type.
	 * \param tag The datafile tag of the node type.
	 * \param defaultDatum The default datum for new data nodes of this type.
	 * \param isWriteCompulsory Whether a property with this node type must be written to the datafile even if it only contains the default datum.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	BooleanNodeType(const QString& name, const QString& description, const QString& tag, bool defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c BooleanNodeType(name, description, tag, defaultDatum, false, parent)
	 * \sa BooleanNodeType(const QString&, const QString&, const QString&, bool, bool, QObject*)
	 */
	BooleanNodeType(const QString& name, const QString& description, const QString& tag, bool defaultDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c BooleanNodeType(name, description, tag, false, false, parent)
	 * \sa BooleanNodeType(const QString&, const QString&, const QString&, bool, bool, QObject*)
	 */
	BooleanNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent = 0);

	//! \copydoc SingleLineNodeType::createNewDataNode(const T&, QObject*) const
	virtual BooleanDataNode* createNewDataNode(const bool& datum, QObject* parent = 0) const;

protected:
	virtual bool parseNodeData(const QString& input) const;

private:
	BooleanNodeType(const BooleanNodeType&);
	BooleanNodeType& operator=(const BooleanNodeType&);
};

#endif
