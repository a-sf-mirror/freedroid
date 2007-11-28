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

#ifndef FDAE_COMPOUND_WIDGET_H
#define FDAE_COMPOUND_WIDGET_H

#include "compoundnode.h"
#include <QPointer>
#include <QWidget>
#include <QLabel>

class PropertyListWidget : public QWidget {
	Q_OBJECT;
	Q_PROPERTY(CompoundDataNode* node READ node);

public:
	PropertyListWidget(CompoundDataNode* node, QWidget* parent = 0);

public:
	virtual ~PropertyListWidget();

	// Qt property access functions
	inline CompoundDataNode* node() { return m_node; }

private slots:
	void propertyStructureChangedHandler();

private:
	CompoundDataNode* m_node;
	QHash<const AbstractNodeType*, QPointer<QWidget> > m_properties;
};


class EntryListWidget : public QWidget {
	Q_OBJECT;
	Q_PROPERTY(CompoundDataNode* node READ node);
	Q_PROPERTY(const AbstractNodeType* entry READ entry);

public:
	EntryListWidget(CompoundDataNode* node, const AbstractNodeType* entry, QTabWidget* tabs, QWidget* parent = 0);

public:
	virtual ~EntryListWidget();

	// Qt property access functions
	inline CompoundDataNode* node() { return m_node; }
	inline const AbstractNodeType* entry() { return m_entry; }

private slots:
	void addNewEntryNode();
	void addEntryNode(AbstractDataNode* node);

private:
	CompoundDataNode* m_node;
	const AbstractNodeType* m_entry;
	QTabWidget* m_tabs;
};


class EntryWidget : public QWidget {
	Q_OBJECT;
	Q_PROPERTY(CompoundDataNode* node READ node);
	Q_PROPERTY(CompoundDataNode* container READ container);

public:
	EntryWidget(CompoundDataNode* node, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent = 0);

public slots:
	void cloneEntryNode();
	void openStandaloneWidget();

public:
	virtual ~EntryWidget();

	// Qt property access functions
	inline CompoundDataNode* node() { return m_node; }
	inline CompoundDataNode* container() { return m_container; }

private slots:
	void dataChangedHandler();

private:
	CompoundDataNode* m_node;
	CompoundDataNode* m_container;
	QTabWidget* m_tabs;
	
	QLabel* m_label;
	QPointer<QWidget> m_standalone;
};

#endif
