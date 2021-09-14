#include "gpsconfig.h"
#include "ui_gpsconfig.h"

GPSConfig::GPSConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GPSConfig)
{
    ui->setupUi(this);


    this->ui->comboBox->addItem(QString("/dev/ttyUSB0"),QVariant(0));
    for(int i=1; i<255; i++){
        this->ui->comboBox->addItem(QString("COM%1").arg(i),QVariant(i));
        if(i < 25)
            this->ui->comboBox->addItem(QString("/dev/ttyUSB%1").arg(i),QVariant(i));
    }

    int brates[] = {110, 150, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 0};

    for(int i=0; i<100; i++){
        if(brates[i] < 1)
            break;
        this->ui->comboBox_2->addItem(QString::number(brates[i]),QVariant(brates[i]));
    }
    this->ui->comboBox_2->setCurrentText("9600");

}

GPSConfig::~GPSConfig()
{
    delete ui;
}

void GPSConfig::on_comboBox_currentIndexChanged(const QString &arg1)
{
    // port
    this->sendConfig();
}

void GPSConfig::on_comboBox_2_currentIndexChanged(const QString &arg1)
{
    // baud
    this->sendConfig();
}




void GPSConfig::sendConfig(){
    QString port = this->ui->comboBox->currentText();
    int baudrate = this->ui->comboBox_2->currentData().toInt();
    emit applyGpsConfig(port,baudrate);
}




