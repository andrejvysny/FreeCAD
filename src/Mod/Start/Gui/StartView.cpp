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
#include <QMessageBox>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QWidget>
#include <QShowEvent>

#include "StartView.h"
#include "ExamplesListDelegate.h"
#include "FileCardDelegate.h"
#include "FileCardView.h"
#include "FlowLayout.h"
#include "LearnLinksWidget.h"
#include "NewFileButton.h"
#include "StartupWizardController.h"
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

static constexpr int pageMargin = 24;
static constexpr int topMargin = 32;
static constexpr int bottomMargin = 16;
static constexpr int sectionSpacing = 16;

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
{
    setObjectName(QLatin1String("StartView"));
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    auto showExamples = hGrp->GetBool("ShowExamples", true);

    std::string customFolder(hGrp->GetASCII("CustomFolder", ""));
    bool showCustomFolder = !customFolder.empty();

    auto documentsWidget = gsl::owner<QWidget*>(new QWidget());

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
    auto leftContentWidget = gsl::owner<QWidget*>(new QWidget(_leftScrollArea));
    _leftScrollArea->setWidget(leftContentWidget);
    _leftContentLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(leftContentWidget));
    _leftContentLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinAndMaxSize);
    _leftContentLayout->setContentsMargins(pageMargin, topMargin, pageMargin, bottomMargin);

    // Wordmark header
    _wordmarkLabel = gsl::owner<QLabel*>(new QLabel());
    _wordmarkLabel->setFixedHeight(72);
    _wordmarkLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    updateWordmark();
    _leftContentLayout->addWidget(_wordmarkLabel);
    _leftContentLayout->addSpacing(sectionSpacing);

    // Version label (shown in footer)
    _headerLabel = gsl::owner<QLabel*>(new QLabel());
    _headerLabel->setObjectName(QStringLiteral("startVersionLabel"));

    // CREATE NEW section
    _createNewLabel = makeSectionLabel(QString());
    _leftContentLayout->addWidget(_createNewLabel);
    auto createNewRow = gsl::owner<QWidget*>(new QWidget);
    auto flowLayout = gsl::owner<FlowLayout*>(new FlowLayout);
    flowLayout->setContentsMargins({});
    createNewRow->setObjectName(QStringLiteral("CreateNewRow"));
    createNewRow->setLayout(flowLayout);
    _leftContentLayout->addWidget(createNewRow);
    configureNewFileButtons(flowLayout);

    // RECENT FILES section
    _recentFilesLabel = makeSectionLabel(QString());
    _leftContentLayout->addWidget(_recentFilesLabel);
    auto recentFilesListWidget = gsl::owner<FileCardView*>(new FileCardView(documentsWidget));
    recentFilesListWidget->setProperty("startFileCardList", true);
    connect(recentFilesListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
    _leftContentLayout->addWidget(recentFilesListWidget);

    // CUSTOM FOLDER section (optional)
    FileCardView* customFolderListWidget {};
    if (showCustomFolder) {
        _customFolderLabel = makeSectionLabel(QString());
        _leftContentLayout->addWidget(_customFolderLabel);
        customFolderListWidget = gsl::owner<FileCardView*>(new FileCardView(documentsWidget));
        customFolderListWidget->setProperty("startFileCardList", true);
        connect(customFolderListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
        _leftContentLayout->addWidget(customFolderListWidget);
    }

    _leftContentLayout->setSpacing(sectionSpacing);
    _leftContentLayout->addStretch(1);

    // Footer (inside left column, after stretch)
    auto footerLayout = gsl::owner<QHBoxLayout*>(new QHBoxLayout());
    _leftContentLayout->addLayout(footerLayout);

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
    footerLayout->addWidget(_headerLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(_showOnStartupCheckBox);

    _bodyLayout->addWidget(_leftScrollArea, 7);

    // ---- Vertical sidebar divider ----
    _sidebarDivider = gsl::owner<QFrame*>(new QFrame());
    _sidebarDivider->setObjectName(QStringLiteral("startSidebarDivider"));
    _sidebarDivider->setFrameShape(QFrame::VLine);
    _sidebarDivider->setFixedWidth(1);
    _bodyLayout->addWidget(_sidebarDivider);

    // ---- Right column (sidebar) ----
    _rightScrollArea = gsl::owner<QScrollArea*>(new QScrollArea());
    _rightScrollArea->setObjectName(QStringLiteral("startRightScrollArea"));
    _rightScrollArea->setFrameShape(QFrame::NoFrame);
    _rightScrollArea->setWidgetResizable(true);
    _rightScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _rightScrollArea->setMinimumWidth(250);
    _rightScrollArea->setMaximumWidth(320);

    _rightPanel = gsl::owner<QWidget*>(new QWidget());
    _rightScrollArea->setWidget(_rightPanel);
    auto rightLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_rightPanel));
    rightLayout->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
    rightLayout->setContentsMargins(pageMargin, pageMargin, pageMargin, pageMargin);

    // EXAMPLES section in sidebar
    _examplesSectionLabel = makeSectionLabel(QString());
    rightLayout->addWidget(_examplesSectionLabel);

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
        examplesListWidget->setSpacing(4);
        auto examplesDelegate =
            gsl::owner<ExamplesListDelegate*>(new ExamplesListDelegate(examplesListWidget));
        examplesListWidget->setItemDelegate(examplesDelegate);
        connect(examplesListWidget, &QListView::clicked, this, &StartView::fileCardSelected);
        rightLayout->addWidget(examplesListWidget, 1);
    }

    _browseExamplesButton = gsl::owner<QPushButton*>(new QPushButton());
    _browseExamplesButton->setObjectName(QStringLiteral("learnLink"));
    _browseExamplesButton->setFlat(true);
    _browseExamplesButton->setCursor(Qt::PointingHandCursor);
    _browseExamplesButton->setIcon(QIcon(QLatin1String(":/icons/document-open.svg")));
    connect(_browseExamplesButton, &QPushButton::clicked, this, &StartView::openExistingFile);
    rightLayout->addWidget(_browseExamplesButton);

    // Stretch pushes Learn to bottom
    rightLayout->addStretch();

    // LEARN section at bottom of sidebar
    _learnSectionLabel = makeSectionLabel(QString());
    rightLayout->addWidget(_learnSectionLabel);

    auto learnLinks = gsl::owner<LearnLinksWidget*>(new LearnLinksWidget());
    rightLayout->addWidget(learnLinks);

    _bodyLayout->addWidget(_rightScrollArea, 3);

    setCentralWidget(documentsWidget);

    configureRecentFilesListWidget(recentFilesListWidget, _recentFilesLabel);
    if (customFolderListWidget) {
        configureCustomFolderListWidget(customFolderListWidget);
    }
    if (examplesListWidget) {
        configureExamplesListWidget(examplesListWidget);
    }

    QTimer::singleShot(2000, [this, recentFilesListWidget]() {
        auto updateFun = [this, recentFilesListWidget]() {
            configureRecentFilesListWidget(recentFilesListWidget, _recentFilesLabel);
        };
        auto recentFiles = Gui::getMainWindow()->findChild<Gui::RecentFilesAction*>();
        if (recentFiles != nullptr) {
            connect(recentFiles, &Gui::RecentFilesAction::recentFilesListModified, this, updateFun);
        }
    });

    isInitialized = true;
    retranslateUi();
}

void StartView::configureNewFileButtons(QLayout* layout) const
{
    auto partDesign = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("Parametric Body"),
         tr("Creates a body with the Part Design workbench"),
         QLatin1String(":/icons/PartDesignWorkbench.svg")}
    ));
    auto assembly = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("Assembly"),
         tr("Creates an assembly project"),
         QLatin1String(":/icons/AssemblyWorkbench.svg")}
    ));
    auto arch = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("BIM/Architecture"),
         tr("Creates an architectural project"),
         QLatin1String(":/icons/BIMWorkbench.svg")}
    ));
    auto draft = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("2D Draft"), tr("Creates a 2D Draft document"), QLatin1String(":/icons/DraftWorkbench.svg")}
    ));
    auto newEmptyFile = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("Empty File"),
         tr("Creates a new empty FreeCAD file"),
         QLatin1String(":/icons/document-new.svg")}
    ));
    auto openFile = gsl::owner<NewFileButton*>(new NewFileButton(
        {tr("Open File"),
         tr("Opens an existing CAD file or 3D model"),
         QLatin1String(":/icons/document-open.svg")}
    ));

    // TODO: Ensure all of the required WBs are actually available
    layout->addWidget(partDesign);
    layout->addWidget(assembly);
    layout->addWidget(arch);
    layout->addWidget(draft);
    layout->addWidget(newEmptyFile);
    layout->addWidget(openFile);

    connect(newEmptyFile, &QPushButton::clicked, this, &StartView::newEmptyFile);
    connect(openFile, &QPushButton::clicked, this, &StartView::openExistingFile);
    connect(partDesign, &QPushButton::clicked, this, &StartView::newPartDesignFile);
    connect(assembly, &QPushButton::clicked, this, &StartView::newAssemblyFile);
    connect(draft, &QPushButton::clicked, this, &StartView::newDraftFile);
    connect(arch, &QPushButton::clicked, this, &StartView::newArchFile);
}

void StartView::configureFileCardWidget(QListView* fileCardWidget)
{
    auto delegate = gsl::owner<FileCardDelegate*>(new FileCardDelegate(fileCardWidget));
    fileCardWidget->setItemDelegate(delegate);
}


void StartView::configureRecentFilesListWidget(QListView* recentFilesListWidget, QLabel* recentFilesLabel)
{
    _recentFilesModel.loadRecentFiles();
    recentFilesListWidget->setModel(&_recentFilesModel);
    configureFileCardWidget(recentFilesListWidget);

    auto recentFilesGroup = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/RecentFiles"
    );
    auto numRecentFiles {recentFilesGroup->GetInt("RecentFiles", 0)};
    if (numRecentFiles == 0) {
        recentFilesListWidget->hide();
        recentFilesLabel->hide();
    }
    else {
        recentFilesListWidget->show();
        recentFilesLabel->show();
    }
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

void StartView::showOnStartupChanged(bool checked)
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    hGrp->SetBool("ShowOnStartup", !checked);
}

void StartView::openFirstStartClicked()
{
    StartupWizardController::instance().showManual();
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

    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
        updateWordmark();
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
    updateWordmark();
    Gui::MDIView::showEvent(event);
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

    // Version label
    auto versionStr = QString::fromUtf8(App::Application::Config()["ExeVersion"].c_str());
    _headerLabel->setText(QStringLiteral("v%1").arg(versionStr));

    // Section labels (plain text, styled via QSS)
    _recentFilesLabel->setText(tr("Recent Files"));
    _createNewLabel->setText(tr("Create New"));
    if (_examplesSectionLabel) {
        _examplesSectionLabel->setText(tr("Examples"));
    }
    if (_learnSectionLabel) {
        _learnSectionLabel->setText(tr("Learn"));
    }
    if (_browseExamplesButton) {
        _browseExamplesButton->setText(tr("Browse all examples..."));
    }

    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    std::string customFolderPath(hGrp->GetASCII("CustomFolder", ""));
    if (!customFolderPath.empty() && _customFolderLabel) {
        if (hGrp->GetBool("ShortCustomFolder", true)) {
            _customFolderLabel->setToolTip(QString::fromUtf8(customFolderPath.c_str()));
            customFolderPath = customFolderPath.substr(customFolderPath.find_last_of("/\\") + 1);
        }
        _customFolderLabel->setText(QString::fromUtf8(customFolderPath.c_str()));
    }

    _openFirstStart->setText(tr("Setup Wizard"));
    _showOnStartupCheckBox->setText(tr("Do not show this Start page again"));
}
