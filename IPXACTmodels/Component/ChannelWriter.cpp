//-----------------------------------------------------------------------------
// File: ChannelWriter.cpp
//-----------------------------------------------------------------------------
// Project: Kactus 2
// Author: Janne Virtanen
// Date: 21.09.2015
//
// Description:
// Writer class for IP-XACT Channel element.
//-----------------------------------------------------------------------------

#include "ChannelWriter.h"

#include <IPXACTmodels/common/NameGroupWriter.h>

//-----------------------------------------------------------------------------
// Function: ChannelWriter::ChannelWriter()
//-----------------------------------------------------------------------------
ChannelWriter::ChannelWriter() : CommonItemsWriter()
{

}

//-----------------------------------------------------------------------------
// Function: ChannelWriter::~ChannelWriter()
//-----------------------------------------------------------------------------
ChannelWriter::~ChannelWriter()
{

}

//-----------------------------------------------------------------------------
// Function: ChannelWriter::createChannelFrom()
//-----------------------------------------------------------------------------
void ChannelWriter::writeChannel(QXmlStreamWriter& writer, QSharedPointer<Channel> channel) const
{
	// Start the element, write name group and presence.
	writer.writeStartElement(QStringLiteral("ipxact:channel"));
	writeNameGroup(writer, channel);
	writeIsPresent(writer, channel);

	foreach (QString const& busInterfaceReference, channel->getInterfaces())
	{
		writer.writeStartElement(QStringLiteral("ipxact:busInterfaceRef"));
		writer.writeTextElement(QStringLiteral("ipxact:localName"), busInterfaceReference);
		writer.writeEndElement();
	}

	writer.writeEndElement();
	return;
}

//-----------------------------------------------------------------------------
// Function: ChannelWriter::writeNameGroup()
//-----------------------------------------------------------------------------
void ChannelWriter::writeNameGroup(QXmlStreamWriter& writer, QSharedPointer<Channel> channel) const
{
	NameGroupWriter nameGroupWriter;
	nameGroupWriter.writeNameGroup(writer, channel);
}

//-----------------------------------------------------------------------------
// Function: ChannelWriter::writeIsPresent()
//-----------------------------------------------------------------------------
void ChannelWriter::writeIsPresent(QXmlStreamWriter& writer, QSharedPointer<Channel> channel) const
{
	if (!channel->getIsPresent().isEmpty())
	{
		writer.writeTextElement(QStringLiteral("ipxact:isPresent"), channel->getIsPresent());
	}
}