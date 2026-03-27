// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QBuffer>
#include <QImage>
#include <QTest>
#include <memory>
#include <vector>

#include <App/Application.h>
#include <Base/Parameter.h>
#include <Mod/Start/App/DisplayedFilesModel.h>
#include <Mod/Start/Gui/FileCardDelegate.h>
#include <Mod/Start/Gui/FileCardView.h>
#include <src/App/InitApplication.h>

namespace
{

QByteArray createThumbnailData()
{
    QByteArray thumbnail;
    QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);

    QBuffer buffer(&thumbnail);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return thumbnail;
}

class MockFileCardModel: public QAbstractListModel
{
public:
    explicit MockFileCardModel(int count, QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
        const QByteArray thumbnail = createThumbnailData();
        rows.reserve(count);
        for (int index = 0; index < count; ++index) {
            rows.push_back(
                {QStringLiteral("Recent%1.FCStd").arg(index + 1),
                 QStringLiteral("1.0 MB"),
                 QStringLiteral("/tmp/recent-%1.FCStd").arg(index + 1),
                 thumbnail}
            );
        }
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }

        return static_cast<int>(rows.size());
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(rows.size())) {
            return {};
        }

        const auto& row = rows.at(index.row());
        switch (static_cast<Start::DisplayedFilesModelRoles>(role)) {
            case Start::DisplayedFilesModelRoles::baseName:
                return row.baseName;
            case Start::DisplayedFilesModelRoles::size:
                return row.size;
            case Start::DisplayedFilesModelRoles::path:
                return row.path;
            case Start::DisplayedFilesModelRoles::image:
                return row.image;
            default:
                break;
        }

        return {};
    }

private:
    struct Row
    {
        QString baseName;
        QString size;
        QString path;
        QByteArray image;
    };

    std::vector<Row> rows;
};

class testFileCardViewLayout: public QObject
{
    Q_OBJECT

public:
    testFileCardViewLayout()
    {
        tests::initApplication();
        startPreferences = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Mod/Start"
        );
        originalSpacing = startPreferences->GetInt("FileCardSpacing", 16);
        originalThumbnailSize = startPreferences->GetInt("FileThumbnailIconsSize", 128);
    }

    ~testFileCardViewLayout() override
    {
        startPreferences->SetInt("FileCardSpacing", originalSpacing);
        startPreferences->SetInt("FileThumbnailIconsSize", originalThumbnailSize);
    }

private Q_SLOTS:
    void init()  // NOLINT
    {
        startPreferences->SetInt("FileCardSpacing", 20);
        startPreferences->SetInt("FileThumbnailIconsSize", 64);
    }

    void cleanup()  // NOLINT
    {}

    void firstCardStartsAtViewportOrigin()  // NOLINT
    {
        MockFileCardModel model(1);
        StartGui::FileCardView view;
        auto delegate = new FileCardDelegate(&view);

        view.setProperty("startFileCardList", true);
        view.setItemDelegate(delegate);
        view.setModel(&model);

        const QSize cellSize = delegate->sizeHint(QStyleOptionViewItem(), model.index(0, 0));
        const int width = cellSize.width() + 32;
        view.resize(width, view.heightForWidth(width));
        view.show();
        view.doItemsLayout();

        QTRY_COMPARE(view.visualRect(model.index(0, 0)).topLeft(), QPoint(0, 0));
    }

    void cardsKeepAGapOnTheSameRow()  // NOLINT
    {
        MockFileCardModel model(2);
        StartGui::FileCardView view;
        auto delegate = new FileCardDelegate(&view);

        view.setProperty("startFileCardList", true);
        view.setItemDelegate(delegate);
        view.setModel(&model);

        const QSize cellSize = delegate->sizeHint(QStyleOptionViewItem(), model.index(0, 0));
        const int width = cellSize.width() * 2 + 32;
        view.resize(width, view.heightForWidth(width));
        view.show();
        view.doItemsLayout();

        const QModelIndex firstIndex = model.index(0, 0);
        const QModelIndex secondIndex = model.index(1, 0);
        QTRY_COMPARE(view.visualRect(firstIndex).topLeft(), QPoint(0, 0));

        const QRect firstRect = view.visualRect(firstIndex);
        const QRect secondRect = view.visualRect(secondIndex);
        const int gap = secondRect.left() - firstRect.left() - delegate->cardSize().width();

        QCOMPARE(secondRect.top(), firstRect.top());
        QVERIFY(gap > 0);
    }

    void cardsWrapToTheNextRowWithoutClipping()  // NOLINT
    {
        MockFileCardModel model(2);
        StartGui::FileCardView view;
        auto delegate = new FileCardDelegate(&view);

        view.setProperty("startFileCardList", true);
        view.setItemDelegate(delegate);
        view.setModel(&model);

        const QSize cellSize = delegate->sizeHint(QStyleOptionViewItem(), model.index(0, 0));
        const int width = cellSize.width() - 1;
        view.resize(width, view.heightForWidth(width));
        view.show();
        view.doItemsLayout();

        QTRY_COMPARE(view.visualRect(model.index(0, 0)).topLeft(), QPoint(0, 0));

        const QRect firstRect = view.visualRect(model.index(0, 0));
        const QRect secondRect = view.visualRect(model.index(1, 0));
        QVERIFY(secondRect.top() > firstRect.top());
        QVERIFY(view.height() >= secondRect.bottom() + 1);
    }

private:
    Base::Reference<ParameterGrp> startPreferences;
    long originalSpacing = 0;
    long originalThumbnailSize = 0;
};

}  // namespace

QTEST_MAIN(testFileCardViewLayout)

#include "FileCardViewLayout.moc"
