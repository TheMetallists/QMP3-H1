#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioInput>
#include <QThread>
#include <QBuffer>
#include "charlie_interface.h"
#include <gpsconfig.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow *self;
    int soundRead(void *buf, int size, int count);
    void displayPacket(gpx_t *gpx, int crcOK);


private slots:
    void on_btnStartup_clicked();
    void displayPacketSlot(gpx_t *gpx, int crcOK);
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_btn_gpc_window_clicked();
    void applyGpsConfigRcv(QString port, int baud);

    void on_checkBox_stateChanged(int arg1);
    void onDisplayLevelSig(short lvl);

signals:
    void displayPacketSlotSig(gpx_t *gpx, int crcOK);
    void displayLevelSig(short lvl);

private:
    Ui::MainWindow *ui;
    QScopedPointer<QAudioInput> ain;
    QIODevice *dev;
    GPSConfig *gpc;
    bool startup = false;
    float mLat;
    float mLon; // bLat ?
    void showHeading();

    QString gpsPort;
    int gpsBaud;
    void startStopGPS();
};

class ProcessThread: public QThread {
public:
    bool isInverted;
protected:
    void run();
};

#endif // MAINWINDOW_H
