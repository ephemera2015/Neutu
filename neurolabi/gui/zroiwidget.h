#ifndef ZROIWIDGET_H
#define ZROIWIDGET_H

#include <QTableWidget>
#include <QDockWidget>

#include "flyem/zflyemproofdoc.h"
#include "zcolorscheme.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
QT_END_NAMESPACE

//
class ZROIWidget : public QDockWidget
{
    Q_OBJECT

public:
    ZROIWidget(QWidget *parent = 0);
    ZROIWidget(const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~ZROIWidget();

public:
    void getROIs(Z3DWindow *window,
                 ZDvidInfo &dvidInfo,
                 std::vector<std::string> roiList,
                 std::vector<ZObject3dScan> loadedROIs,
                 std::vector<std::string> roiSourceList);
    void makeGUI();

signals:
    void toBeClosed();

public slots:
    void updateROIs();
    void updateROISelections(int row, int column);
    void updateROIColors(int row, int column);
    void updateROIRendering(QTableWidgetItem* item);
    void updateSelection();

protected:
    void closeEvent(QCloseEvent * event);

public:
    //
    Z3DWindow *m_window;
    ZDvidInfo m_dvidInfo;

    //
    std::vector<std::string> m_roiList;
    std::vector<ZObject3dScan> m_loadedROIs;
    QColor defaultColor;
    std::vector<std::string> m_roiSourceList;
    std::vector<bool> colorModified;

    //
    QCheckBox *selectAll;
    QTableWidget *tw_ROIs;
};


#endif // ZROIWIDGET_H