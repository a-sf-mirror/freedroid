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

#include "compoundwidget.h"
#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QPoint>
#include <QCoreApplication>

PropertyListWidget::PropertyListWidget(CompoundDataNode* node, QWidget* parent) :
	QWidget(parent),
	m_node(node),
	m_properties()
{
	Q_ASSERT(node != 0);
	QVBoxLayout* layout = new QVBoxLayout(this);
	
	const AbstractNodeType* property;
	foreach(property, node->typedType()->properties()) {
		if (node->containsProperty(property)) {
			QWidget* widget = node->propertyNode(property)->createPropertyWidget(node, this);
			m_properties.insert(property, widget);
			layout->addWidget(widget);
		}
	}

	layout->setMargin(4);
	setLayout(layout);

	connect(node, SIGNAL(propertyStructureChanged()),
			this, SLOT(propertyStructureChangedHandler()));
}

PropertyListWidget::~PropertyListWidget() {}

void PropertyListWidget::propertyStructureChangedHandler() {
	QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());
	Q_ASSERT(layout != 0);
	
	int index = 0;
	const AbstractNodeType* property;
	setUpdatesEnabled(false);
	foreach(property, node()->typedType()->properties()) {
		if (node()->containsProperty(property) && m_properties.value(property) == 0) {
			QWidget* widget = node()->propertyNode(property)->createPropertyWidget(node(), this);
			m_properties.insert(property, widget);
			layout->insertWidget(index, widget);
		}
		if (m_properties.value(property) != 0)
			++index;
	}
	setUpdatesEnabled(true);
}

EntryListWidget::EntryListWidget(CompoundDataNode* node, const AbstractNodeType* entry, QTabWidget* tabs, QWidget* parent) :
	QWidget(parent),
	m_node(node),
	m_entry(entry),
	m_tabs(tabs)
{
	Q_ASSERT(node != 0);
	Q_ASSERT(entry != 0);
	Q_ASSERT(tabs != 0);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(4);

	QWidget* addWidget = new QWidget(this);
	QHBoxLayout* addLayout = new QHBoxLayout(addWidget);

	QLabel* addLabel = new QLabel(addWidget);
	addLabel->setText(tr("Add a new entry:"));
	addLabel->setToolTip(tr("Add a new default entry to the end of the list"));
	addLabel->setMinimumWidth(200);
	addLayout->addWidget(addLabel);

	addLayout->addStretch(1);

	QToolButton* addButton = new QToolButton(addWidget);
	addButton->setIcon(QIcon(":/icons/edit-add.png"));
	addButton->setIconSize(QSize(32,32));
	addLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked(bool)),
			this, SLOT(addNewEntryNode()));

	addLayout->setMargin(2);
	addWidget->setLayout(addLayout);
	layout->addWidget(addWidget);

	AbstractDataNode* entryNode;
	foreach(entryNode, node->entryNodes(entry))
		if (entryNode != 0)
			layout->addWidget(entryNode->createEntryWidget(node, tabs, this));

	setLayout(layout);

	connect(node, SIGNAL(entryNodeAdded(AbstractDataNode*)),
			this, SLOT(addEntryNode(AbstractDataNode*)));
}

EntryListWidget::~EntryListWidget() {}

void EntryListWidget::addNewEntryNode() {
	AbstractDataNode* entryNode = entry()->createNewDataNode(node());
	node()->addEntryNode(entryNode);
}

void EntryListWidget::addEntryNode(AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	if (node->type() != entry()) return;
	QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());
	Q_ASSERT(layout != 0);

	QWidget* widget = node->createEntryWidget(this->node(), m_tabs, this);
	layout->addWidget(widget);

	QWidget* parent = widget;
	QScrollArea* scroll = 0;
	while (widget != 0 && scroll == 0) {
		parent = parent->parentWidget();
		scroll = dynamic_cast<QScrollArea*>(parent);
	}
	if (scroll != 0) {
		// We must update the layout cache to make ensureWidgetVisible work.
		widget->layout()->update();
		QCoreApplication::processEvents();
		QCoreApplication::sendPostedEvents();

		scroll->ensureWidgetVisible(widget, 12, 12);
	}

	EntryWidget* entryWidget = dynamic_cast<EntryWidget*>(widget);
	if (entryWidget != 0)
		entryWidget->openStandaloneWidget();
}


EntryWidget::EntryWidget(CompoundDataNode* node, CompoundDataNode* container, QTabWidget* tabs, QWidget* parent) :
	QWidget(parent),
	m_node(node),
	m_container(container),
	m_tabs(tabs),
	m_label(new QLabel(this)),
	m_standalone()
{
	Q_ASSERT(node != 0);
	Q_ASSERT(container != 0);
	Q_ASSERT(tabs != 0);
	Q_ASSERT(container->containsEntryNode(node));
	QHBoxLayout* layout = new QHBoxLayout(this);
		
	QLabel* icon = new QLabel(this);
	{
		// ToDo: Fetch the image from the game
		QPixmap pixmap = QPixmap(":/icons/dummyicon.png");
		QSize size = pixmap.size();
		size.scale(40,40, Qt::KeepAspectRatio);
		icon->setPixmap(pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	}
	layout->addWidget(icon);

	m_label->setText(node->name());
	m_label->setToolTip(node->description());
	m_label->setMinimumWidth(200);
	layout->addWidget(m_label);
	connect(node, SIGNAL(dataChanged()),
			this, SLOT(dataChangedHandler()));

	layout->addStretch(1);

	QToolButton* edit = new QToolButton(this);
	edit->setIcon(QIcon(":/icons/edit.png"));
	edit->setIconSize(QSize(32,32));
	layout->addWidget(edit);
	connect(edit, SIGNAL(clicked(bool)),
			this, SLOT(openStandaloneWidget()));

	QToolButton* clone = new QToolButton(this);
	clone->setIcon(QIcon(":/icons/edit-copy.png"));
	clone->setIconSize(QSize(32,32));
	layout->addWidget(clone);
	connect(clone, SIGNAL(clicked(bool)),
			this, SLOT(cloneEntryNode()));

	QToolButton* trash = new QToolButton(this);
	trash->setIcon(QIcon(":/icons/edit-trash.png"));
	trash->setIconSize(QSize(32,32));
	layout->addWidget(trash);
	connect(trash, SIGNAL(clicked(bool)),
			node, SLOT(deleteLater()));

	layout->setMargin(2);
	setLayout(layout);
	setFocusProxy(edit);
	connect(node, SIGNAL(destroyed(QObject*)),
			this, SLOT(deleteLater()));
}

EntryWidget::~EntryWidget() {}


void EntryWidget::dataChangedHandler() {
	m_label->setText(node()->name());
	m_label->setToolTip(node()->description());
	if (m_standalone != 0)
		m_tabs->setTabText(m_tabs->indexOf(m_standalone), node()->name());
		// ToDo: Update entry icon.
}

void EntryWidget::cloneEntryNode() {
	container()->addEntryNode(node()->clone(container()));
}

void EntryWidget::openStandaloneWidget() {
	if (m_standalone == 0) {
		m_standalone = node()->createStandaloneWidget(m_tabs);
		m_tabs->addTab(m_standalone, node()->name());
		// ToDo: Include entry icon.
	}
	m_tabs->setCurrentWidget(m_standalone);
}
