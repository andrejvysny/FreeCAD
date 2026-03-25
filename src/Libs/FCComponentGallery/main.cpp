// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include <QApplication>

#include "GalleryWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("FreeCAD Component Gallery");
    app.setStyle("Fusion");

    FcGallery::GalleryWindow window;
    window.show();

    return app.exec();
}
