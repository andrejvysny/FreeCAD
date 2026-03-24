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
#include <QCursor>
#include <QPalette>
#include <QPixmap>
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

static constexpr int kPageMargin = 24;
static constexpr int kTopMargin = 32;
static constexpr int kBottomMargin = 16;
static constexpr int kSectionSpacing = 16;

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
    auto cardSpacing = hGrp->GetInt("FileCardSpacing", 16);  // NOLINT
    auto showExamples = hGrp->GetBool("ShowExamples", true);

    std::string customFolder(hGrp->GetASCII("CustomFolder", ""));
    bool showCustomFolder = !customFolder.empty();

    // First Start wizard — created without parent, shown via MainWindow overlay
    _firstStartWidget = new FirstStartWidget(nullptr);
    connect(_firstStartWidget, &FirstStartWidget::dismissed, this, &StartView::firstStartWidgetDismissed);

    // =========================================================================
    // Documents page — two-column layout (full height sidebar)
    // =========================================================================
    auto documentsWidget = gsl::owner<QWidget*>(new QWidget());
    _contents->addWidget(documentsWidget);

    // Read max content size preferences for centering on large screens
    _maxContentWidth = static_cast<int>(hGrp->GetInt("MaxContentWidth", defaultMaxContentWidth));
    _maxContentHeight = static_cast<int>(hGrp->GetInt("MaxContentHeight", defaultMaxContentHeight));

    // Top-level: two-column HBox — margins set dynamically for centering
    _bodyLayout = gsl::owner<QHBoxLayout*>(new QHBoxLayout());
    _bodyLayout->setSpacing(0);
    _bodyLayout->setContentsMargins(0, 0, 0, 0);
    documentsWidget->setLayout(_bodyLayout);

    // ---- Left column (scrollable main content) ----
    _leftScrollArea = gsl::owner<QScrollArea*>(new QScrollArea());
    _leftScrollArea->setObjectName(QStringLiteral("startLeftScrollArea"));
    _leftScrollArea->setFrameShape(QFrame::NoFrame);
    _leftScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    _leftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    _leftScrollArea->setWidgetResizable(true);
    _leftContentWidget = gsl::owner<QWidget*>(new QWidget(_leftScrollArea));
    _leftScrollArea->setWidget(_leftContentWidget);
    _leftContentLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_leftContentWidget));
    _leftContentLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinAndMaxSize);
    _leftContentLayout->setContentsMargins(kPageMargin, kTopMargin, kPageMargin, kBottomMargin);

    // Wordmark header (first row in left column)
    _wordmarkLabel = gsl::owner<QLabel*>(new QLabel());
    _wordmarkLabel->setFixedHeight(72);
    _wordmarkLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    updateWordmark();
    _leftContentLayout->addWidget(_wordmarkLabel);
    _leftContentLayout->addSpacing(kSectionSpacing);

    // Version label (shown in footer, set by retranslateUi)
    _headerLabel = gsl::owner<QLabel*>(new QLabel());
    _headerLabel->setObjectName(QStringLiteral("startVersionLabel"));
    // Tagline label (unused, kept for retranslateUi compat)
    _headerTaglineLabel = gsl::owner<QLabel*>(new QLabel());
    _headerTaglineLabel->hide();


    // RECENT FILES section (flat, no card)
    _recentFilesLabel = makeSectionLabel(QString());
    _leftContentLayout->addWidget(_recentFilesLabel);
    _recentFilesListWidget = gsl::owner<FileCardView*>(new FileCardView(_contents));
    _recentFilesListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    _recentFilesListWidget->setCursor(Qt::PointingHandCursor);
    connect(_recentFilesListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
    connect(_recentFilesListWidget, &QWidget::customContextMenuRequested, this, &StartView::fileCardContextMenu);
    _leftContentLayout->addWidget(_recentFilesListWidget);

    // CREATE NEW section (flat, no card)
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

    // GETTING STARTED card (dismissible)
    auto showGettingStarted = hGrp->GetBool("ShowGettingStarted", true);
    _gettingStartedLabel = makeSectionLabel(QString());
    _gettingStartedCard = gsl::owner<GettingStartedCard*>(new GettingStartedCard());
    if (showGettingStarted) {
        _leftContentLayout->addWidget(_gettingStartedLabel);
        _leftContentLayout->addWidget(_gettingStartedCard);
    }
    else {
        _gettingStartedLabel->hide();
        _gettingStartedCard->hide();
    }
    connect(_gettingStartedCard, &GettingStartedCard::dismissed, this, [this]() {
        _gettingStartedLabel->hide();
        _gettingStartedCard->hide();
        auto grp = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Mod/Start");
        grp->SetBool("ShowGettingStarted", false);
    });

    _leftContentLayout->setSpacing(static_cast<int>(cardSpacing));
    _leftContentLayout->addStretch(1);

    _bodyLayout->addWidget(_leftScrollArea, 7);

    // ---- Vertical sidebar divider ----
    _sidebarDivider = gsl::owner<QFrame*>(new QFrame());
    _sidebarDivider->setObjectName(QStringLiteral("startSidebarDivider"));
    _sidebarDivider->setFrameShape(QFrame::VLine);
    _sidebarDivider->setFixedWidth(1);
    _bodyLayout->addWidget(_sidebarDivider);

    // ---- Right column (full-height sidebar) ----
    _rightScrollArea = gsl::owner<QScrollArea*>(new QScrollArea());
    _rightScrollArea->setObjectName(QStringLiteral("startRightScrollArea"));
    auto rightScrollArea = _rightScrollArea;
    rightScrollArea->setFrameShape(QFrame::NoFrame);
    rightScrollArea->setWidgetResizable(true);
    rightScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightScrollArea->setMinimumWidth(250);
    rightScrollArea->setMaximumWidth(320);

    _rightPanel = gsl::owner<QWidget*>(new QWidget());
    rightScrollArea->setWidget(_rightPanel);
    auto rightLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_rightPanel));
    rightLayout->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
    rightLayout->setContentsMargins(kPageMargin, kPageMargin, kPageMargin, kPageMargin);

    // EXAMPLES section in sidebar (flat, no card)
    _examplesContainer = gsl::owner<QFrame*>(new QFrame());
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
        examplesListWidget->setCursor(Qt::PointingHandCursor);
        examplesListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        auto examplesDelegate = gsl::owner<ExamplesListDelegate*>(new ExamplesListDelegate(examplesListWidget));
        examplesListWidget->setItemDelegate(examplesDelegate);
        connect(examplesListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
        examplesLayout->addWidget(examplesListWidget, 1);
    }

    _browseExamplesButton = gsl::owner<QPushButton*>(new QPushButton());
    _browseExamplesButton->setObjectName(QStringLiteral("learnLink"));
    _browseExamplesButton->setFlat(true);
    _browseExamplesButton->setCursor(Qt::PointingHandCursor);
    _browseExamplesButton->setIcon(QIcon(QLatin1String(":/icons/document-open.svg")));
    connect(_browseExamplesButton, &QPushButton::clicked, this, &StartView::openExistingFile);
    examplesLayout->addWidget(_browseExamplesButton);

    rightLayout->addWidget(_examplesContainer, 1);  // stretch to fill

    // Stretch pushes Learn to bottom
    rightLayout->addStretch();

    // LEARN section at bottom of sidebar
    _learnContainer = gsl::owner<QFrame*>(new QFrame());
    auto learnContainerLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_learnContainer));
    learnContainerLayout->setContentsMargins(0, 0, 0, 0);

    _learnSectionLabel = makeSectionLabel(QString());
    learnContainerLayout->addWidget(_learnSectionLabel);

    auto learnLinks = gsl::owner<LearnLinksWidget*>(new LearnLinksWidget());
    learnContainerLayout->addWidget(learnLinks);

    rightLayout->addWidget(_learnContainer);

    _bodyLayout->addWidget(rightScrollArea, 3);

    // --- Footer (inside left column, after stretch) ---
    auto footerLayout = gsl::owner<QHBoxLayout*>(new QHBoxLayout());
    _leftContentLayout->addLayout(footerLayout);

    _openFirstStart = gsl::owner<QPushButton*>(new QPushButton());
    _openFirstStart->setCursor(Qt::PointingHandCursor);
    _openFirstStart->setFlat(true);
    _openFirstStart->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: 1px solid rgba(128,128,128,0.35);"
        "border-radius: 6px; padding: 4px 12px; color: rgba(0,0,0,0.55); font-size: 11px; }"
        "QPushButton:hover { border-color: rgba(65,143,222,0.6); color: #418FDE; }"
    ));
    connect(_openFirstStart, &QPushButton::clicked, this, &StartView::openFirstStartClicked);

    _showOnStartupCheckBox = gsl::owner<QCheckBox*>(new QCheckBox());
    bool showOnStartup = hGrp->GetBool("ShowOnStartup", true);
    _showOnStartupCheckBox->setCheckState(
        showOnStartup ? Qt::CheckState::Unchecked : Qt::CheckState::Checked
    );
    connect(_showOnStartupCheckBox, &QCheckBox::toggled, this, &StartView::showOnStartupChanged);

    footerLayout->addWidget(_openFirstStart);
    footerLayout->addStretch();
    footerLayout->addWidget(_headerLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(_showOnStartupCheckBox);

    setCentralWidget(_contents);

    // =========================================================================
    // Configure models and page selection
    // =========================================================================
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

StartView::~StartView()
{
    if (auto mainWindow = Gui::getMainWindow()) {
        if (mainWindow->isFullWindowOverlayVisible()) {
            mainWindow->hideFullWindowOverlay();
        }
    }
    delete _firstStartWidget;
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
    showWizardOverlay();
}

void StartView::firstStartWidgetDismissed()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    hGrp->SetBool("FirstStart2025", false);
    if (auto mainWindow = Gui::getMainWindow()) {
        disconnect(mainWindow, &Gui::MainWindow::fullWindowOverlayHidden,
                   this, &StartView::firstStartWidgetDismissed);
        mainWindow->hideFullWindowOverlay();
    }
}

void StartView::showWizardOverlay()
{
    if (auto mainWindow = Gui::getMainWindow()) {
        if (_firstStartWidget) {
            _firstStartWidget->resetToFirstStep();
            mainWindow->showFullWindowOverlay(_firstStartWidget);
            // Dismiss wizard if overlay is hidden externally (e.g. Escape key)
            connect(mainWindow, &Gui::MainWindow::fullWindowOverlayHidden,
                    this, &StartView::firstStartWidgetDismissed,
                    Qt::UniqueConnection);
        }
    }
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

    if (event->type() == QEvent::PaletteChange
        || event->type() == QEvent::StyleChange) {
        updateWordmark();
        if (_recentFilesListWidget && _recentFilesListWidget->viewport()) {
            _recentFilesListWidget->viewport()->update();
        }
        if (_rightPanel) {
            _rightPanel->updateGeometry();
        }
        if (_rightScrollArea) {
            _rightScrollArea->updateGeometry();
        }
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

    // Show wizard overlay on first run (deferred so MainWindow geometry is ready)
    if (!_firstStartShown) {
        auto hGrp = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Mod/Start"
        );
        if (hGrp->GetBool("FirstStart2025", true)) {
            _firstStartShown = true;
            QTimer::singleShot(0, this, &StartView::showWizardOverlay);
        }
    }

    updateWordmark();  // re-check wordmark after stylesheet is applied
    Gui::MDIView::showEvent(event);
}

void StartView::resizeEvent(QResizeEvent* event)
{
    Gui::MDIView::resizeEvent(event);
    updateLayout();
    updateContentCentering();
}


void StartView::updateContentCentering()
{
    if (!_bodyLayout || !_contents) {
        return;
    }

    int availW = _contents->width();
    int availH = _contents->height();

    int marginH = (_maxContentWidth > 0) ? qMax(0, (availW - _maxContentWidth) / 2) : 0;
    int marginV = (_maxContentHeight > 0) ? qMax(0, (availH - _maxContentHeight) / 2) : 0;

    _bodyLayout->setContentsMargins(marginH, marginV, marginH, marginV);
}

void StartView::updateWordmark()
{
    if (!_wordmarkLabel) {
        return;
    }

    bool isDark = false;

    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );
    auto theme = QString::fromStdString(hGrp->GetASCII("Theme", "Classic"));

    if (theme.contains(QLatin1String("Dark"), Qt::CaseInsensitive)) {
        isDark = true;
    }
    else if (theme.contains(QLatin1String("Light"), Qt::CaseInsensitive)) {
        isDark = false;
    }
    else {
        QColor bg = _wordmarkLabel->palette().color(QPalette::Window);
        double luminance = 0.299 * bg.redF() + 0.587 * bg.greenF() + 0.114 * bg.blueF();
        isDark = luminance < 0.5;
    }

    QString wordmarkPath = isDark
        ? QStringLiteral(":/branding/FreeCAD-wordmark-light.svg")
        : QStringLiteral(":/branding/FreeCAD-wordmark.svg");
    QPixmap wordmark(wordmarkPath);
    if (!wordmark.isNull()) {
        _wordmarkLabel->setPixmap(wordmark.scaledToHeight(56, Qt::SmoothTransformation));
    }
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
        if (_sidebarDivider) {
            _sidebarDivider->show();
        }
    }
    else {
        // Collapse to single column: move sidebar widgets into left content
        if (_rightScrollArea) {
            _rightScrollArea->hide();
        }
        if (_sidebarDivider) {
            _sidebarDivider->hide();
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
    _headerLabel->setText(QStringLiteral("v%1").arg(versionStr));
    if (_headerTaglineLabel) {
        _headerTaglineLabel->setText(tr("Open Source Parametric 3D Modeler"));
    }

    // Section labels
    _recentFilesLabel->setText(tr("Recent Files"));
    _createNewLabel->setText(tr("Create New"));
    if (_examplesSectionLabel) {
        _examplesSectionLabel->setText(tr("Examples"));
    }
    if (_learnSectionLabel) {
        _learnSectionLabel->setText(tr("Learn"));
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

    if (_gettingStartedLabel) {
        _gettingStartedLabel->setText(tr("Getting Started"));
    }
    if (_browseExamplesButton) {
        _browseExamplesButton->setText(tr("Browse all examples..."));
    }

    _openFirstStart->setText(tr("Setup Wizard"));
    _showOnStartupCheckBox->setText(tr("Do not show this Start page again"));
}
