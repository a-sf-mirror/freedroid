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
 * \file floatnode.h
 * \brief The building blocks to store floating point data in datafiles
 * \author Jon Severinsson (jon@severinsson.net)
 */

#ifndef FDAE_FLOAT_NODE_H
#define FDAE_FLOAT_NODE_H

#include "singlelinenode.h"

class FloatNodeType;


/*!
 * \class FloatDataNode
 * \brief A single floating point numeral datum from a datafile.
 *
 * \sa FloatNodeType
 * \sa SingleLineDataNode
 */
class FloatDataNode : public SingleLineDataNode<double> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_DATA_NODE;

public:
	//! \copydoc SingleLineDataNode::SingleLineDataNode(const SingleLineNodeType, const T&, QObject*)
	FloatDataNode(const FloatNodeType* type, double datum, QObject* parent = 0);

	//! \copydoc IntegerDataNode::typedType()
	const FloatNodeType* typedType() const;

signals:
	//! \copydoc IntegerDataNode::typedType(int)
	void datumChanged(double datum);

public:
	virtual FloatDataNode* clone(QObject* parent = 0);

public slots:
	// Qt property access functions
	virtual void setDatum(double datum);

protected:
	virtual void writeNodeData(QTextStream& out) const;

	virtual QWidget* createWidget(WidgetMode mode, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent);

private:
	FloatDataNode(const FloatDataNode&);
	FloatDataNode& operator=(const FloatDataNode&);
};



/*!
 * \class FloatNodeType
 * \brief A single-line node type for storing floating point numbers
 *
 * \sa FloatDataNode
 * \sa SingleLineNodeType
 */
class FloatNodeType : public SingleLineNodeType<double> {
	Q_OBJECT;
	FDAE_SINGLE_LINE_NODE_TYPE;

	/*!
	 * \brief The smallest valid datum for data nodes of this node type.
	 */
	Q_PROPERTY(double minDatum READ minDatum);

	/*!
	 * \brief The largest valid datum for data nodes of this node type.
	 */
	Q_PROPERTY(double maxDatum READ maxDatum);

public:
	/*!
	 * \brief Creates a new single-line node type for storing floating point numbers
	 * \param name The name of the node type.
	 * \param description A one line description of the node type.
	 * \param tag The datafile tag of the node type.
	 * \param minDatum The smallest valid datum for data nodes of this node type.
	 * \param maxDatum The largest valid datum for data nodes of this node type.
	 * \param defaultDatum The default datum for new data nodes of this type.
	 * \param isWriteCompulsory Whether a property with this node type must be written to the datafile even if it only contains the default datum.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	FloatNodeType(const QString& name, const QString& description, const QString& tag, double minDatum, double maxDatum, double defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c FloatNodeType(name, description, tag, minDatum, maxDatum, defaultDatum, false, parent)
	 * \sa FloatNodeType(const QString&, const QString&, const QString&, double, double, double, bool, QObject*)
	 */
	FloatNodeType(const QString& name, const QString& description, const QString& tag, double minDatum, double maxDatum, double defaultDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c FloatNodeType(name, description, tag, minDatum, maxDatum, (minDatum > 0.0 ? minDatum : maxDatum < 0.0 ? maxDatum : 0.0), false, parent)
	 * \sa FloatNodeType(const QString&, const QString&, const QString&, double, double, double, bool, QObject*)
	 */
	FloatNodeType(const QString& name, const QString& description, const QString& tag, double minDatum, double maxDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c FloatNodeType(name, description, tag, -999999999.9999, 999999999.9999, defaultDatum, isWriteCompulsory, parent)
	 * \sa FloatNodeType(const QString&, const QString&, const QString&, double, double, double, bool, QObject*)
	 */
	FloatNodeType(const QString& name, const QString& description, const QString& tag, double defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c FloatNodeType(name, description, tag, -999999999.9999, 999999999.9999, defaultDatum, false, parent)
	 * \sa FloatNodeType(const QString&, const QString&, const QString&, double, double, double, bool, QObject*)
	 */
	FloatNodeType(const QString& name, const QString& description, const QString& tag, double defaultDatum, QObject* parent = 0);

	/*!
	 * This convienience method is equivalent to \c FloatNodeType(name, description, tag, -999999999.9999, 999999999.9999, 0.0, false, parent)
	 * \sa FloatNodeType(const QString&, const QString&, const QString&, double, double, double, bool, QObject*)
	 */
	FloatNodeType(const QString& name, const QString& description, const QString& tag, QObject* parent = 0);

	//! \copydoc SingleLineNodeType::createNewDataNode(const T&, QObject*) const
	virtual FloatDataNode* createNewDataNode(const double& datum, QObject* parent = 0) const;

public:
	// Qt property access functions
	inline double minDatum() const { return m_min_datum; }
	inline double maxDatum() const { return m_max_datum; }

protected:
	virtual double parseNodeData(const QString& input) const;

private:
	FloatNodeType(const FloatNodeType&);
	FloatNodeType& operator=(const FloatNodeType&);
	
	const double m_min_datum;
	const double m_max_datum;
};

#endif
