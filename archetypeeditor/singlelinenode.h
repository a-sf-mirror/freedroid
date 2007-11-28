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
 * \file singlelinenode.h
 * \brief The building blocks to store single-line data in datafiles
 * \author Jon Severinsson (jon@severinsson.net)
 */

#ifndef FDAE_SINGLE_LINE_NODE_H
#define FDAE_SINGLE_LINE_NODE_H

#include "abstractnode.h"

#include <QString>
#include <QTextStream>

template <typename T> class SingleLineNodeType;

#define FDAE_SINGLE_LINE_DATA_NODE \
	/*!\
	 * \brief The actual datum stored in the data node\
	 */\
	Q_PROPERTY(T datum READ datum WRITE setDatum);

#define FDAE_SINGLE_LINE_NODE_TYPE \
	/*!\
	 * \brief The data tag of this node type.\
	 * This property contains the data tag of this single-line node type, as present in the datafile.\
	 * The actual datum begins directly at the end of the tag, so for readability it should end with\
	 * either a \b = or a \b : character.\
	 */\
	Q_PROPERTY(QString tag READ tag);\
	/*!\
	 * \brief The default datum for data nodes of this node type.\
	 * This is the datum that will be used when initializing new nodes of this type,\
	 * as well as when a property with this node type is missing from the datafile.\
	 */\
	Q_PROPERTY(QString defaultDatum READ defaultDatum);\
	/*!\
	 * \brief Whether a property with this node type must be written to the datafile even if it only contains the default datum.\
	 *\
	 * This indicates whether a property with this node type always must be written to the datafile.\
	 * It is used as a work-around for the poor parser in the game, and should only be set when\
	 * nessesary.\
	 *\
	 * \b Note: The value of \c isWriteCompulsory will in no way change the result after saving the\
	 * datafile and reading it back to the editor, but it might affect the game. Noteably the game\
	 * will crash if some particular node types is missing from the datafile.\
	 *\
	 * It might, ofcourse, also change result if the datafile is opened in a later version of the \
	 * editor that specifies a different default datum.\
	 */\
	Q_PROPERTY(QString isWriteCompulsory READ isWriteCompulsory);\

/*!
 * \class SingleLineDataNode
 * \brief A single datum from a datafile.
 *
 * This virtual class is used as a base class for all single line data nodes. It's main purpose is
 * as an implementation detail, and should only be used directly by its subclasses.
 * 
 * \sa SingleLineNodeType
 * \sa AbstractDataNode
 * \sa IntegerDataNode
 * \sa FloatDataNode
 * \sa BooleanDataNode
 * \sa StringDataNode
 * \sa EnumerationDataNode
 */
template <typename T> class SingleLineDataNode : public AbstractDataNode {

public:
	/*!
	 * \brief Creates a new single-line data node
	 * \param type The node type of the node type.
	 * \param datum The actual datum stored in this data node.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	SingleLineDataNode<T>(const SingleLineNodeType<T>* type, const T& datum, QObject* parent = 0);

	//! \copydoc AbstractDataNode::writeNode(QTextStream&) const
	void writeNode(QTextStream& out) const;

protected:
	/*!
	 * \brief A utility function that writes the datum to a datafile
	 * \param out An output stream to a datafile
	 *
	 * This virtual function must be provided by all subclasses. It must write the datum, and the
	 * datum only, in such a way that both #SingleLineNodeType::parseNodeData and the game parser
	 * can parse it.
	 */
	virtual void writeNodeData(QTextStream& out) const = 0;

public:
	// Qt property access functions
	inline const T& datum() const { return m_datum; }
	virtual void setDatum(const T& datum);

private:
	SingleLineDataNode<T>(const SingleLineDataNode<T>&);
	SingleLineDataNode<T>& operator=(const SingleLineDataNode<T>&);

	T m_datum;
};



/*!
 * \class SingleLineNodeType
 * \brief A single-line node type
 *
 * This virtual class is used as a base class for all single line nodes types. It's main purpose is
 * as an implementation detail, and should only be used directly by its subclasses.
 *
 * \sa AbstractNodeType
 * \sa IntegerNodeType
 * \sa FloatNodeType
 * \sa BooleanNodeType
 * \sa StringNodeType
 * \sa EnumerationNodeType
 */
template <typename T> class SingleLineNodeType : public AbstractNodeType {

public:
	/*!
	 * \brief Creates a new single-line node type
	 * \param name The name of the node type.
	 * \param description A one line description of the node type.
	 * \param tag The datafile tag of the node type.
	 * \param defaultDatum The default datum for new data nodes of this type.
	 * \param isWriteCompulsory Whether a property with this node type must be written to the datafile even if it only contains the default datum.
	 * \param parent The Qt parent object, for automatic memory de-allocation.
	 */
	SingleLineNodeType<T>(const QString& name, const QString& description, const QString& tag, const T& defaultDatum, bool isWriteCompulsory, QObject* parent = 0);

	//! \copydoc AbstractNodeType::canReadData(const QString&) const
	virtual bool canReadData(const QString& next) const { return next.startsWith(tag()); }

	//! \copydoc AbstractNodeType::readData(QString&, QTextStream&, QObject*) const
	virtual AbstractDataNode* readData(QString& next, QTextStream& remainder, QObject* parent = 0) const;

	//! \copydoc AbstractNodeType::createNewDataNode(QObject*) const
	virtual AbstractDataNode* createNewDataNode(QObject* parent = 0) const { return createNewDataNode(defaultDatum(), parent); }

	/*!
	 * \brief Creates a new data node of this type
	 * \param parent The Qt parent object of the new data node.
	 * \param datum The datum stored in this node.
	 *
	 * This functions creates a new data node with this node type and the specified datum.
	 *
	 * \b Note: The returned ponter will point to an object on the heap, and must be deleted using
	 * #QObject::deleteLater(). If you specified a parent object Qt will do this automatically
	 * when the parent object is deleted.
	 */
	virtual SingleLineDataNode<T>* createNewDataNode(const T& datum, QObject* parent = 0) const = 0;

protected:
	/*!
	 * \brief A utility function that parses the datum from the datafile
	 * \param input The input datum to be parsed
	 *
	 * This virtual function must be provided by all subclasses. It must be able to parse all valid
	 * data as it appears after the tag in a datafile, to the correct C++ type.
	 */
	virtual T parseNodeData(const QString& input) const = 0;

public:
	// Qt property access functions
	inline const QString& tag() const { return m_tag; }
	inline const T& defaultDatum() const { return m_default_datum; }
	inline bool isWriteCompulsory() const { return m_is_write_compulsory; };

private:
	SingleLineNodeType<T>(const SingleLineNodeType<T>&);
	SingleLineNodeType<T>& operator=(const SingleLineNodeType<T>&);

	const QString m_tag;
	const T m_default_datum;
	const bool m_is_write_compulsory;
};


/***************************************************************************
 * Below follows some template stuff already described above.              *
 ***************************************************************************/

template <typename T> SingleLineNodeType<T>::SingleLineNodeType(const QString& name, const QString& description, const QString& tag, const T& defaultDatum, bool isWriteCompulsory, QObject* parent) :
	AbstractNodeType(name, description, parent),
	m_tag(tag),
	m_default_datum(defaultDatum),
	m_is_write_compulsory(isWriteCompulsory)
{
	Q_ASSERT(!tag.isEmpty());
}

template <typename T> AbstractDataNode* SingleLineNodeType<T>::readData(QString& line, QTextStream& out, QObject* parent) const {
	Q_UNUSED(out);
	Q_ASSERT(canReadData(line));
	return createNewDataNode(parseNodeData(line.mid(tag().length())), parent);
}


template <typename T> SingleLineDataNode<T>::SingleLineDataNode(const SingleLineNodeType<T>* type, const T& datum, QObject* parent) :
	AbstractDataNode(type, parent),
	m_datum()
{
	setDatum(datum);
}

template <typename T> void SingleLineDataNode<T>::writeNode(QTextStream& out) const {
	const SingleLineNodeType<T>* type = dynamic_cast<const SingleLineNodeType<T>*>(AbstractDataNode::type());
	Q_ASSERT(type != 0);
	if (type->isWriteCompulsory() || type->defaultDatum() != datum()) {
		out << type->tag();
		writeNodeData(out);
		out << "\n";
	}
}

template <typename T> void SingleLineDataNode<T>::setDatum(const T& datum) {
	if (m_datum == datum)
		return;
	m_datum = datum;
	emit dataChanged();
}

#endif
