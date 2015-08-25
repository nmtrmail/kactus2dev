//-----------------------------------------------------------------------------
// File: PortReference.h
//-----------------------------------------------------------------------------
// Project: Kactus 2
// Author: Mikko Teuho
// Date: 25.08.2015
//
// Description:
// Describes either an internal or external port reference in an ad-hoc connection.
//-----------------------------------------------------------------------------

#ifndef PORTREFERENCE_H
#define PORTREFERENCE_H

#include <IPXACTmodels/ipxactmodels_global.h>

#include <IPXACTmodels/common/PartSelect.h>

#include <QString>

//-----------------------------------------------------------------------------
// Describes either an internal or external port reference in an ad-hoc connection.
//
// Internal port (ipxact:internalPortReference) references a port of a component instance.
//
// External port (ipxact:externalPortReference) references a port of the encompassing component.
//-----------------------------------------------------------------------------
class IPXACTMODELS_EXPORT PortReference
{

public:
    /*! \brief The constructor
    *
    * \param portReferenceNode A reference to a QDomNode where
    * the information should be parsed from.
    */
    //PortRef(QDomNode& portReferenceNode);

    /*!
     *  The constructor.
     *
     *      @param [in] portRef         The name of the referenced port.
     *      @param [in] componentRef    The name of the referenced component.
     */
    PortReference(QString const& portRef, QString const& componentRef = QString());

    /*!
     *  Copy constructor.
     */
    PortReference(const PortReference& other);

    /*!
     *  The destructor.
     */
    virtual ~PortReference();

    /*!
     *  Assignment operator.
     */
    PortReference& operator=(const PortReference& other);

    /*!
     *  Gets the name of the referenced port.     
     *
     *      @return The referenced port.
     */
    QString getPortRef() const;

    /*!
     *  Gets the name of the referenced component instance.     
     *
     *      @return The name of the referenced component instance.
     *
     *      @remark Only for internal port references.
     */
    QString getComponentRef() const;

    /*! \brief Check if the port ref is in a valid state.
    * 
    * \param externalRef Must be true for external refs so component reference 
    * is not checked.
    * \param instanceNames List containing the component instance names from
    * the containing design.
    * \param errorList The list to add the possible error messages to.
    * \param parentIdentifier String from parent to help to identify the location of the error.
    *
    * \return bool True if the state is valid and writing is possible.
    */
    //bool isValid(bool externalRef, const QStringList& instanceNames,
    //    QStringList& errorList, const QString& parentIdentifier) const;

    /*! \brief Check if the port ref is in a valid state.
    * 
    * \param externalRef Must be true for external refs so component reference 
    * is not checked.
    * \param instanceNames List containing the component instance names from
    * the containing design.
    * 
    * \return bool True if the state is valid and writing is possible.
    */
    //bool isValid(bool externalRef, const QStringList& instanceNames) const;

    /*!
     *  Set port reference.
     *
     *      @param [in] newPortRef  The new port reference.
     */
    void setPortRef(QString const& newPortRef);

    /*!
     *  Set the component reference.
     *
     *      @param [in] newComponentRef     The new component reference.
     */
    void setComponentRef(QString const& newComponentRef);

    /*!
     *  Get the presence.
     *
     *      @return The presence.
     */
    QString getIsPresent() const;

    /*!
     *  Set the presence.
     *
     *      @param [in] newIsPresent    The new presence.
     */
    void setIsPresent(QString const& newIsPresent);

    /*!
     *  Get the part select.
     *
     *      @return The part select.
     */
    QSharedPointer<PartSelect> getPartSelect() const;

    /*!
     *  Set the part select.
     *
     *      @param [in] newPartSelect   The new part select.
     */
    void setPartSelect(QSharedPointer<PartSelect> newPartSelect);

private:

    //-----------------------------------------------------------------------------
    // Data.
    //-----------------------------------------------------------------------------

    //! The name of the referenced port.
    QString portRef_;

    //! The name of the referenced component instance.
    QString componentRef_;

    //! The presence.
    QString isPresent_;

    //! The part select.
    QSharedPointer<PartSelect> partSelect_;
};
#endif // PORTREFERENCE_H
