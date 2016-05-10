//-----------------------------------------------------------------------------
// File: AdhocEditor.cpp
//-----------------------------------------------------------------------------
// Project: Kactus 2
// Author: Mikko Teuho
// Date: 29.04.2016
//
// Description:
// Editor to edit the details of an Ad hoc port in designAd-hoc editor.
//-----------------------------------------------------------------------------

#include "AdhocEditor.h"

#include <common/graphicsItems/ComponentItem.h>

#include <editors/ComponentEditor/common/ExpressionEditor.h>
#include <editors/ComponentEditor/common/ParameterCompleter.h>
#include <editors/ComponentEditor/common/ComponentParameterFinder.h>
#include <editors/ComponentEditor/common/IPXactSystemVerilogParser.h>
#include <editors/ComponentEditor/parameters/ComponentParameterModel.h>

#include <designEditors/common/DesignDiagram.h>
#include <designEditors/HWDesign/AdHocItem.h>
#include <designEditors/HWDesign/AdHocPortItem.h>
#include <designEditors/HWDesign/AdHocInterfaceItem.h>

#include <IPXACTmodels/common/validators/ValueFormatter.h>
#include <IPXACTmodels/Design/Design.h>

#include <QFormLayout>

//-----------------------------------------------------------------------------
// Function: AdhocEditor::AdHocEditor()
//-----------------------------------------------------------------------------
AdHocEditor::AdHocEditor(QWidget* parent):
QWidget(parent),
componentFinder_(new ComponentParameterFinder(QSharedPointer<Component>(0))),
expressionParser_(new IPXactSystemVerilogParser(componentFinder_)),
portName_(new QLabel(this)),
portDirection_(new QLabel(this)),
leftBoundValue_(new QLabel(this)),
rightBoundValue_(new QLabel(this)),
tiedValueEditor_(new ExpressionEditor(componentFinder_, this)),
containedPortItem_()
{
    tiedValueEditor_->setFixedHeight(20);

    ComponentParameterModel* parameterModel = new ComponentParameterModel(componentFinder_, this);
    parameterModel->setExpressionParser(expressionParser_);

    ParameterCompleter* tiedValueCompleter = new ParameterCompleter(this);
    tiedValueCompleter->setModel(parameterModel);

    tiedValueEditor_->setAppendingCompleter(tiedValueCompleter);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    setupLayout();

    connect(tiedValueEditor_, SIGNAL(editingFinished()), this, SLOT(onTiedValueChanged()), Qt::UniqueConnection);
}

//-----------------------------------------------------------------------------
// Function: AdHocEditor::~AdHocEditor()
//-----------------------------------------------------------------------------
AdHocEditor::~AdHocEditor()
{

}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::setupLayout()
//-----------------------------------------------------------------------------
void AdHocEditor::setupLayout()
{
    QFormLayout* overallLayout = new QFormLayout();

    overallLayout->addRow(tr("Name:"), portName_);
    overallLayout->addRow(tr("Direction:"), portDirection_);
    overallLayout->addRow(tr("Left bound:"), leftBoundValue_);
    overallLayout->addRow(tr("Right bound:"), rightBoundValue_);
    overallLayout->addRow(tr("Tied value:"), tiedValueEditor_);

    setLayout(overallLayout);
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::setAdhocPort()
//-----------------------------------------------------------------------------
void AdHocEditor::setAdhocPort(AdHocItem* endPoint)
{
    if(endPoint->isAdHoc())
    {
        containedPortItem_ = endPoint;

        portName_->show();
        portDirection_->show();
        leftBoundValue_->show();
        rightBoundValue_->show();
        tiedValueEditor_->show();

        AdHocPortItem* adhocPortItem = dynamic_cast<AdHocPortItem*>(containedPortItem_);
        AdHocInterfaceItem* adhocInterfaceItem = dynamic_cast<AdHocInterfaceItem*>(containedPortItem_);

        QString containingItemName = "";
        QSharedPointer<Port> referencedPort = containedPortItem_->getPort();

        if (adhocPortItem)
        {
            ComponentItem* instanceItem = adhocPortItem->encompassingComp();
            if (instanceItem)
            {
                containingItemName = instanceItem->name();
            }
        }

        if (referencedPort)
        {
            componentFinder_->setComponent(containedPortItem_->getOwnerComponent());

            DirectionTypes::Direction direction = referencedPort->getDirection();

            portName_->setText(referencedPort->name());
            portDirection_->setText(DirectionTypes::direction2Str(direction));
            leftBoundValue_->setText(expressionParser_->parseExpression(referencedPort->getLeftBound()));
            rightBoundValue_->setText(expressionParser_->parseExpression(referencedPort->getRightBound()));

            QString tiedValue = "";

            if ((adhocPortItem && direction == DirectionTypes::IN) ||
                (adhocInterfaceItem && direction == DirectionTypes::OUT) ||
                direction == DirectionTypes::INOUT)
            {
                DesignDiagram* containingDiagram = dynamic_cast<DesignDiagram*>(containedPortItem_->scene());
                bool locked = containingDiagram->isProtected();

                tiedValue = getTiedValue(containingItemName);

                tiedValueEditor_->setEnabled(!locked);
            }
            else
            {
                tiedValueEditor_->setEnabled(false);
            }

            tiedValueEditor_->blockSignals(true);
            tiedValueEditor_->setExpression(tiedValue);
            tiedValueEditor_->setToolTip(formattedValueFor(tiedValue));
            tiedValueEditor_->blockSignals(false);

            parentWidget()->setMaximumHeight(QWIDGETSIZE_MAX);
        }
    }
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::getTiedValue()
//-----------------------------------------------------------------------------
QString AdHocEditor::getTiedValue(QString const& instanceName) const
{
    QSharedPointer<AdHocConnection> connection = getTiedConnection(instanceName);
    if (connection)
    {
        return connection->getTiedValue();
    }
    else
    {
        return QString("");
    }
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::getTiedConnection()
//-----------------------------------------------------------------------------
QSharedPointer<AdHocConnection> AdHocEditor::getTiedConnection(QString const& instanceName) const
{
    DesignDiagram* containingDiagram = dynamic_cast<DesignDiagram*>(containedPortItem_->scene());
    QSharedPointer<Design> containingDesign = containingDiagram->getDesign();

    foreach (QSharedPointer<AdHocConnection> connection, *containingDesign->getAdHocConnections())
    {
        if (!instanceName.isEmpty())
        {
            foreach (QSharedPointer<PortReference> internalReference, *connection->getInternalPortReferences())
            {
                if (internalReference->getPortRef() == containedPortItem_->name() &&
                    internalReference->getComponentRef() == instanceName &&
                    !connection->getTiedValue().isEmpty())
                {
                    return connection;
                }
            }
        }
        else
        {
            foreach (QSharedPointer<PortReference> externalReference, *connection->getExternalPortReferences())
            {
                if (externalReference->getPortRef() == containedPortItem_->name() &&
                    !connection->getTiedValue().isEmpty())
                {
                    return connection;
                }
            }
        }
    }

    return QSharedPointer<AdHocConnection>();
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::formattedValueFor()
//-----------------------------------------------------------------------------
QString AdHocEditor::formattedValueFor(QString const& expression) const
{
    if (expression.isEmpty() || expressionParser_->isPlainValue(expression))
    {
        return expression;
    }
    else if (expressionParser_->isValidExpression(expression))
    {
        ValueFormatter formatter;
        return formatter.format(
            expressionParser_->parseExpression(expression), expressionParser_->baseForExpression(expression));
    }
    else
    {
        return tr("n/a");
    }
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::onTiedValueChanged()
//-----------------------------------------------------------------------------
void AdHocEditor::onTiedValueChanged()
{
    tiedValueEditor_->finishEditingCurrentWord();

    QString newTiedValue = tiedValueEditor_->getExpression();

    QString formattedTiedValue = formattedValueFor(newTiedValue);

    tiedValueEditor_->setToolTip(formattedTiedValue);

    DesignDiagram* containingDiagram = dynamic_cast<DesignDiagram*>(containedPortItem_->scene());
    QSharedPointer<Design> containingDesign = containingDiagram->getDesign();

    QString instanceName = "";
    ComponentItem* containingInstance = containedPortItem_->encompassingComp();
    if (containingInstance)
    {
        instanceName = containingInstance->name();
    }

    QSharedPointer<AdHocConnection> connection = getTiedConnection(instanceName);

    if (!newTiedValue.isEmpty())
    {
        if (connection)
        {
            connection->setTiedValue(newTiedValue);
        }
        else
        {
            createConnectionForTiedValue(newTiedValue, containingDesign);
        }
    }
    else if (connection)
    {
        containingDesign->getAdHocConnections()->removeAll(connection);
    }

    drawTieOffItem(formattedTiedValue);

    emit contentChanged();
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::createConnectionForTiedValue()
//-----------------------------------------------------------------------------
void AdHocEditor::createConnectionForTiedValue(QString const& newTiedValue,
    QSharedPointer<Design> containingDesign)
{
    QString connectionName = createNameForTiedValueConnection();

    QSharedPointer<AdHocConnection> connection (new AdHocConnection(connectionName));
    connection->setTiedValue(newTiedValue);

    QSharedPointer<PortReference> portReference (new PortReference(containedPortItem_->name()));

    ComponentItem* containingComponent = containedPortItem_->encompassingComp();
    if (containingComponent)
    {
        portReference->setComponentRef(containingComponent->name());
        connection->getInternalPortReferences()->append(portReference);
    }
    else
    {
        connection->getExternalPortReferences()->append(portReference);
    }

    containingDesign->getAdHocConnections()->append(connection);
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::createNameForTiedValueConnection()
//-----------------------------------------------------------------------------
QString AdHocEditor::createNameForTiedValueConnection() const
{
    ComponentItem* containingComponent = containedPortItem_->encompassingComp();

    QString instanceName = "";

    if (containingComponent)
    {
        instanceName = containingComponent->name();
        instanceName.append("_");
    }

    QString portName = containedPortItem_->name();

    QString tiedValuePart = "_to_tiedValue";

    return instanceName + portName + tiedValuePart;
}

//-----------------------------------------------------------------------------
// Function: AdhocEditor::drawTieOffItem()
//-----------------------------------------------------------------------------
void AdHocEditor::drawTieOffItem(QString const& formattedTieOff) const
{
    int intTiedValue = expressionParser_->parseExpression(formattedTieOff).toInt();

    if (formattedTieOff.isEmpty())
    {
        containedPortItem_->removeTieOffItem();
    }
    else if (formattedTieOff== tr("n/a"))
    {
        containedPortItem_->createNonResolvableTieOff();
    }
    else if (intTiedValue == 1)
    {
        containedPortItem_->createHighTieOff();
    }
    else if (intTiedValue == 0)
    {
        containedPortItem_->createLowTieOff();
    }
    else
    {
        containedPortItem_->createNumberedTieOff();
    }
}

//-----------------------------------------------------------------------------
// Function: AdHocEditor::clear()
//-----------------------------------------------------------------------------
void AdHocEditor::clear()
{
    portName_->hide();
    portDirection_->hide();
    leftBoundValue_->hide();
    rightBoundValue_->hide();
    tiedValueEditor_->hide();

    parentWidget()->setMaximumHeight(20);
}