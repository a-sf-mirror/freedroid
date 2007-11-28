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

#ifndef FDAE_GUI_H
#define FDAE_GUI_H

class CompoundDataNode;

#include <QtGui>
#include <QMainWindow>
#include <QCloseEvent>
#include <QTabWidget>

class GUI : public QMainWindow
{
	Q_OBJECT;

public:

	GUI();

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void documentWasModified();
	void disableCloseOfDocumentTab();

	void openFile();
	void saveFile();
	void saveFileAs();
	void closeFile();
	void closeTab();

private:
	void createActions();
	void createMenus();
	void createToolBars();
	bool contineWithDestructiveAction();

	CompoundDataNode *document;
	QString file;
	bool modified;

	QTabWidget *tabs;

	QMenu *fileMenu;
	QToolBar *fileToolBar;
	QAction *openAct;
	QAction *saveAct;
	QAction *saveAsAct;
	QAction *closeAct;
	QAction *exitAct;
};

#endif
