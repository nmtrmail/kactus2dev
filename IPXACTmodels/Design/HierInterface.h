//-----------------------------------------------------------------------------
// File: HierInterface.h
//-----------------------------------------------------------------------------
// Project: Kactus 2
// Author: Mikko Teuho
// Date: 19.08.2015
//
// Description:
// Describes the ipxact:hierInterface element in an IP-XACT document.
//-----------------------------------------------------------------------------

#ifndef HIERINTERFACE_H
#define HIERINTERFACE_H

#include <IPXACTmodels/common/Extendable.h>
#include <IPXACTmodels/ipxactmodels_global.h>

#include <QString>

//-----------------------------------------------------------------------------
// ipxact:hierInterface element. Works as a base class for other design interfaces.
//-----------------------------------------------------------------------------
class IPXACTMODELS_EXPORT HierInterface : public Extendable
{

public:
    /*! The constructor
     *
     *       @param [in]   interfaceNode   A QDomNode where the information should be parsed from.
     */
    //HierInterface(QDomNode & interfaceNode);

    /*!
     *  The constructor.
     *
     *      @param [in] busRef  The name of the referenced bus interface.
     */
    HierInterface(QString busRef = QString(""));

    /*!
     *  Copy constructor.
     */
    HierInterface(const HierInterface& other);

    /*!
     *  The destructor.
     */
    virtual ~HierInterface();

    /*!
     *  Assignment operator.
     */
    HierInterface& operator=(const HierInterface& other);

    /*!
     *  Comparison operator.
     *
     *      @return True, if the bus interfaces reference the same interface.
     */
    bool operator==(const HierInterface& other);

    /*!
     *  The != operator.
     */
    bool operator!=(const HierInterface& other);

    /*!
     *  The < operator.
     */
    bool operator<(const HierInterface& other);

    /*! Check if the interface is in a valid state.
     * 
     *       @param [in] instanceNames       Contains the names of component instances in containing design.
     *       @param [in] errorList           The list to add the possible error messages to.
     *       @param [in] parentIdentifier    String from parent to help to identify the location of the error.
     *
     *       @return bool True if the state is valid and writing is possible.
     */
    //bool isValid(QStringList const& instanceNames, QStringList& errorList, QString const& parentIdentifier) const;

    /*! Check if the interface is in a valid state.
     * 
     *       @param [in] instanceNames     Contains the names of component instances in containing design.
     * 
     *       @return bool True if the state is valid and writing is possible.
     */
    //bool isValid(const QStringList& instanceNames) const;    

    /*!
     *  Get the name of the referenced bus interface.
     *
     *      @return The name of the referenced bus interface.
     */
    QString getBusReference() const;

    /*!
     *  Set the bus interface reference.
     *
     *      @param [in] newBusReference     The name of the new bus interface reference.
     */
    void setBusReference(QString const& newBusReference);

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
     *  Get the description.
     *
     *      @return The description.
     */
    QString getDescription() const;

    /*!
     *  Set the description.
     *
     *      @param [in] newDescription  The new description.
     */
    void setDescription(QString const& newDescription);

private:

    //-----------------------------------------------------------------------------
    // Data.
    //-----------------------------------------------------------------------------

    //! The name of the referenced bus interface.
    QString busRef_;

    //! The presence.
    QString isPresent_;

    //! The description.
    QString description_;
};
#endif // HIERINTERFACE_H
