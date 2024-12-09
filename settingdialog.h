#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include "config.h"

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();

    void initSetting(const Config *cfg);

signals:
    void sendConfig(Config *cfg);

private slots:
    void on_pushButton_ok_clicked();

    void on_pushButton_cancel_clicked();

private:
    Ui::SettingDialog *ui;
};

#endif // SETTINGDIALOG_H
