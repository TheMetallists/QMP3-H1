#ifndef GPSCONFIG_H
#define GPSCONFIG_H

#include <QDialog>

namespace Ui {
class GPSConfig;
}

class GPSConfig : public QDialog
{
    Q_OBJECT

public:
    explicit GPSConfig(QWidget *parent = nullptr);
    ~GPSConfig();
signals:
    void applyGpsConfig(QString port, int baud);

private slots:
    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_comboBox_2_currentIndexChanged(const QString &arg1);

private:
    Ui::GPSConfig *ui;
    void sendConfig();
};

#endif // GPSCONFIG_H
