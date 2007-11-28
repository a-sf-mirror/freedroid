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

#ifndef FDAE_DATAFILE_SYNTAX_H
#define FDAE_DATAFILE_SYNTAX_H

#include "compoundnode.h"
#include <QList>

class DatafileSyntaxes : QObject {
	Q_OBJECT;
	
public:
	static CompoundDataNode* readData(const QString& filename, QObject* parent = 0);
	static bool writeNode(const CompoundDataNode* node, const QString& filename);

private:
	static DatafileSyntaxes* instance();
	DatafileSyntaxes();
	const CompoundNodeType* detectFileType(QTextStream& stream, QString& line);

	DatafileSyntaxes(const DatafileSyntaxes&);
	DatafileSyntaxes& operator= (const DatafileSyntaxes&);

	static DatafileSyntaxes* m_instance;
	QList<const CompoundNodeType*> m_types;
};

#endif
