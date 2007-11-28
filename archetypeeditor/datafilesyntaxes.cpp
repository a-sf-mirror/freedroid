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

#include <QObject>

#include "datafilesyntaxes.h"
#include "integernode.h"
#include "floatnode.h"
#include "booleannode.h"
#include "stringnode.h"
#include "singlechoicenode.h"
#include "compoundnode.h"

#include <QFile>

namespace EquipmentSlot {
	SingleChoiceList SLOTS;
	SingleChoice* NONE = SLOTS.addChoice(QObject::tr("None"), "none");
	SingleChoice* WEAPON = SLOTS.addChoice(QObject::tr("Weapon"), "weapon");
	SingleChoice* ARMOUR = SLOTS.addChoice(QObject::tr("Armour"), "armour");
	SingleChoice* SHIELD = SLOTS.addChoice(QObject::tr("Shield"), "shield");
	SingleChoice* HEADWEAR = SLOTS.addChoice(QObject::tr("Headwear"), "special");
	SingleChoice* FOOTWEAR = SLOTS.addChoice(QObject::tr("Footwear"), "drive");
}

namespace AmunitionType {
	SingleChoiceList TYPES;
	SingleChoice* NONE = TYPES.addChoice(QObject::tr("None"), "none");
	SingleChoice* PLASMA = TYPES.addChoice(QObject::tr("Plasma"), "plasma_ammunition");
	SingleChoice* LASER = TYPES.addChoice(QObject::tr("Laser"), "laser_ammunition");
	SingleChoice* EXTERMINATOR = TYPES.addChoice(QObject::tr("Exterminator"), "exterminator_ammunition");
}

bool isChecked(const AbstractDataNode* node) {
	if (node == 0) return false;
	const BooleanDataNode* bnode = dynamic_cast<const BooleanDataNode*>(node);
	Q_ASSERT(bnode != 0);
	return bnode->datum();
}

bool isUnchecked(const AbstractDataNode* node) {
	if (node == 0) return false;
	const BooleanDataNode* bnode = dynamic_cast<const BooleanDataNode*>(node);
	Q_ASSERT(bnode != 0);
	return !bnode->datum();
}

bool isNotChecked(const AbstractDataNode* node) {
	if (node == 0) return true;
	const BooleanDataNode* bnode = dynamic_cast<const BooleanDataNode*>(node);
	Q_ASSERT(bnode != 0);
	return !bnode->datum();
}

bool isNotUnchecked(const AbstractDataNode* node) {
	if (node == 0) return true;
	const BooleanDataNode* bnode = dynamic_cast<const BooleanDataNode*>(node);
	Q_ASSERT(bnode != 0);
	return bnode->datum();
}

bool isEquipment(const AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	const SingleChoiceDataNode* scnode = dynamic_cast<const SingleChoiceDataNode*>(node);
	Q_ASSERT(scnode != 0);
	return scnode->datum() != EquipmentSlot::NONE;
}

bool isNotEquipment(const AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	const SingleChoiceDataNode* scnode = dynamic_cast<const SingleChoiceDataNode*>(node);
	Q_ASSERT(scnode != 0);
	return scnode->datum() == EquipmentSlot::NONE;
}

bool isWeapon(const AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	const SingleChoiceDataNode* scnode = dynamic_cast<const SingleChoiceDataNode*>(node);
	Q_ASSERT(scnode != 0);
	return scnode->datum() == EquipmentSlot::WEAPON;
}

bool isNonWeaponEquipment(const AbstractDataNode* node) {
	Q_ASSERT(node != 0);
	const SingleChoiceDataNode* scnode = dynamic_cast<const SingleChoiceDataNode*>(node);
	Q_ASSERT(scnode != 0);
	return scnode->datum() != EquipmentSlot::NONE && scnode->datum() != EquipmentSlot::WEAPON;
}

CompoundNodeType* getItemlistType(QObject* parent) {
	CompoundNodeType* itemlist = new CompoundNodeType(QObject::tr("Itemlist"), QObject::tr(""), "*** Start of item data section: ***", "*** End of item data section: ***", parent);
	itemlist->addProperty(new FloatNodeType(QObject::tr("Common Bullet Speed Factor"), QObject::tr(""), "Common factor for all ranged weapons bullet speed values: ", 1.0, true, itemlist));
	itemlist->addProperty(new FloatNodeType(QObject::tr("Common Bullet Damage Factor"), QObject::tr(""), "Common factor for all ranged weapons bullet damage values: ", 1.0, true, itemlist));
	itemlist->addProperty(new FloatNodeType(QObject::tr("Common Melee Damage Factor"), QObject::tr(""), "Common factor for all melee weapons damage values: ", 1.0, true, itemlist));

	CompoundNodeType* item = new CompoundNodeType(QObject::tr("Item"), QObject::tr(""), "** Start of new item specification subsection **", "** End of new item specification subsection **", itemlist);
	itemlist->addEntry(item);

	StringNodeType* itemName = new StringNodeType(QObject::tr("Item Name"), QObject::tr("This is the name of the item, as seen in the game"), "Item name=", "", true, item);
	StringNodeType* itemDescription = new StringNodeType(QObject::tr("Item Description"), QObject::tr(""), "Item description text=", "", true, item);
	item->addProperty(itemName);
	item->addProperty(itemDescription);
	item->setNameProperty(itemName);
	item->setDescriptionProperty(itemDescription);
	item->setDummyName("THIS ITEM DOES NOT EXIST");
	item->setDummyDescription("Dummy item. If you see this, report a bug.");

	SingleChoiceNodeType* equipment = new SingleChoiceNodeType(QObject::tr("Equipment Slot"), QObject::tr(""), "Item can be installed in slot with name=", &EquipmentSlot::SLOTS, EquipmentSlot::NONE, true, item);
	BooleanNodeType* singleuse = new BooleanNodeType(QObject::tr("Single Use Item"), QObject::tr("This item can be used once, then it is consumed. Examples includes power capsules, scripts and grenades"), "Item can be applied in combat=", false, true, item);
	BooleanNodeType* indestructible = new BooleanNodeType(QObject::tr("Indestructible"), QObject::tr("This item can not be destroyed"), "Item can be installed in influ=", false, true, item);
	item->addProperty(equipment);
	item->addConditionalProperty(singleuse,
	                             equipment,
	                             &isNotEquipment);
	item->addConditionalProperty(indestructible,
	                             equipment,
	                             &isEquipment);

	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Base Durability"), QObject::tr(""), "Base item duration=", 0, 32767, 0, false, item),
	                             indestructible,
	                             &isUnchecked);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Maximum Durability Modifier"), QObject::tr("Each item will get a random number of bonus durability points up to this number in addition to the base durability specified above"), "plus duration modifier=", 0, 32767, 0, false, item),
	                             indestructible,
	                             &isUnchecked);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Width in Inventory"), QObject::tr(""), "inventory_size_x=", 1, 10, 1, true, item),
	                             singleuse,
	                             &isNotChecked);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Height in Inventory"), QObject::tr(""), "inventory_size_y=", 1 , 6, 1, true, item),
	                             singleuse,
	                             &isNotChecked);
	item->addConditionalProperty(new BooleanNodeType(QObject::tr("Stackable in Inventory"), QObject::tr("You can have multiple copies of this item in the same spot in the inventory."), "Items of this type collect together in inventory=", false, true, item),
	                             equipment,
	                             &isNotEquipment);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Required Strength"), QObject::tr(""), "Strength minimum required to wear/wield this item=", -1, 32767, -1, false, item),
	                             equipment,
	                             &isEquipment);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Required Dexterity"), QObject::tr(""), "Dexterity minimum required to wear/wield this item=", -1, 32767, -1, false, item),
	                             equipment,
	                             &isEquipment);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Required CPU"), QObject::tr(""), "Magic minimum required to wear/wield this item=", -1, 32767, -1, false, item),
	                             equipment,
	                             &isEquipment);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Base AC Bonus"), QObject::tr(""), "Item as defensive item: base_ac_bonus=", 0, false, item),
	                             equipment,
	                             &isNonWeaponEquipment);
	item->addConditionalProperty(new IntegerNodeType(QObject::tr("Maximum AC Bonus Modifier"), QObject::tr(""), "Item as defensive item: ac_bonus_modifier=", 0, false, item),
	                             equipment,
	                             &isNonWeaponEquipment);
	item->addProperty(new StringNodeType(QObject::tr("Shop Animation Path Prefix"), QObject::tr(""), "Item uses rotation series with prefix=", "NONE_AVAILABLE_YET", true, item));
	item->addProperty(new StringNodeType(QObject::tr("Inventory Image File"), QObject::tr(""), "File or directory name for inventory image=", "inventory_image_7.png", true, item));
	item->addProperty(new StringNodeType(QObject::tr("Drop Sound File"), QObject::tr(""), "Item uses drop sound with filename=", "", true, item));
	item->addProperty(new IntegerNodeType(QObject::tr("Base Price"), QObject::tr(""), "Base list price=", 0, 32767, 0, true, item));

	CompoundNodeType* weapon = new CompoundNodeType(QObject::tr("Weapon Specifications"), QObject::tr(""), "----- the following part is only relevant for weapons -----", "----- end of part only relevant for weapons -----", item);
	item->addConditionalProperty(weapon, equipment, &isWeapon);

	weapon->addProperty(new FloatNodeType(QObject::tr("Recharge Time"), QObject::tr(""), "Item as gun: recharging time=", 0.0, true, weapon));
	weapon->addProperty(new FloatNodeType(QObject::tr("Reload Time"), QObject::tr(""), "Item as gun: reloading time=", 0.0, true, weapon));
	weapon->addProperty(new FloatNodeType(QObject::tr("Bullet Speed"), QObject::tr(""), "Item as gun: speed of bullets=", 0.0, true, weapon));
	weapon->addProperty(new FloatNodeType(QObject::tr("Melee weapon"), QObject::tr("0.0 means it's a gun, anything else and it's a melee weapon"), "Item as gun: angle change of bullets=", 0.0, true, weapon));
	weapon->addProperty(new IntegerNodeType(QObject::tr("Damage Base"), QObject::tr(""), "Item as gun: damage of bullets=", 0, true, weapon));
	weapon->addProperty(new IntegerNodeType(QObject::tr("DamageModifier"), QObject::tr(""), "Item as gun: modifier for damage of bullets=", 0, true, weapon));
	weapon->addProperty(new IntegerNodeType(QObject::tr("One Shot Only"), QObject::tr(""), "Item as gun: one_shot_only=", false, true, weapon));
	weapon->addProperty(new IntegerNodeType(QObject::tr("Bullet Image Type"), QObject::tr(""), "Item as gun: bullet_image_type=", 0, true, weapon));
	weapon->addProperty(new FloatNodeType(QObject::tr("Bullet Lifetime"), QObject::tr(""), "Item as gun: bullet_lifetime=", 0.0, true, weapon));
	weapon->addProperty(new FloatNodeType(QObject::tr("Melee Weapon Offset"), QObject::tr(""), "Item as gun: offset for melee weapon=", 0.0, true, weapon));
	weapon->addProperty(new FloatNodeType(QObject::tr("Starting Angle Modifier"), QObject::tr(""), "Item as gun: modifier for starting angle=", 0.0, true, weapon));
	weapon->addProperty(new BooleanNodeType(QObject::tr("Ignore Collisions With Walls"), QObject::tr(""), "Item as gun: ignore collisions with wall=", false, true, weapon));
	weapon->addProperty(new BooleanNodeType(QObject::tr("Reflect Other Bullets"), QObject::tr(""), "Item as gun: reflect other bullets=", false, true, weapon));
	weapon->addProperty(new BooleanNodeType(QObject::tr("Pass Through Explosives"), QObject::tr(""), "Item as gun: pass through explosions=", false, true, weapon));
	weapon->addProperty(new BooleanNodeType(QObject::tr("Pass Through Hit Bodies"), QObject::tr(""), "Item as gun: pass through hit bodies=", false, true, weapon));
	weapon->addProperty(new SingleChoiceNodeType(QObject::tr("Required Ammunition Type"), QObject::tr(""), "Item as gun: required ammunition type=", &AmunitionType::TYPES, AmunitionType::NONE, true, weapon));
	weapon->addProperty(new IntegerNodeType(QObject::tr("Ammo Clip Size"), QObject::tr(""), "Item as gun: ammo clip size=", 0, true, weapon));
	weapon->addProperty(new BooleanNodeType(QObject::tr("Two Handed Weapon"), QObject::tr(""), "Item as gun: weapon requires both hands=", false, true, weapon));

	return itemlist;
}

CompoundNodeType* getPrefixlistType(QObject* parent) {
	CompoundNodeType* prefixlist = new CompoundNodeType(QObject::tr("Prefixlist"), QObject::tr(""), "*** Start of presuff data section: ***", "*** End of presuff data section ***", parent);

	CompoundNodeType* prefix = new CompoundNodeType(QObject::tr("Prefix"), QObject::tr("The template for an item prefix that can be applied to any item"), "** Start of new prefix specification subsection **", "** End of new prefix specification subsection **", prefixlist);
	CompoundNodeType* suffix = new CompoundNodeType(QObject::tr("Suffix"), QObject::tr("The template for an item suffix that can be applied to any item"), "** Start of new suffix specification subsection **", "** End of new suffix specification subsection **", prefixlist);
	prefixlist->addEntry(prefix);
	prefixlist->addEntry(suffix);

	StringNodeType* prefixName = new StringNodeType(QObject::tr("Prefix name"), QObject::tr("The name of the prefix. Will be prepended to the name of an item."), "Prefix name=", "", true, prefix);
	StringNodeType* suffixName = new StringNodeType(QObject::tr("Suffix name"), QObject::tr("The name of the suffix. Will be appended to the name of an item."), "Prefix name=", "", true, suffix);
	prefix->addProperty(prefixName);
	suffix->addProperty(suffixName);
	prefix->setNameProperty(prefixName);
	suffix->setNameProperty(suffixName);
	prefix->setDummyName("I am a big bad bug in the game. Report me. Please. ");
	suffix->setDummyName("I am a big bad bug in the game. Report me. Please. ");

	AbstractNodeType* property = 0;
	property = new IntegerNodeType(QObject::tr("Bonus to Dexterity"), QObject::tr("The Strength bonus this item gives"), "Bonus to dexterity=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to Dexterity modifier"), QObject::tr("A random modifier between 0 and this value will be added to the Strength bonus"), "Bonus to dexterity modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to Life"), QObject::tr("The Life bonus this item gives"), "Bonus to vitality=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to Life modifier"), QObject::tr("A random modifier between 0 and this value will be added to the Life bonus"), "Bonus to vitality modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to CPU"), QObject::tr("The CPU bonus this item gives"), "Bonus to magic=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to CPU modifier"), QObject::tr("A random modifier between 0 and this value will be added to the CPU bonus"), "Bonus to magic modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to Strength"), QObject::tr("The Dexterity bonus this item gives"), "Bonus to strength=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to Strength modifier"), QObject::tr("A random modifier between 0 and this value will be added to the Dexterity bonus"), "Bonus to strength modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to all attributes"), QObject::tr("All four primary attributes will get this bonus"), "Bonus to all attributes=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to all attributes modifier"), QObject::tr("A random modifier between 0 and this value will be added to the bonus to all four primary attributes"), "Bonus to all attributes modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to health"), QObject::tr(""), "Bonus to life=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to health modifier"), QObject::tr(""), "Bonus to life modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to core temperature"), QObject::tr(""), "Bonus to mana=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to core temperature modifier"), QObject::tr(""), "Bonus to mana modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to tohit"), QObject::tr(""), "Bonus to tohit=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to tohit modifier"), QObject::tr(""), "Bonus to tohit modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to armor class or damage"), QObject::tr(""), "Bonus to armor class or damage=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Bonus to armor class or damage modifier"), QObject::tr(""), "Bonus to armor class or damage modifier=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new FloatNodeType(QObject::tr("Bonus to health recovery"), QObject::tr(""), "Bonus to life recovery=", 0.0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new FloatNodeType(QObject::tr("Bonus to core temperature recovery"), QObject::tr(""), "Bonus to mana recovery=", 0.0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Light radius bonus"), QObject::tr(""), "Light radius bonus=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new FloatNodeType(QObject::tr("Price factor"), QObject::tr(""), "Price factor=", 3.0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);
	property = new IntegerNodeType(QObject::tr("Level"), QObject::tr(""), "Level=", 0, true, prefixlist);
	prefix->addProperty(property);
	suffix->addProperty(property);

	return prefixlist;
}



DatafileSyntaxes* DatafileSyntaxes::m_instance = 0;

DatafileSyntaxes* DatafileSyntaxes::instance() {
	if (m_instance == 0)
		m_instance = new DatafileSyntaxes;
	return m_instance;
}

DatafileSyntaxes::DatafileSyntaxes() :
	QObject(0),
	m_types()
{
	m_types.append(getItemlistType(this));
	m_types.append(getPrefixlistType(this));
}

const CompoundNodeType* DatafileSyntaxes::detectFileType(QTextStream& stream, QString& line) {
	const CompoundNodeType* type = 0;
	forever {
		line = stream.readLine();
		if (line.isNull())
			return 0;
		foreach(type, m_types)
			if (type->canReadData(line))
				return type;
	}
}

CompoundDataNode* DatafileSyntaxes::readData(const QString& filename, QObject* parent) {
	Q_ASSERT(!filename.isEmpty());
	QFile file(filename);
	bool ok = file.open( QIODevice::ReadOnly );
	if (!ok) {
		qCritical("The specified file \"%s\" could not be opened for reading.", filename.toLocal8Bit().data());
		return 0;
	}
	
	QTextStream stream(&file);
	stream.setCodec("ISO 8859-15");
	QString line;
	const CompoundNodeType* type = instance()->detectFileType(stream, line);
	if (type == 0) {
		qCritical("The specified file \"%s\" does not contain valid data.", filename.toLocal8Bit().data());
		return 0;
	}

	CompoundDataNode* node = type->readData(line, stream, parent);
	Q_ASSERT(node != 0);
	return node;
}

bool DatafileSyntaxes::writeNode(const CompoundDataNode* node, const QString& filename) {
	Q_ASSERT(node != 0);
	QFile file(filename);
	bool ok = file.open( QIODevice::WriteOnly );
	if (!ok) {
		qCritical("The specified file \"%s\" could not be opened for writing.", filename.toLocal8Bit().data());
		return false;
	}
	QTextStream stream(&file);
	stream.setCodec("ISO 8859-15");
	stream.setFieldAlignment( QTextStream::AlignLeft);
	stream.setNumberFlags(QTextStream::ForcePoint);
	stream.setRealNumberNotation(QTextStream::FixedNotation);
	stream.setRealNumberPrecision(6);
	node->writeNode(stream);
	stream << "\n\n" << "*** End of this Freedroid data File ***" << endl;
	return true;
}
