#include "mainwindow.h"
#include <gpsconfig.h>


void MainWindow::on_btn_gpc_window_clicked()
{
    if(this->gpc == nullptr){
        this->gpc = new GPSConfig(this);
    }

    connect(this->gpc, &GPSConfig::applyGpsConfig,
        this,&MainWindow::applyGpsConfigRcv);

    this->gpc->show();

}


void MainWindow::applyGpsConfigRcv(QString port, int baud){
    this->gpsPort = port;
    this->gpsBaud = baud;
    this->startStopGPS();
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{

    if(this->gpsBaud <1 || this->gpsPort.length() < 1){
        //
    }
}

void MainWindow::startStopGPS(){
    //
}
