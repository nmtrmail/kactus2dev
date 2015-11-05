//-----------------------------------------------------------------------------
// File: ComponentWizardViewsPage.cpp
//-----------------------------------------------------------------------------
// Project: Kactus 2
// Author: Esko Pekkarinen
// Date: 09.10.2014
//
// Description:
// Component wizard page for setting up views.
//-----------------------------------------------------------------------------

#include "ComponentWizardViewsPage.h"
#include "ComponentWizardPages.h"
#include "ComponentWizard.h"
#include "ViewListModel.h"

#include <editors/ComponentEditor/views/vieweditor.h>

#include <library/LibraryManager/libraryinterface.h>

#include <IPXACTmodels/Component/Component.h>
#include <IPXACTmodels/Component/View.h>
#include <IPXACTmodels/Component/ComponentInstantiation.h>
#include <IPXACTmodels/Component/DesignInstantiation.h>
#include <IPXACTmodels/Component/DesignConfigurationInstantiation.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::ComponentWizardViewsPage()
//-----------------------------------------------------------------------------
ComponentWizardViewsPage::ComponentWizardViewsPage(LibraryInterface* lh,
    QSharedPointer<ParameterFinder> parameterFinder, QSharedPointer<ExpressionFormatter> expressionFormatter,
    ComponentWizard* parent)
    : QWizardPage(parent), 
    library_(lh), 
    parent_(parent),
    editorTabs_(new QTabWidget(this)),
    addButton_(new QPushButton(QIcon(":/icons/common/graphics/add.png"), QString(), this)),
    removeButton_(new QPushButton(QIcon(":/icons/common/graphics/remove.png"), QString(), this)),
    viewList_(new QListView(this)),
    viewModel_(new ViewListModel(this)),
    parameterFinder_(parameterFinder),
    expressionFormatter_(expressionFormatter)
{
    setTitle(tr("Views"));
    setSubTitle(tr("Setup the views for the component."));
    setFinalPage(true);

    viewList_->setSelectionMode(QAbstractItemView::SingleSelection);
    viewList_->setModel(viewModel_);
    
    setupLayout();

    connect(addButton_, SIGNAL(clicked()), this, SLOT(onViewAdded()), Qt::UniqueConnection);
    connect(removeButton_, SIGNAL(clicked()), this, SLOT(onViewRemoved()), Qt::UniqueConnection);
    connect(viewList_, SIGNAL(clicked(QModelIndex const&)), 
        this, SLOT(onViewSelected(QModelIndex const&)), Qt::UniqueConnection);
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::~ComponentWizardViewsPage()
//-----------------------------------------------------------------------------
ComponentWizardViewsPage::~ComponentWizardViewsPage()
{

}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::nextId()
//-----------------------------------------------------------------------------
int ComponentWizardViewsPage::nextId() const
{
    return ComponentWizardPages::CONCLUSION;
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::initializePage()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::initializePage()
{   
   editorTabs_->clear();

   QSharedPointer<Component> component = parent_->getComponent();
   viewModel_->setComponent(component);
   
   foreach(QSharedPointer<View> view, *component->getViews())
   {
       createEditorForView(component, view);
   }

   removeButton_->setEnabled(parent_->getComponent()->getViews()->count() > 1);
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::isComplete()
//-----------------------------------------------------------------------------
bool ComponentWizardViewsPage::isComplete() const
{
    for(int i = 0; i < editorTabs_->count(); i++)
    {
        ViewEditor* editor = dynamic_cast<ViewEditor*>(editorTabs_->widget(i));
        if (!editor->isValid())
        {         
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::onViewEdited()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::onViewEdited()
{    
    int currentIndex = editorTabs_->currentIndex();

    editorTabs_->setTabText(currentIndex, parent_->getComponent()->getViewNames().at(currentIndex));
    updateIconForTab(currentIndex);
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::onViewAdded()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::onViewAdded()
{
    viewModel_->addView();

    QSharedPointer<Component> component = parent_->getComponent(); 
    QSharedPointer<View> createdView = component->getViews()->last();

    createEditorForView(component, createdView);

    removeButton_->setEnabled(true);

    emit completeChanged();
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::onViewRemoved()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::onViewRemoved()
{
    QModelIndexList selectedIndexes = viewList_->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
    {
        return;
    }

    QModelIndex selectedIndex = selectedIndexes.first();
    QString selectedView = parent_->getComponent()->getViewNames().at(selectedIndex.row());    
    
    removeEditorsForView(selectedView);

    viewModel_->removeView(selectedIndex);

    removeButton_->setEnabled(parent_->getComponent()->getViews()->count() > 1);

    emit completeChanged();
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::onViewSelected()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::onViewSelected(QModelIndex const& index)
{
    if (index.isValid())
    {
        editorTabs_->setCurrentIndex(index.row());
    }
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::createEditorForView()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::createEditorForView(QSharedPointer<Component> component, QSharedPointer<View> view)
{
    QSharedPointer<ComponentInstantiation> componentInstantiation =
        getReferencedComponentInstantiation(component, view);
    QSharedPointer<DesignInstantiation> designInstantiation = getReferencedDesignInstantiation(component, view);
    QSharedPointer<DesignConfigurationInstantiation> designConfigurationInstantiation =
        getReferencedDesignConfigurationInstantiation(component, view);

    ViewEditor* editor = new ViewEditor(component, view, componentInstantiation, designInstantiation,
        designConfigurationInstantiation, library_, parameterFinder_, expressionFormatter_, this);

    int editorIndex = editorTabs_->addTab(editor, view->name());

    updateIconForTab(editorIndex);

    connect(editor, SIGNAL(contentChanged()), this, SLOT(onViewEdited()), Qt::UniqueConnection);
    connect(editor, SIGNAL(contentChanged()), this, SIGNAL(completeChanged()), Qt::UniqueConnection);    
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::updateIconForTab()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::updateIconForTab(int tabIndex) const
{
    ViewEditor* editor = dynamic_cast<ViewEditor*>(editorTabs_->widget(tabIndex));
    if (editor->isValid())
    {
        editorTabs_->setTabIcon(tabIndex, QIcon());        
    }
    else
    {
        editorTabs_->setTabIcon(tabIndex, QIcon(":icons/common/graphics/exclamation.png"));
    }
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::removeEditorsForView()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::removeEditorsForView(QString const& viewName)
{
    for(int i = 0; i < editorTabs_->count(); i++)
    {
        if (editorTabs_->tabText(i) == viewName)
        {         
            QWidget* removedEditor = editorTabs_->widget(i);
            editorTabs_->removeTab(i);
            delete removedEditor;
            i--;
        }
    }
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::setupLayout()
//-----------------------------------------------------------------------------
void ComponentWizardViewsPage::setupLayout()
{
    viewList_->setMaximumHeight(75);
    editorTabs_->setContentsMargins(2, 2, 2, 2);

    QVBoxLayout* topLayout = new QVBoxLayout(this);

    QDialogButtonBox* viewListButtons = new QDialogButtonBox(Qt::Vertical, this);
    viewListButtons->addButton(addButton_, QDialogButtonBox::ActionRole);
    viewListButtons->addButton(removeButton_, QDialogButtonBox::ActionRole);
    
    QHBoxLayout* viewsLayout = new QHBoxLayout();
    viewsLayout->addWidget(viewList_);
    viewsLayout->addWidget(viewListButtons);

    topLayout->addLayout(viewsLayout);
    topLayout->addWidget(editorTabs_, 1);
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::getReferencedComponentInstantiation()
//-----------------------------------------------------------------------------
QSharedPointer<ComponentInstantiation> ComponentWizardViewsPage::getReferencedComponentInstantiation(
    QSharedPointer<Component> component, QSharedPointer<View> targetView) const
{
    if (!targetView->getComponentInstantiationRef().isEmpty())
    {
        foreach (QSharedPointer<ComponentInstantiation> instantiation, *component->getComponentInstantiations())
        {
            if (instantiation->name() == targetView->getComponentInstantiationRef())
            {
                return instantiation;
            }
        }
    }

    return QSharedPointer<ComponentInstantiation>();
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::getReferencedDesignInstantiation()
//-----------------------------------------------------------------------------
QSharedPointer<DesignInstantiation> ComponentWizardViewsPage::getReferencedDesignInstantiation(
    QSharedPointer<Component> component, QSharedPointer<View> targetView) const
{
    if (!targetView->getDesignInstantiationRef().isEmpty())
    {
        foreach (QSharedPointer<DesignInstantiation> instantiation, *component->getDesignInstantiations())
        {
            if (instantiation->name() == targetView->getDesignInstantiationRef())
            {
                return instantiation;
            }
        }
    }

    return QSharedPointer<DesignInstantiation>();
}

//-----------------------------------------------------------------------------
// Function: ComponentWizardViewsPage::getReferencedDesignConfigurationInstantiation()
//-----------------------------------------------------------------------------
QSharedPointer<DesignConfigurationInstantiation> ComponentWizardViewsPage::
    getReferencedDesignConfigurationInstantiation(QSharedPointer<Component> component,
    QSharedPointer<View> targetView) const
{
    if (!targetView->getDesignConfigurationInstantiationRef().isEmpty())
    {
        foreach (QSharedPointer<DesignConfigurationInstantiation> instantiation,
            *component->getDesignConfigurationInstantiations())
        {
            if (instantiation->name() == targetView->getDesignConfigurationInstantiationRef())
            {
                return instantiation;
            }
        }
    }

    return QSharedPointer<DesignConfigurationInstantiation>();
}
