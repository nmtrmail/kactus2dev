//-----------------------------------------------------------------------------
// File: busifinterfaceslave.h
//-----------------------------------------------------------------------------
// Project: Kactus2
// Author: Antti Kamppi
// Date: 7.4.2011
//
// Description:
// Editor to the slave details of a bus interface.
//-----------------------------------------------------------------------------

#ifndef BUSIFINTERFACESLAVE_H
#define BUSIFINTERFACESLAVE_H

#include "busifinterfacemodeeditor.h"
#include "bridgeseditor.h"

#include <editors/ComponentEditor/common/ReferenceSelector/ReferenceSelector.h>
#include <editors/ComponentEditor/instantiations/filesetrefeditor.h>

#include <IPXACTmodels/Component/SlaveInterface.h>

#include <QSharedPointer>

class BusInterface;
class Component;

//-----------------------------------------------------------------------------
//! Editor to the slave details of a bus interface.
//-----------------------------------------------------------------------------
class BusIfInterfaceSlave : public BusIfInterfaceModeEditor 
{
	Q_OBJECT

public:

	/*! The constructor
	 *
	 *        @param [in]   busif       The bus interface being edited.
	 *        @param [in]   component   The component being edited.
	 *        @param [in]   parent      The owner of this editor.
	 *
	*/
	BusIfInterfaceSlave(QSharedPointer<BusInterface> busif,	QSharedPointer<Component> component, QWidget* parent);
	
	//! The destructor
	virtual ~BusIfInterfaceSlave();

	/*! Check for the validity of the edited item.
	*
	*       @return True if item is valid.
	*/
	virtual bool isValid() const;

	/*! Restore the changes made in the editor back to ones in the model.
	*
	*/
	virtual void refresh();

	/*! Get the interface mode of the editor
	 * 
	 *      @return General::InterfaceMode Specifies the interface mode.
	*/
	virtual General::InterfaceMode getInterfaceMode() const;

	//! Save the interface mode-specific details to the bus interface.
	virtual void saveModeSpecific();

private slots:

	/*!
     *  Handler for changes in the memory map reference.
     *
     *      @param [in] newMemoryMapName    The name of the selected memory map.
     */
	void onMemoryMapChange(QString const& newMemoryMapName);

    /*!
     *  Handler for selecting the memory map.
     *
     *      @param [in] checked     True, if the memory map was selected.
     */
    void onMemoryMapSelected(bool checked);

    /*!
     *  Handler for selecting the transparent bridges.
     *
     *      @param [in] checked     True, if the transparent bridges was selected.
     */
    void onTransparentBridgeSelected(bool checked);

    /*!
     *  Handler for changes in the file set references.
     */
    void onFileSetReferencesChanged();

private:

	//! No copying
	BusIfInterfaceSlave(const BusIfInterfaceSlave& other);    

	//! No assignment
	BusIfInterfaceSlave& operator=(const BusIfInterfaceSlave& other);

    /*!
     *  Setup the current file set references.
     */
    void setupFileSetReferences();

    /*!
     *  Sets the editor layout.
     */
    void setupLayout();

    //-----------------------------------------------------------------------------
    // Data.
    //-----------------------------------------------------------------------------

	//! The slave interface mode being edited.
	QSharedPointer<SlaveInterface> slave_;

    //! Holder for the memory map option.
    QGroupBox* memoryMapBox_;

	//! The editor to select a memory map from the component.
	ReferenceSelector memoryMapReferenceSelector_;

	//! The editor to edit the bridges of a slave interface.
	BridgesEditor bridges_;

    //! Editor to set the file set references.
    FileSetRefEditor fileSetRefs_;
};

#endif // BUSIFINTERFACESLAVE_H
