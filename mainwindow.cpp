#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QAudioDeviceInfo>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QTime>
//#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QGeoCoordinate>
#include <QFile>
#include <gpsconfig.h>
#include <math.h>

#if defined _WIN32 || defined __CYGWIN__
    #include <windows.h>
#else
    #include <unistd.h>
#endif



extern "C" {
#include "charlie_interface.h"
}

MainWindow *MainWindow::self = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->gpc = nullptr;
    this->gpsPort = QString("");
    this->gpsBaud = 0;

    //this->ui->checkBox->

    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    this->ui->cbSoundcardList->addItem(defaultDeviceInfo.deviceName(), QVariant::fromValue(defaultDeviceInfo));
    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (deviceInfo != defaultDeviceInfo)
            this->ui->cbSoundcardList->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }

    this->self = this;

    connect(
        this, &MainWindow::displayPacketSlotSig,
        this, &MainWindow::displayPacketSlot
    );
    connect(
        this, &MainWindow::displayLevelSig,
        this, &MainWindow::onDisplayLevelSig
    );
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnStartup_clicked()
{
    this->ui->cbSoundcardList->setEnabled(false);
    this->ui->btnStartup->setEnabled(false);


    QAudioDeviceInfo dif  = QAudioDeviceInfo::defaultInputDevice();

    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (deviceInfo.deviceName() == this->ui->cbSoundcardList->currentData().toString()){
            dif = deviceInfo;
        }
    }


    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    if (!dif.isFormatSupported(format)) {
        QMessageBox Msgbox;
        Msgbox.setWindowTitle("Генерал К.К. Фэйлор");
        Msgbox.setText("Звуковуха, которую вы выбрали не умеет в нужный формат!");
        Msgbox.exec();

        this->ui->cbSoundcardList->setEnabled(true);
        this->ui->btnStartup->setEnabled(true);
        return;
        format = dif.nearestFormat(format);
    }


    this->ain.reset(new QAudioInput(dif, format));
    auto io = this->ain->start(); // was auto io=
    this->dev = io;

    connect(this->dev, &QIODevice::readyRead,
        [&, io]() {
        //qDebug()<<"Ready Read!";
        if(!startup){
            ProcessThread *pt = new ProcessThread();
            pt->isInverted = this->ui->cbInvert->isChecked();
            pt->start();
            startup = true;
        }
    });
}

void ProcessThread::run(){
    int iIsInverted = 0;
    if(this->isInverted)
        iIsInverted = 1;

    processing_threadV(iIsInverted);
}

int MainWindow::soundRead(void *buf, int size, int count){
    if (buf == nullptr)
        return 0;

    //qDebug()<<"READ: "<<size<<" -> "<<count << (long long)buf;

    int tgtLen = size * count;
    while (!(ain->bytesReady() > tgtLen)) {
#if defined _WIN32 || defined __CYGWIN__
    Sleep(100);
#else
    usleep(100 * 1000);
#endif
    }


    auto read = dev->read((char *)buf, tgtLen);
    //    auto read = sndBuffer.read((char *)buf, tgtLen);
    if(read != tgtLen){
        //qDebug()<<"Sound read underrun!";
        //qDebug()<<read<<" ->"<<tgtLen;
        exit(2);
    }

    short *lvlCtrBuf = (short *)buf;
    short lvlAvg = 0;
    for(unsigned int y=0; y<(tgtLen/sizeof(short));y++){
        lvlAvg = (lvlAvg+abs(lvlCtrBuf[y]))/2;
    }
    emit displayLevelSig(lvlAvg);

    return read;
}

void MainWindow::onDisplayLevelSig(short lvl){
    this->ui->pbInputLevel->setValue((int)lvl);
}

extern "C"
int sound_read(void *buf, int size, int count) {

    if(MainWindow::self != nullptr){
        return MainWindow::self->soundRead(buf,size,count);
    }else{
        return -1;
    }
}

void MainWindow::displayPacket(gpx_t *gpx, int crcOK){
    emit displayPacketSlotSig(gpx,crcOK);
}

void MainWindow::displayPacketSlot(gpx_t *gpx, int crcOK){
    //
    if(crcOK){
        this->ui->tbPosition->setText(QString("%1, %2")
                                      .arg(QString::number(gpx->lat,'f',5))
                                      .arg(QString::number(gpx->lon,'f',5))
                                      );
        this->mLat = gpx->lat;
        this->mLon = gpx->lon;
        //
        this->showHeading();

        QString weather("");

        weather += QString("A: [%1 °C, %2 %]  ").arg(gpx->A_adcT).arg(gpx->A_adcH);
        weather += QString("B: [%1 °C, %2 %]  ").arg(gpx->B_adcT).arg(gpx->B_adcH);
        weather += QString("C: [%1 °C, %2 %]  ").arg(gpx->C_adcT).arg(gpx->C_adcH);
        weather += QString("X: [%1 °C, %2 %]").arg(gpx->cfg_T).arg(gpx->cfg_H);

        this->ui->lblWX->setText(weather);

        this->ui->lblSats->setText(QString::number(gpx->numSats));

        this->ui->tbAltitude->setText(QString::number(gpx->alt,'f',5));
        this->ui->cb_gotLock->setChecked(true);

        if(this->ui->cb_writeLog->isChecked()){
            QString lPath = QCoreApplication::applicationDirPath()+QString("/");
            lPath += QDateTime::currentDateTime().toString("yyyy-MM-dd-");
            lPath += QString("%1_LOG.CSV").arg(floor(QDateTime::currentDateTime().time().hour() / 14)*14);


            QFile file(lPath);
            bool drawHeader = !file.exists();

            if(file.open(QIODevice::Append|QIODevice::Text)){

                QTextStream logg(&file);

                if(drawHeader){
                    logg<<"Date;Time;Latitude;Longitude;Altitude;NumSats;Distance;Bearing";
                    Qt::endl(logg);
                }

                logg<<QDateTime::currentDateTime().toString("dd-MM-yyyy")<<";";
                logg<<QDateTime::currentDateTime().toString("hh:mm:ss")<<";";
                logg<<this->mLat << ";"<<this->mLon<<";"<<gpx->alt<<";";
                logg<<gpx->numSats<<";"<<this->ui->lb_tgt_distance->text()<<";";
                logg<<this->ui->lb_tgt_heading->text();
                Qt::endl(logg);

                file.close();
            }
        }




    }else{
        this->ui->cb_gotLock->setChecked(false);
    }



    // display RAW values
    QString raw = QString("RAW: ");

    if(crcOK){
        raw += QString(" [GOOD] ");
    }else{
        raw += QString(" [FAIL] ");
    }

    raw += QString("LAT: %1 ; ").arg(QString::number(gpx->lat,'f',5));
    raw += QString("LON: %1 ; ").arg(QString::number(gpx->lon,'f',5));
    raw += QString("ALT: %1 ; ").arg(QString::number(gpx->alt,'f',5));

    this->ui->rawDataDisplay->setText(raw);
}

void MainWindow::showHeading(){
    // populate distance and bearing
    bool isOk = true;
    float curLat, curLon;

    curLat = this->ui->ed_my_lat->text().toFloat(&isOk);
    if(isOk){
        curLon = this->ui->ed_my_long->text().toFloat(&isOk);
    }

    if(isOk){
        QGeoCoordinate base(curLat,curLon);
        QGeoCoordinate baloon(this->mLat,this->mLon);

        this->ui->lb_tgt_distance->setText(QString::number(base.distanceTo(baloon))+QString(" M"));

        QString hdg("??");
        qreal _hdg = base.azimuthTo(baloon);
        if(_hdg>=340 || _hdg < 30){
            hdg = QString("С");
        }else if (_hdg>=30 || _hdg < 60) {
            hdg = QString("СB");
        }else if (_hdg>=60 || _hdg < 110) {
            hdg = QString("B");
        }else if (_hdg>=110 || _hdg < 150) {
            hdg = QString("ЮB");
        }else if (_hdg>=150 || _hdg < 210) {
            hdg = QString("Ю");
        }else if (_hdg>=210 || _hdg < 250) {
            hdg = QString("ЮЗ");
        }else if (_hdg>=250 || _hdg < 300) {
            hdg = QString("З");
        }else if (_hdg>=300 || _hdg < 340) {
            hdg = QString("CЗ");
        }

        this->ui->lb_tgt_heading->setText(QString::number(base.azimuthTo(baloon),'f',3)+QString(" ° [%1]").arg(hdg));
    }else{
        this->ui->lb_tgt_distance->setText("ERROR");
        this->ui->lb_tgt_heading->setText("ERROR");
    }
}

extern "C"
void display_packet(gpx_t *gpx, int crcOK){
    if(MainWindow::self != nullptr){
        MainWindow::self->displayPacket(gpx,crcOK);
    }else{
        //qDebug()<<"CANNOT DISPLAY PACKET: "<<gpx->lat<<" "<<gpx->lon;
    }
}


void MainWindow::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl(
                                  QString("https://yandex.ru/maps/?pt=%1,%2&z=15&l=skl")
                                  .arg(QString::number(this->mLat,'f',5))
                                  .arg(QString::number(this->mLon,'f',5))
                                  ));
}

void MainWindow::on_pushButton_2_clicked()
{
    QDesktopServices::openUrl(
                QUrl(
                    QString("https://www.google.com/maps/search/?api=1&query=%1,%2")
                    .arg(QString::number(this->mLat,'f',5))
                    .arg(QString::number(this->mLon,'f',5))
    ));
    // https://www.google.com/maps/search/?api=1&query=36.26577,-92.54324
}






