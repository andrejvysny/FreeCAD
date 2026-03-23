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

#pragma once

#include <Mod/Start/StartGlobal.h>
#include <Base/Type.h>
#include <Gui/MDIView.h>

#include "../App/DisplayedFilesModel.h"
#include "../App/RecentFilesModel.h"
#include "../App/ExamplesModel.h"
#include "../App/CustomFolderModel.h"
#include "OpenFileProxyModel.h"

class QCheckBox;
class QEvent;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QListView;
class QMdiSubWindow;
class QScrollArea;
class QStackedWidget;
class QPushButton;
class QVBoxLayout;

namespace Gui
{
class Document;
}

namespace StartGui
{

class GettingStartedCard;
class LearnLinksWidget;

class StartGuiExport StartView: public Gui::MDIView
{
    Q_OBJECT

    TYPESYSTEM_HEADER_WITH_OVERRIDE();  // NOLINT

public:
    StartView(QWidget* parent);

    const char* getName() const override
    {
        return "StartView";
    }

    void newEmptyFile();
    void newPartDesignFile();
    void openExistingFile();
    void newAssemblyFile();
    void newDraftFile();
    void newArchFile();
    void recentFileAdded(const QString& filename);

    bool onHasMsg(const char* pMsg) const override;

public:
    enum class PostStartBehavior
    {
        switchWorkbench,
        doNotSwitchWorkbench
    };

protected:
    void changeEvent(QEvent* e) override;
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    void configureNewFileButtons(QLayout* layout, bool compact = false) const;
    static void configureFileCardWidget(QListView* fileCardWidget);
    void configureRecentFilesListWidget(QListView* recentFilesListWidget, QLabel* recentFilesLabel);
    void configureExamplesListWidget(QListView* examplesListWidget);
    void configureCustomFolderListWidget(QListView* customFolderListWidget);

    void postStart(PostStartBehavior behavior);

    void fileCardSelected(const QModelIndex& index);
    void showOnStartupChanged(bool checked);
    void openFirstStartClicked();
    void firstStartWidgetDismissed();
    void fileCardContextMenu(const QPoint& pos);

    QString fileCardStyle() const;

private Q_SLOTS:
    void onMdiSubWindowActivated(QMdiSubWindow* subWindow);

private:
    void retranslateUi();
    void setListViewUpdatesEnabled(bool enabled);
    void updateLayout();

    QStackedWidget* _contents = nullptr;
    Start::RecentFilesModel _recentFilesModel;
    Start::ExamplesModel _examplesModel;
    Start::CustomFolderModel _customFolderModel;
    OpenFileProxyModel _openFileProxyModel;

    // Header
    QLabel* _headerLabel = nullptr;

    // Section labels
    QLabel* _recentFilesLabel = nullptr;
    QLabel* _createNewLabel = nullptr;
    QLabel* _customFolderLabel = nullptr;
    QLabel* _examplesSectionLabel = nullptr;
    QLabel* _learnSectionLabel = nullptr;

    // Layout references for responsive reparenting
    QVBoxLayout* _leftContentLayout = nullptr;
    QWidget* _rightPanel = nullptr;
    QScrollArea* _rightScrollArea = nullptr;
    QHBoxLayout* _bodyLayout = nullptr;
    bool _isTwoColumn = true;
    static constexpr int responsiveBreakpoint = 800;

    // Recent files list (stored for context menu)
    QListView* _recentFilesListWidget = nullptr;

    // Sidebar widgets (stored for reparenting)
    QWidget* _examplesContainer = nullptr;
    QWidget* _learnContainer = nullptr;

    // Browse examples button
    QPushButton* _browseExamplesButton = nullptr;

    // Footer
    QPushButton* _openFirstStart = nullptr;
    QCheckBox* _showOnStartupCheckBox = nullptr;

    bool isInitialized = false;
};

}  // namespace StartGui
