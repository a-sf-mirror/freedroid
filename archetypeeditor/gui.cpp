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

#include "gui.h"
#include "datafilesyntaxes.h"
#include <QCoreApplication>

GUI::GUI()
{
	document = 0;
	modified = false;

	tabs = new QTabWidget(this);
	QToolButton *corner = new QToolButton(tabs);
	corner->setIcon(QIcon(":/icons/tab-remove.png"));
	corner->setIconSize(QSize(16,16));
	corner->setEnabled(false);
	tabs->setCornerWidget(corner);
	connect(corner, SIGNAL(clicked(bool)),
			this, SLOT(closeTab()));
	connect(tabs, SIGNAL(currentChanged(int)),
			this, SLOT(disableCloseOfDocumentTab()));
	
	setCentralWidget(tabs);
	setWindowIcon(QIcon(":/icons/appicon.png"));
	setWindowTitle(tr("FreeDroid Archetype Editor"));
	setIconSize(QSize(32, 32));
	setMinimumSize(500, 400);

	createActions();
	createToolBars();
}

bool GUI::contineWithDestructiveAction() {
	if (modified) {
		int ret = QMessageBox::warning(this,
		                              tr("Close Document - FreeDroid Archetype Editor"),
		                              tr("The document has been modified.\n"
		                              "Do you want to save your changes?"),
		                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Cancel)
			return false; 
		if (ret == QMessageBox::Save)
			saveFile();
	}
	return true;
}

void GUI::closeEvent(QCloseEvent *event)
{
	if (contineWithDestructiveAction())
		event->accept();
	else
		event->ignore();
}

void GUI::documentWasModified() {
	modified = true;
	saveAct->setEnabled(true);
}

void GUI::openFile()
{
	QString fileName = QFileDialog::getOpenFileName(this);
	if (!fileName.isEmpty()) {
		document = DatafileSyntaxes::readData(fileName, this);
		
		if (document == 0) {
			QMessageBox::critical(this,
			                     tr("Error - FreeDroid Archetype Editor"),
			                     tr("Could not open the specified file."),
			                     QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape);
			return;
		}
		
		file = fileName;
		modified = false;

		tabs->addTab(document->createStandaloneWidget(tabs), document->name());
		openAct->setEnabled(false);
		saveAct->setEnabled(false);
		saveAsAct->setEnabled(true);
		closeAct->setEnabled(true);

		while (QCoreApplication::hasPendingEvents()) {
			QCoreApplication::processEvents(QEventLoop::DeferredDeletion);
			QCoreApplication::sendPostedEvents();
		}

		connect(document, SIGNAL(dataChanged()),
				this, SLOT(documentWasModified()));

	}
}

void GUI::saveFile()
{
	DatafileSyntaxes::writeNode(document, file);
	modified = false;
	saveAct->setEnabled(false);
}

void GUI::saveFileAs()
{
	QString fileName = QFileDialog::getSaveFileName(this);
	if (!fileName.isEmpty()) {
		DatafileSyntaxes::writeNode(document, fileName);
		file = fileName;
		modified = false;
		saveAct->setEnabled(false);
	}
}

void GUI::closeFile()
{
	if (contineWithDestructiveAction()) {
		disconnect(document, 0, this, 0);
		document->deleteLater();
		document = 0;
		modified = false;
		openAct->setEnabled(true);
		saveAct->setEnabled(false);
		saveAsAct->setEnabled(false);
		closeAct->setEnabled(false);
	}
}

void GUI::closeTab()
{
	tabs->currentWidget()->deleteLater();
}

void GUI::disableCloseOfDocumentTab()
{
	tabs->cornerWidget()->setEnabled(tabs->currentIndex() != 0);
}

void GUI::createActions()
{
	openAct = new QAction(QIcon(":/icons/document-open.png"), tr("&Open..."), this);
	openAct->setShortcut(tr("Ctrl+O"));
	openAct->setToolTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

	saveAct = new QAction(QIcon(":/icons/document-save.png"), tr("&Save"), this);
	saveAct->setShortcut(tr("Ctrl+S"));
	saveAct->setToolTip(tr("Save the document to disk"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

	saveAsAct = new QAction(QIcon(":/icons/document-save-as.png"), tr("Save &As..."), this);
	saveAsAct->setToolTip(tr("Save the document under a new name"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveFileAs()));

	closeAct = new QAction(QIcon(":/icons/document-close.png"), tr("&Close File"), this);
	closeAct->setToolTip(tr("Close the document"));
	connect(closeAct, SIGNAL(triggered()), this, SLOT(closeFile()));

	exitAct = new QAction(QIcon(":/icons/application-exit.png"), tr("E&xit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	exitAct->setToolTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	saveAct->setEnabled(false);
	saveAsAct->setEnabled(false);
	closeAct->setEnabled(false);
}

void GUI::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);
	fileToolBar->addAction(saveAsAct);
	fileToolBar->addAction(closeAct);
	fileToolBar->addSeparator();
	fileToolBar->addAction(exitAct);
	QMainWindow::addToolBar(Qt::LeftToolBarArea, fileToolBar);
}
