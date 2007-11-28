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
 * \file integernode.h
 * \brief The building blocks to store integer data in datafiles
 * \author Jon Severinsson (jon@severinsson.net)
 */

#ifndef FDAE_INTEGER_NODE_H
#define FDAE_INTEGER_NODE_H

#include "singlelinenode.h"

class IntegerNodeType;

/*!
 * \class IntegerDataNode
 * \brief A single integer datum from a datafile.
 *
 * \sa IntegerNodeType
 * \sa SingleLineDataNode
 */
class IntegerDataNode : public SingleLineDataNode<int> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_DATA_NODE;

public:
	//! \copydoc SingleLineDataNode::SingleLineDataNode(const SingleLineNodeType, const T&, QObject*)
	IntegerDataNode(const IntegerNodeType* type, int datum, QObject* parent = 0);

	/*!
	 * \brief Returns the node type of this data node as it's actual C++ type.
	 * This function returns the same ponter as #AbstractDataNode::type() does, but it returns it
	 * typed as it's actual C++ type rather than as an AbstractNodeType. This is realy just a
	 * workaround for the limitation of the linear compiler C++ inherited from C.
	 */
	const IntegerNodeType* typedType() const;

signals:
	/*!
	 * \brief Indicates that the datum stored in this data node has changed.
	 * \param datum the new datum
	 */
	void datumChanged(int datum);

public:
	virtual IntegerDataNode* clone(QObject* parent = 0);

public slots:
	// Qt property access functions
	virtual void setDatum(int datum);

protected:
	virtual void writeNodeData(QTextStream& out) const;

	virtual QWidget* createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent);

private:
	IntegerDataNode(const IntegerDataNode&);
	IntegerDataNode& operator=(const IntegerDataNode&);
};

/*!
 * \class IntegerNodeType
 * \brief A single-line node type for storing itegers
 *
 * \sa IntegerDataNode
 * \sa SingleLineNodeType
 */
class IntegerNodeType : public SingleLineNodeType<int> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_NODE_TYPE;

	/*!
	 * \brief The smallest valid datum for data nodes of this node type.
	 */
	Q_PROPERTY(int minDatum READ minDatum);

	/*!
	 * \brief The largest valid datum for data nodes of this node type.
	 */
	Q_PROPERTY(int maxDatum READ maxDatum);

public:
	/*!
	 * \brief Creates a new single-line node type for storing integers
	 * \param name The name of the node type.
	 * \param description A one line description of the node type.
	 * \param tag The datafile tag of the node type.
	 * \param minDatum The smallest valid datum for data nodes of this node type.
	 * \param maxDatum The largest valid datum for data nodes of this node type.
	 * \param defaultDatum The default datum for new data nodes of this type.
	 * \param isWriteCompulsory Whether a property with this node type must be written to the datafile even if it only contains the default datum.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	IntegerNodeType(const QString& name, const QString& description, const QString& tag, int minDatum, int maxDatum, int defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c IntegerNodeType(name, description, tag, minDatum, maxDatum, defaultDatum, false, parent)
	 * \sa IntegerNodeType(const QString&, const QString&, const QString&, int, int, int, bool, QObject*)
	 */
	IntegerNodeType(const QString& name, const QString& description, const QString& tag, int minDatum, int maxDatum, int defaultDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c IntegerNodeType(name, description, tag, minDatum, maxDatum, (minDatum > 0 ? minDatum : maxDatum < 0 ? maxDatum : 0), false, parent)
	 * \sa IntegerNodeType(const QString&, const QString&, const QString&, int, int, int, bool, QObject*)
	 */
	IntegerNodeType(const QString& name, const QString& description, const QString& tag, int minDatum, int maxDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c IntegerNodeType(name, description, tag, -32767, 32767, defaultDatum, isWriteCompulsory, parent)
	 * \sa IntegerNodeType(const QString&, const QString&, const QString&, int, int, int, bool, QObject*)
	 */
	IntegerNodeType(const QString& name, const QString& description, const QString& tag, int defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c IntegerNodeType(name, description, tag, -32767, 32767, defaultDatum, false, parent)
	 * \sa IntegerNodeType(const QString&, const QString&, const QString&, int, int, int, bool, QObject*)
	 */
	IntegerNodeType(const QString& name, const QString& description, const QString& tag, int defaultDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c IntegerNodeType(name, description, tag, -32767, 32767, 0, false, parent)
	 * \sa IntegerNodeType(const QString&, const QString&, const QString&, int, int, int, bool, QObject*)
	 */
	IntegerNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent = 0);

	//! \copydoc SingleLineNodeType::createNewDataNode(const T&, QObject*) const
	virtual IntegerDataNode* createNewDataNode(const int& datum, QObject* parent = 0) const;

public:
	// Qt property access functions
	inline int minDatum() const { return m_min_datum; }
	inline int maxDatum() const { return m_max_datum; }

protected:
	virtual int parseNodeData(const QString& input) const;

private:
	IntegerNodeType(const IntegerNodeType&);
	IntegerNodeType& operator=(const IntegerNodeType&);

	const int m_min_datum;
	const int m_max_datum;
};

#endif
