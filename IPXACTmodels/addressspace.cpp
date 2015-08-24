/* 
 *
 *  Created on: 28.7.2010
 *      Author: Antti Kamppi
 */

#include "addressspace.h"
#include "memorymap.h"
#include "XmlUtils.h"
#include "GenericVendorExtension.h"

#include <common/utils.h>

#include <IPXACTmodels/common/Parameter.h>
#include <IPXACTmodels/common/ParameterReader.h>
#include <IPXACTmodels/common/ParameterWriter.h>

#include <QDomNode>
#include <QDomNamedNodeMap>
#include <QSharedPointer>
#include <QString>
#include <QObject>
#include <QList>
#include <QXmlStreamWriter>

const int DEFAULT_ADDRESS_UNIT_BITS = 8;

// the constructor
AddressSpace::AddressSpace(QDomNode &addrSpaceNode):
NameGroup(),
range_(QString()),
rangeAttributes_(),
width_(-1), 
widthAttributes_(),
segments_(),
addressUnitBits_(DEFAULT_ADDRESS_UNIT_BITS), 
localMemoryMap_(0),
parameters_(),
vendorExtensions_()
{
	for (int i = 0; i < addrSpaceNode.childNodes().count(); ++i) {
		QDomNode tempNode = addrSpaceNode.childNodes().at(i);

		// get range
		if (tempNode.nodeName() == QString("spirit:range")) {
			range_ = tempNode.childNodes().at(0).nodeValue();

			// get the attributes
			rangeAttributes_ = XmlUtils::parseAttributes(tempNode);
		}

		// get width
		else if (tempNode.nodeName() == QString("spirit:width")) {
			width_ = tempNode.childNodes().at(0).nodeValue().toInt();

			// get the attributes
			widthAttributes_ = XmlUtils::parseAttributes(tempNode);
		}

		else if (tempNode.nodeName() == QString("spirit:segments")) {
			for (int j = 0; j < tempNode.childNodes().count(); ++j) {
				QDomNode segmentNode = tempNode.childNodes().at(j);
				segments_.append(QSharedPointer<Segment>(
					new Segment(segmentNode)));
			}
		}

		// get addressUnitBits
		else if (tempNode.nodeName() == QString("spirit:addressUnitBits")) {
			addressUnitBits_ = tempNode.childNodes().at(0).nodeValue().toInt();
		}

		else if (tempNode.nodeName() == QString("spirit:localMemoryMap")) {
			localMemoryMap_ = QSharedPointer<MemoryMap>(
					new MemoryMap(tempNode));
		}

		else if (tempNode.nodeName() == QString("spirit:parameters"))
        {
            ParameterReader reader;
			for (int j = 0; j < tempNode.childNodes().count(); ++j) {
				QDomNode parameterNode = tempNode.childNodes().at(j);
				parameters_.append(reader.createParameterFrom(parameterNode));
			}
		}
        else if (tempNode.nodeName() == QString("spirit:vendorExtensions")) 
        {
            int extensionCount = tempNode.childNodes().count();
            for (int j = 0; j < extensionCount; ++j) {
                QDomNode extensionNode = tempNode.childNodes().at(j);                
                vendorExtensions_.append(QSharedPointer<VendorExtension>(new GenericVendorExtension(extensionNode)));
            }
        }
	}	
}

AddressSpace::AddressSpace( const AddressSpace &other ):
NameGroup(other),
range_(other.range_),
rangeAttributes_(other.rangeAttributes_),
width_(other.width_),
widthAttributes_(other.widthAttributes_),
segments_(),
addressUnitBits_(other.addressUnitBits_),
localMemoryMap_(),
parameters_(),
vendorExtensions_(other.vendorExtensions_)
 {
	if (other.localMemoryMap_) {
		localMemoryMap_ = QSharedPointer<MemoryMap>(
			new MemoryMap(*other.localMemoryMap_.data()));
	}

	parameters_.clear();
	foreach (QSharedPointer<Parameter> param, other.parameters_) {
		if (param) {
			QSharedPointer<Parameter> copy = QSharedPointer<Parameter>(
				new Parameter(*param.data()));
			parameters_.append(copy);
		}
	}

	segments_.clear();
	foreach (QSharedPointer<Segment> segment, other.segments_) {
		if (segment) {
			QSharedPointer<Segment> copy = QSharedPointer<Segment>(
				new Segment(*segment.data()));
			segments_.append(copy);
		}
	}
}

AddressSpace::AddressSpace():
NameGroup(),
range_(),
rangeAttributes_(),
width_(-1), 
widthAttributes_(),
segments_(),
addressUnitBits_(DEFAULT_ADDRESS_UNIT_BITS), 
localMemoryMap_(0),
parameters_(),
vendorExtensions_()
{
}

AddressSpace & AddressSpace::operator=( const AddressSpace &other ) {
	if (this != &other) {
		NameGroup::operator=(other);
		range_ = other.range_;
		rangeAttributes_ = other.rangeAttributes_;
		width_ = other.width_;
		widthAttributes_ = other.widthAttributes_;
		addressUnitBits_ = other.addressUnitBits_;
        vendorExtensions_ = other.vendorExtensions_;

		if (other.localMemoryMap_) {
			localMemoryMap_ = QSharedPointer<MemoryMap>(
				new MemoryMap(*other.localMemoryMap_.data()));
		}
		else
			localMemoryMap_ = QSharedPointer<MemoryMap>();

		parameters_.clear();
		foreach (QSharedPointer<Parameter> param, other.parameters_) {
			if (param) {
				QSharedPointer<Parameter> copy = QSharedPointer<Parameter>(
					new Parameter(*param.data()));
				parameters_.append(copy);
			}
		}

		segments_.clear();
		foreach (QSharedPointer<Segment> segment, other.segments_) {
			if (segment) {
				QSharedPointer<Segment> copy = QSharedPointer<Segment>(
					new Segment(*segment.data()));
				segments_.append(copy);
			}
		}
	}
	return *this;
}

// the destructor
AddressSpace::~AddressSpace() {
	rangeAttributes_.clear();
	widthAttributes_.clear();
	segments_.clear();
}

void AddressSpace::write(QXmlStreamWriter& writer) {
	writer.writeStartElement("spirit:addressSpace");

	writer.writeTextElement("spirit:name", name());

	if (!displayName().isEmpty()) {
		writer.writeTextElement("spirit:displayName", displayName());
	}

	if (!description().isEmpty()) {
		writer.writeTextElement("spirit:description", description());
	}

    // start the spirit:range tag
    writer.writeStartElement("spirit:range");

    // write the attributes for the element
    XmlUtils::writeAttributes(writer, rangeAttributes_);

    // write the value of the element and close the tag
    writer.writeCharacters(range_);
    writer.writeEndElement(); // spirit:range

    // start the spirit:width element
    writer.writeStartElement("spirit:width");

    // write the attributes for the element
    XmlUtils::writeAttributes(writer, widthAttributes_);

    // write the value of the element and close the tag
    writer.writeCharacters(QString::number(width_));
    writer.writeEndElement(); // spirit:width

	if (!segments_.isEmpty()) {

		writer.writeStartElement("spirit:segments");
		foreach (QSharedPointer<Segment> segment, segments_) {
			segment->write(writer);
		}
		writer.writeEndElement(); // spirit:segments
	}

	writer.writeTextElement("spirit:addressUnitBits", QString::number(addressUnitBits_));

	if (localMemoryMap_ && !localMemoryMap_->isEmpty()) {
		writer.writeStartElement("spirit:localMemoryMap");
		localMemoryMap_->write(writer);
		writer.writeEndElement(); // spirit:localMemoryMap
	}

	if (parameters_.size() != 0)
    {
		writer.writeStartElement("ipxact:parameters");

        ParameterWriter parameterWriter;
		// write each parameter
		for (int i = 0; i < parameters_.size(); ++i)
        {
			parameterWriter.writeParameter(writer, parameters_.at(i));
		}

		writer.writeEndElement(); // ipxact:parameters
	}

    if (!vendorExtensions_.isEmpty())
    {
        writer.writeStartElement("spirit:vendorExtensions");
        XmlUtils::writeVendorExtensions(writer, vendorExtensions_);
        writer.writeEndElement(); // spirit:vendorExtensions
    }

	writer.writeEndElement(); // spirit:addressSpace
}

//-----------------------------------------------------------------------------
// Function: addressspace::isValid()
//-----------------------------------------------------------------------------
bool AddressSpace::isValid(QSharedPointer<QList<QSharedPointer<Choice> > > componentChoices,
    QStringList remapStateNames, QStringList& errorList, const QString& parentIdentifier ) const
{
	bool valid = true;
	const QString thisIdentifier(QObject::tr("address space %1").arg(name()));

	if (name().isEmpty()) {
		errorList.append(QObject::tr("No name specified for %1 within %2").arg(
			thisIdentifier).arg(parentIdentifier));
		valid = false;
	}

	if (range_.isEmpty()) {
		errorList.append(QObject::tr("No range set for %1 within %2").arg(
			thisIdentifier).arg(parentIdentifier));
		valid = false;
	}

	if (width_ < 0) {
		errorList.append(QObject::tr("No width set for %1 within %2").arg(
			thisIdentifier).arg(parentIdentifier));
		valid = false;
	}

	foreach (QSharedPointer<Segment> segment, segments_) {
		if (!segment->isValid(errorList, thisIdentifier)) {
			valid = false;
		}
	}

	if (localMemoryMap_) {
		
		// if the local memory map contains definitions but is not valid
		if (!localMemoryMap_->isEmpty() && !localMemoryMap_->isValid(componentChoices, remapStateNames, errorList,
            thisIdentifier))
        {
			valid = false;
		}
	}

//     ParameterValidator2014 validator;
//     foreach (QSharedPointer<Parameter> param, parameters_)
//     {
//         errorList.append(validator.findErrorsIn(param.data(), parentIdentifier, componentChoices));
//         if (!validator.validate(param.data(), componentChoices)) 
//         {
//             valid = false;
//         }
//     }

	return valid;
}

//-----------------------------------------------------------------------------
// Function: addressspace::isValid()
//-----------------------------------------------------------------------------
bool AddressSpace::isValid(QSharedPointer<QList<QSharedPointer<Choice> > > componentChoices,
    QStringList remapStateNames) const
{
	if (name().isEmpty()) {
		return false;
	}

	if (range_.isEmpty()) {
		return false;
	}

	if (width_ < 0) {
		return false;
	}

	foreach (QSharedPointer<Segment> segment, segments_) {
		if (!segment->isValid()) {
			return false;
		}
	}

	if (localMemoryMap_) {

		// if the memory map contains definitions but is not valid
		if (!localMemoryMap_->isEmpty() && !localMemoryMap_->isValid(componentChoices, remapStateNames))
        {
			return false;
		}
	}

//     ParameterValidator2014 validator;
//     foreach (QSharedPointer<Parameter> param, parameters_)
//     {
//         if (!validator.validate(param.data(), componentChoices)) 
//         {
//             return false;
//         }
//     }
	return true;
}

void AddressSpace::setAddressUnitBits(unsigned int addressUnitBits) {
	addressUnitBits_ = addressUnitBits;
}

unsigned int AddressSpace::getAddressUnitBits() const {
	return addressUnitBits_;
}

void AddressSpace::setRange(const QString &range) {
	range_ = range;
}

QString AddressSpace::getRange() const {
	return range_;
}

const QMap<QString, QString>& AddressSpace::getRangeAttributes() {
	return rangeAttributes_;
}

void AddressSpace::setRangeAttributes(
		const QMap<QString, QString> &rangeAttributes) {
	// delete old range attributes
	rangeAttributes_.clear();

	// save new range attributes
	rangeAttributes_ = rangeAttributes;
}

int AddressSpace::getWidth() const {
	return width_;
}

const QMap<QString, QString>& AddressSpace::getWidthAttributes() {
	return widthAttributes_;
}

void AddressSpace::setWidthAttributes(
		const QMap<QString, QString> &widthAttributes) {
	// delete old width attributes
	widthAttributes_.clear();

	// save new width attributes
	widthAttributes_ = widthAttributes;
}

void AddressSpace::setWidth(int width) {
	width_ = width;
}

const QSharedPointer<MemoryMap> AddressSpace::getLocalMemoryMap() const {
	return localMemoryMap_;
}

QSharedPointer<MemoryMap> AddressSpace::getLocalMemoryMap() {
	if (!localMemoryMap_) {
		localMemoryMap_ = QSharedPointer<MemoryMap>(new MemoryMap());
	}
	return localMemoryMap_;
}

bool AddressSpace::hasLocalMemoryMap() const
{
    return localMemoryMap_;
}

void AddressSpace::setLocalMemoryMap( QSharedPointer<MemoryMap> localMemoryMap ) {
	if (localMemoryMap_) {
		localMemoryMap_.clear();
	}
	localMemoryMap_ = localMemoryMap;
}

QList<QSharedPointer<Segment> >& AddressSpace::getSegments() {
	return segments_;
}

const QList<QSharedPointer<Segment> >& AddressSpace::getSegments() const {
	return segments_;
}

void AddressSpace::setSegments( const QList<QSharedPointer<Segment> >& segments ) {
	segments_ = segments;
}

QList<QSharedPointer<Parameter> >& AddressSpace::getParameters() {
	return parameters_;
}

const QList<QSharedPointer<Parameter> >& AddressSpace::getParameters() const {
	return parameters_;
}

quint64 AddressSpace::getLastAddress() const
{
	quint64 range = General::str2Uint(range_);
	
	// if range is undefined
	if (range <= 0) {
		return 0;
	}
	return range - 1;
}

//-----------------------------------------------------------------------------
// Function: AddressSpace::setWidthExpression()
//-----------------------------------------------------------------------------
void AddressSpace::setWidthExpression(QString const& expression)
{
    widthAttributes_.insert("kactus2:widthExpression", expression);
}

//-----------------------------------------------------------------------------
// Function: AddressSpace::getWidthExpression()
//-----------------------------------------------------------------------------
QString AddressSpace::getWidthExpression()
{
    QString expression = widthAttributes_.value("kactus2:widthExpression");
    if (expression.isEmpty())
    {
        expression = QString::number(getWidth());
    }

    return expression;
}
