// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2024 The FreeCAD Project Association AISBL               *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/


#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTimer>
#include <QWidget>
#include <QStackedWidget>
#include <QShowEvent>

#include "StartView.h"
#include "ExamplesListDelegate.h"
#include "FileCardDelegate.h"
#include "FileCardView.h"
#include "FirstStartWidget.h"
#include "FlowLayout.h"
#include "GettingStartedCard.h"
#include "LearnLinksWidget.h"
#include "NewFileButton.h"
#include "OpenFileProxyModel.h"
#include <App/DocumentObject.h>
#include <App/Application.h>
#include <Base/Interpreter.h>
#include <Base/Tools.h>
#include <Gui/Action.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/ModuleIO.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>
#include <gsl/pointers>
#include <string>

using namespace StartGui;

TYPESYSTEM_SOURCE_ABSTRACT(StartGui::StartView, Gui::MDIView)  // NOLINT


namespace
{

QLabel* makeSectionLabel(const QString& text, QWidget* parent = nullptr)
{
    auto label = gsl::owner<QLabel*>(new QLabel(text, parent));
    label->setObjectName(QStringLiteral("startSectionLabel"));
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    return label;
}

}  // namespace


StartView::StartView(QWidget* parent)
    : Gui::MDIView(nullptr, parent)
    , _contents(new QStackedWidget(parent))
{
    setObjectName(QLatin1String("StartView"));
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    auto cardSpacing = hGrp->GetInt("FileCardSpacing", 15);  // NOLINT
    auto showExamples = hGrp->GetBool("ShowExamples", true);

    std::string customFolder(hGrp->GetASCII("CustomFolder", ""));
    bool showCustomFolder = !customFolder.empty();

    // =========================================================================
    // First Start page (unchanged)
    // =========================================================================
    auto firstStartScrollArea = gsl::owner<QScrollArea*>(new QScrollArea());
    firstStartScrollArea->setFrameShape(QFrame::NoFrame);
    firstStartScrollArea->setStyleSheet(QStringLiteral("QScrollArea { border: none; background: transparent; }"));
    auto firstStartScrollWidget = gsl::owner<QWidget*>(new QWidget(firstStartScrollArea));
    firstStartScrollArea->setWidget(firstStartScrollWidget);
    firstStartScrollArea->setWidgetResizable(true);

    auto firstStartRegion = gsl::owner<QHBoxLayout*>(new QHBoxLayout(firstStartScrollWidget));
    firstStartRegion->setAlignment(Qt::AlignCenter);
    auto firstStartWidget = gsl::owner<FirstStartWidget*>(new FirstStartWidget(this));
    connect(firstStartWidget, &FirstStartWidget::dismissed, this, &StartView::firstStartWidgetDismissed);
    firstStartRegion->addWidget(firstStartWidget);
    _contents->addWidget(firstStartScrollArea);

    // =========================================================================
    // Documents page — redesigned two-column layout
    // =========================================================================
    auto documentsWidget = gsl::owner<QWidget*>(new QWidget());
    _contents->addWidget(documentsWidget);
    auto documentsMainLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout());
    documentsMainLayout->setContentsMargins(0, 0, 0, 0);
    documentsWidget->setLayout(documentsMainLayout);

    // --- Header: FreeCAD icon + version ---
    auto headerLayout = gsl::owner<QHBoxLayout*>(new QHBoxLayout());
    headerLayout->setContentsMargins(8, 8, 8, 0);
    auto logoLabel = gsl::owner<QLabel*>(new QLabel());
    logoLabel->setPixmap(QIcon(QLatin1String(":/icons/freecad.svg")).pixmap(32, 32));
    _headerLabel = gsl::owner<QLabel*>(new QLabel());
    _headerLabel->setObjectName(QStringLiteral("startPageHeader"));
    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(_headerLabel);
    headerLayout->addStretch();
    documentsMainLayout->addLayout(headerLayout);

    // --- Body: two-column layout ---
    _bodyLayout = gsl::owner<QHBoxLayout*>(new QHBoxLayout());
    _bodyLayout->setSpacing(0);
    documentsMainLayout->addLayout(_bodyLayout, 1);

    // ---- Left column (scrollable main content) ----
    auto leftScrollArea = gsl::owner<QScrollArea*>(new QScrollArea());
    leftScrollArea->setFrameShape(QFrame::NoFrame);
    leftScrollArea->setStyleSheet(QStringLiteral("QScrollArea { border: none; background: transparent; }"));
    leftScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    leftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    leftScrollArea->setWidgetResizable(true);
    auto leftScrollWidget = gsl::owner<QWidget*>(new QWidget(leftScrollArea));
    leftScrollArea->setWidget(leftScrollWidget);
    _leftContentLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(leftScrollWidget));
    _leftContentLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinAndMaxSize);

    // RECENT FILES section
    _recentFilesLabel = makeSectionLabel(QString());
    _leftContentLayout->addWidget(_recentFilesLabel);

    _recentFilesListWidget = gsl::owner<FileCardView*>(new FileCardView(_contents));
    _recentFilesListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_recentFilesListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
    connect(_recentFilesListWidget, &QWidget::customContextMenuRequested, this, &StartView::fileCardContextMenu);
    _leftContentLayout->addWidget(_recentFilesListWidget);

    // CREATE NEW section
    _createNewLabel = makeSectionLabel(QString());
    _leftContentLayout->addWidget(_createNewLabel);

    auto createNewRow = gsl::owner<QWidget*>(new QWidget);
    auto flowLayout = gsl::owner<FlowLayout*>(new FlowLayout);
    flowLayout->setContentsMargins({});
    createNewRow->setObjectName(QStringLiteral("CreateNewRow"));
    createNewRow->setLayout(flowLayout);
    _leftContentLayout->addWidget(createNewRow);
    configureNewFileButtons(flowLayout, true);

    // CUSTOM FOLDER section (preference-gated)
    FileCardView* customFolderListWidget {};
    if (showCustomFolder) {
        _customFolderLabel = makeSectionLabel(QString());
        _leftContentLayout->addWidget(_customFolderLabel);
        customFolderListWidget = gsl::owner<FileCardView*>(new FileCardView(_contents));
        connect(customFolderListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
        _leftContentLayout->addWidget(customFolderListWidget);
    }

    // GETTING STARTED card
    auto gettingStartedCard = gsl::owner<GettingStartedCard*>(new GettingStartedCard());
    _leftContentLayout->addWidget(gettingStartedCard);

    _leftContentLayout->setSpacing(static_cast<int>(cardSpacing));
    _leftContentLayout->addStretch();

    _bodyLayout->addWidget(leftScrollArea, 7);

    // ---- Right column (sidebar in scroll area) ----
    _rightScrollArea = gsl::owner<QScrollArea*>(new QScrollArea());
    auto rightScrollArea = _rightScrollArea;
    rightScrollArea->setFrameShape(QFrame::NoFrame);
    rightScrollArea->setStyleSheet(QStringLiteral("QScrollArea { border: none; background: transparent; }"));
    rightScrollArea->setWidgetResizable(true);
    rightScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightScrollArea->setMinimumWidth(250);
    rightScrollArea->setMaximumWidth(320);

    _rightPanel = gsl::owner<QWidget*>(new QWidget());
    rightScrollArea->setWidget(_rightPanel);
    auto rightLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_rightPanel));
    rightLayout->setContentsMargins(8, 0, 8, 0);

    // EXAMPLES section in sidebar
    _examplesContainer = gsl::owner<QWidget*>(new QWidget());
    auto examplesLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_examplesContainer));
    examplesLayout->setContentsMargins(0, 0, 0, 0);

    _examplesSectionLabel = makeSectionLabel(QString());
    examplesLayout->addWidget(_examplesSectionLabel);

    QListView* examplesListWidget = nullptr;
    if (showExamples) {
        examplesListWidget = gsl::owner<QListView*>(new QListView());
        examplesListWidget->setObjectName(QStringLiteral("examplesListView"));
        examplesListWidget->setViewMode(QListView::ListMode);
        examplesListWidget->setFlow(QListView::TopToBottom);
        examplesListWidget->setFrameShape(QFrame::NoFrame);
        examplesListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        examplesListWidget->setMouseTracking(true);
        examplesListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        auto examplesDelegate = gsl::owner<ExamplesListDelegate*>(new ExamplesListDelegate(examplesListWidget));
        examplesListWidget->setItemDelegate(examplesDelegate);
        connect(examplesListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
        examplesLayout->addWidget(examplesListWidget);
    }

    _browseExamplesButton = gsl::owner<QPushButton*>(new QPushButton());
    _browseExamplesButton->setObjectName(QStringLiteral("learnLink"));
    _browseExamplesButton->setFlat(true);
    _browseExamplesButton->setIcon(QIcon(QLatin1String(":/icons/list-add.svg")));
    connect(_browseExamplesButton, &QPushButton::clicked, this, &StartView::openExistingFile);
    examplesLayout->addWidget(_browseExamplesButton);

    rightLayout->addWidget(_examplesContainer);

    // LEARN section in sidebar
    _learnContainer = gsl::owner<QWidget*>(new QWidget());
    auto learnContainerLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_learnContainer));
    learnContainerLayout->setContentsMargins(0, 0, 0, 0);

    _learnSectionLabel = makeSectionLabel(QString());
    learnContainerLayout->addWidget(_learnSectionLabel);

    auto learnLinks = gsl::owner<LearnLinksWidget*>(new LearnLinksWidget());
    learnContainerLayout->addWidget(learnLinks);

    rightLayout->addWidget(_learnContainer);

    _bodyLayout->addWidget(rightScrollArea, 3);

    // --- Footer ---
    auto footerLayout = gsl::owner<QHBoxLayout*>(new QHBoxLayout());
    documentsMainLayout->addLayout(footerLayout);

    _openFirstStart = gsl::owner<QPushButton*>(new QPushButton());
    _openFirstStart->setIcon(QIcon(QLatin1String(":/icons/preferences-general.svg")));
    connect(_openFirstStart, &QPushButton::clicked, this, &StartView::openFirstStartClicked);

    _showOnStartupCheckBox = gsl::owner<QCheckBox*>(new QCheckBox());
    bool showOnStartup = hGrp->GetBool("ShowOnStartup", true);
    _showOnStartupCheckBox->setCheckState(
        showOnStartup ? Qt::CheckState::Unchecked : Qt::CheckState::Checked
    );
    connect(_showOnStartupCheckBox, &QCheckBox::toggled, this, &StartView::showOnStartupChanged);

    footerLayout->addWidget(_openFirstStart);
    footerLayout->addStretch();
    footerLayout->addWidget(_showOnStartupCheckBox);

    setCentralWidget(_contents);

    // =========================================================================
    // Configure models and page selection
    // =========================================================================
    auto firstStart = hGrp->GetBool("FirstStart2024", true);
    _contents->setCurrentWidget(firstStart ? firstStartScrollArea : documentsWidget);

    // Set up proxy model for recent files (appends "Open file..." card)
    _openFileProxyModel.setSourceModel(&_recentFilesModel);
    configureRecentFilesListWidget(_recentFilesListWidget, _recentFilesLabel);

    if (customFolderListWidget) {
        configureCustomFolderListWidget(customFolderListWidget);
    }
    if (examplesListWidget) {
        configureExamplesListWidget(examplesListWidget);
    }

    QTimer::singleShot(2000, [this]() {
        auto recentFiles = Gui::getMainWindow()->findChild<Gui::RecentFilesAction*>();
        if (recentFiles != nullptr) {
            connect(recentFiles, &Gui::RecentFilesAction::recentFilesListModified, this, [this]() {
                configureRecentFilesListWidget(_recentFilesListWidget, _recentFilesLabel);
            });
        }
    });

    isInitialized = true;
    retranslateUi();
}

void StartView::configureNewFileButtons(QLayout* layout, bool compact) const
{
    auto partDesign = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("Parametric Body"),
         tr("Creates a body with the Part Design workbench"),
         QLatin1String(":/icons/PartDesignWorkbench.svg")},
        compact
    ));
    auto assembly = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("Assembly"),
         tr("Creates an assembly project"),
         QLatin1String(":/icons/AssemblyWorkbench.svg")},
        compact
    ));
    auto arch = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("BIM/Architecture"),
         tr("Creates an architectural project"),
         QLatin1String(":/icons/BIMWorkbench.svg")},
        compact
    ));
    auto draft = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("2D Draft"), tr("Creates a 2D Draft document"), QLatin1String(":/icons/DraftWorkbench.svg")},
        compact
    ));
    auto newEmptyFile = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("Empty File"),
         tr("Creates a new empty FreeCAD file"),
         QLatin1String(":/icons/document-new.svg")},
        compact
    ));

    layout->addWidget(partDesign);
    layout->addWidget(assembly);
    layout->addWidget(arch);
    layout->addWidget(draft);
    layout->addWidget(newEmptyFile);

    connect(newEmptyFile, &QPushButton::clicked, this, &StartView::newEmptyFile);
    connect(partDesign, &QPushButton::clicked, this, &StartView::newPartDesignFile);
    connect(assembly, &QPushButton::clicked, this, &StartView::newAssemblyFile);
    connect(draft, &QPushButton::clicked, this, &StartView::newDraftFile);
    connect(arch, &QPushButton::clicked, this, &StartView::newArchFile);
}

void StartView::configureFileCardWidget(QListView* fileCardWidget)
{
    auto delegate = gsl::owner<FileCardDelegate*>(new FileCardDelegate(fileCardWidget));
    fileCardWidget->setItemDelegate(delegate);
    fileCardWidget->setMinimumWidth(fileCardWidget->parentWidget()->width());
}


void StartView::configureRecentFilesListWidget(QListView* recentFilesListWidget, QLabel* recentFilesLabel)
{
    _recentFilesModel.loadRecentFiles();
    recentFilesListWidget->setModel(&_openFileProxyModel);

    // Configure delegate with timestamp and pinned features
    auto delegate = dynamic_cast<FileCardDelegate*>(recentFilesListWidget->itemDelegate());
    if (!delegate) {
        delegate = gsl::owner<FileCardDelegate*>(new FileCardDelegate(recentFilesListWidget));
        recentFilesListWidget->setItemDelegate(delegate);
    }
    delegate->setShowTimestamp(true);
    delegate->setShowPinnedIndicator(true);

    recentFilesListWidget->setMinimumWidth(recentFilesListWidget->parentWidget()->width());

    // Always show — the proxy model adds the "Open file..." card even when empty
    recentFilesListWidget->show();
    recentFilesLabel->show();
}


void StartView::configureExamplesListWidget(QListView* examplesListWidget)
{
    _examplesModel.loadExamples();
    examplesListWidget->setModel(&_examplesModel);
}


void StartView::configureCustomFolderListWidget(QListView* customFolderListWidget)
{
    _customFolderModel.loadCustomFolder();
    customFolderListWidget->setModel(&_customFolderModel);
    configureFileCardWidget(customFolderListWidget);
}


void StartView::newEmptyFile()
{
    Gui::Application::Instance->commandManager().runCommandByName("Std_New");
    postStart(PostStartBehavior::switchWorkbench);
}

void StartView::newPartDesignFile()
{
    Gui::Application::Instance->commandManager().runCommandByName("Std_New");
    Gui::Application::Instance->activateWorkbench("PartDesignWorkbench");
    Gui::Application::Instance->commandManager().runCommandByName("PartDesign_Body");
    postStart(PostStartBehavior::doNotSwitchWorkbench);
}

void StartView::openExistingFile()
{
    auto originalDocument = Gui::Application::Instance->activeDocument();
    Gui::Application::Instance->commandManager().runCommandByName("Std_Open");
    Gui::Application::checkForRecomputes();
    if (Gui::Application::Instance->activeDocument() != originalDocument) {
        postStart(PostStartBehavior::switchWorkbench);
    }
}

void StartView::newAssemblyFile()
{
    Gui::Application::Instance->commandManager().runCommandByName("Std_New");
    Gui::Application::Instance->activateWorkbench("AssemblyWorkbench");
    Gui::Application::Instance->commandManager().runCommandByName("Assembly_CreateAssembly");
    Gui::Application::Instance->commandManager().runCommandByName("Std_Refresh");
    postStart(PostStartBehavior::doNotSwitchWorkbench);
}

void StartView::newDraftFile()
{
    Gui::Application::Instance->commandManager().runCommandByName("Std_New");
    Gui::Application::Instance->activateWorkbench("DraftWorkbench");
    Gui::Application::Instance->commandManager().runCommandByName("Std_ViewTop");
    postStart(PostStartBehavior::doNotSwitchWorkbench);
}

void StartView::newArchFile()
{
    Gui::Application::Instance->commandManager().runCommandByName("Std_New");
    try {
        Gui::Application::Instance->activateWorkbench("BIMWorkbench");
    }
    catch (...) {
        Gui::Application::Instance->activateWorkbench("ArchWorkbench");
    }

    Gui::Command::doCommand(
        Gui::Command::Gui,
        "Gui.activeDocument().activeView().viewDefaultOrientation(None, 10000.0)"
    );
    postStart(PostStartBehavior::doNotSwitchWorkbench);
}

bool StartView::onHasMsg(const char* pMsg) const
{
    if (strcmp("AllowsOverlayOnHover", pMsg) == 0) {
        return false;
    }

    return MDIView::onHasMsg(pMsg);
}

void StartView::postStart(PostStartBehavior behavior)
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );

    if (behavior == PostStartBehavior::switchWorkbench) {
        auto wb = hGrp->GetASCII("AutoloadModule", "");
        if (wb == "$LastModule") {
            wb = App::GetApplication()
                     .GetParameterGroupByPath("User parameter:BaseApp/Preferences/General")
                     ->GetASCII("LastModule", "");
        }
        if (!wb.empty()) {
            Gui::Application::Instance->activateWorkbench(wb.c_str());
        }
    }
    if (hGrp->GetBool("closeStart", false)) {
        for (QWidget* w = this; w != nullptr; w = w->parentWidget()) {
            if (auto mdiSub = qobject_cast<QMdiSubWindow*>(w)) {
                mdiSub->close();
                return;
            }
        }
    }
}


void StartView::fileCardSelected(const QModelIndex& index)
{
    try {
        auto filename = index.data(static_cast<int>(Start::DisplayedFilesModelRoles::path)).toString();

        // Handle the "Open file..." sentinel card
        if (filename == OpenFileProxyModel::OpenFileSentinel) {
            openExistingFile();
            return;
        }

        Gui::ModuleIO::verifyAndOpenFile(filename);
    }
    catch (Base::PyException& e) {
        Base::Console().error(e.getMessage().c_str());
    }
    catch (Base::Exception& e) {
        Base::Console().error(e.getMessage().c_str());
    }
    catch (...) {
        Base::Console().error("An unknown error occurred");
    }
}

void StartView::fileCardContextMenu(const QPoint& pos)
{
    auto index = _recentFilesListWidget->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    auto path = index.data(static_cast<int>(Start::DisplayedFilesModelRoles::path)).toString();
    if (path == OpenFileProxyModel::OpenFileSentinel) {
        return;
    }

    QMenu menu(this);
    bool pinned = _recentFilesModel.isPinned(path);
    auto action = menu.addAction(pinned ? tr("Unpin") : tr("Pin"));
    if (menu.exec(_recentFilesListWidget->viewport()->mapToGlobal(pos)) == action) {
        _recentFilesModel.togglePinned(path);
    }
}

void StartView::showOnStartupChanged(bool checked)
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    hGrp->SetBool("ShowOnStartup", !checked);
}

void StartView::openFirstStartClicked()
{
    _contents->setCurrentIndex(0);
}

void StartView::firstStartWidgetDismissed()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    hGrp->SetBool("FirstStart2024", false);
    _contents->setCurrentIndex(1);
}

void StartView::changeEvent(QEvent* event)
{
    if (!isInitialized) {
        return;
    }

    _openFirstStart->setEnabled(true);
    Gui::Document* doc = Gui::Application::Instance->activeDocument();
    if (doc) {
        if (auto view = dynamic_cast<Gui::View3DInventor*>(doc->getActiveView())) {
            Gui::View3DInventorViewer* viewer = view->getViewer();
            if (viewer->isEditing()) {
                _openFirstStart->setEnabled(false);
            }
        }
    }

    if (event->type() == QEvent::LanguageChange) {
        this->retranslateUi();
    }

    Gui::MDIView::changeEvent(event);
}

void StartView::showEvent(QShowEvent* event)
{
    if (auto mainWindow = Gui::getMainWindow()) {
        if (auto mdiArea = mainWindow->findChild<QMdiArea*>()) {
            connect(
                mdiArea,
                &QMdiArea::subWindowActivated,
                this,
                &StartView::onMdiSubWindowActivated,
                Qt::UniqueConnection
            );
        }
    }
    Gui::MDIView::showEvent(event);
}

void StartView::resizeEvent(QResizeEvent* event)
{
    Gui::MDIView::resizeEvent(event);
    updateLayout();
}

void StartView::updateLayout()
{
    if (!isInitialized || !_rightPanel || !_leftContentLayout) {
        return;
    }

    int width = this->width();
    bool shouldBeTwoColumn = width >= responsiveBreakpoint;

    if (shouldBeTwoColumn == _isTwoColumn) {
        return;
    }

    if (shouldBeTwoColumn) {
        // Restore two-column: move containers back to right panel
        _leftContentLayout->removeWidget(_examplesContainer);
        _leftContentLayout->removeWidget(_learnContainer);
        auto rightLayout = _rightPanel->layout();
        if (rightLayout) {
            rightLayout->addWidget(_examplesContainer);
            rightLayout->addWidget(_learnContainer);
        }
        if (_rightScrollArea) {
            _rightScrollArea->show();
        }
    }
    else {
        // Collapse to single column: move sidebar widgets into left content
        if (_rightScrollArea) {
            _rightScrollArea->hide();
        }
        // Insert before the stretch at the end
        int insertIdx = _leftContentLayout->count() - 1;  // before stretch
        _leftContentLayout->insertWidget(insertIdx, _examplesContainer);
        _leftContentLayout->insertWidget(insertIdx + 1, _learnContainer);
    }

    _isTwoColumn = shouldBeTwoColumn;
}

void StartView::onMdiSubWindowActivated(QMdiSubWindow* subWindow)
{
    bool isOurWindow = subWindow && subWindow->isAncestorOf(this);
    setListViewUpdatesEnabled(isOurWindow);
}

void StartView::setListViewUpdatesEnabled(bool enabled)
{
    QList<QListView*> listViews = findChildren<QListView*>();
    for (QListView* listView : listViews) {
        listView->setUpdatesEnabled(enabled);
        if (listView->viewport()) {
            listView->viewport()->setUpdatesEnabled(enabled);
        }
    }
}

void StartView::recentFileAdded(const QString& filename)
{
    _recentFilesModel.recentFileAdded(filename);
}

void StartView::retranslateUi()
{
    QString title = QCoreApplication::translate("Workbench", "Start");
    setWindowTitle(title);

    // Header
    auto versionStr = QString::fromUtf8(App::Application::Config()["ExeVersion"].c_str());
    auto appName = QString::fromUtf8(App::Application::Config()["ExeName"].c_str());
    _headerLabel->setText(QStringLiteral("<h2>%1 %2</h2>").arg(appName, versionStr));

    // Section labels
    _recentFilesLabel->setText(tr("RECENT FILES"));
    _createNewLabel->setText(tr("CREATE NEW"));
    if (_examplesSectionLabel) {
        _examplesSectionLabel->setText(tr("EXAMPLES"));
    }
    if (_learnSectionLabel) {
        _learnSectionLabel->setText(tr("LEARN"));
    }

    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    std::string customFolder(hGrp->GetASCII("CustomFolder", ""));
    if (!customFolder.empty() && _customFolderLabel) {
        if (hGrp->GetBool("ShortCustomFolder", true)) {
            _customFolderLabel->setToolTip(QString::fromUtf8(customFolder.c_str()));
            customFolder = customFolder.substr(customFolder.find_last_of("/\\") + 1);
        }
        _customFolderLabel->setText(QString::fromUtf8(customFolder.c_str()));
    }

    if (_browseExamplesButton) {
        _browseExamplesButton->setText(tr("Browse all examples..."));
    }

    _openFirstStart->setText(tr("Open First Start Setup"));
    _showOnStartupCheckBox->setText(tr("Do not show this Start page again"));
}
