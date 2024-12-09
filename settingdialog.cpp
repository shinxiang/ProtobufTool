#include "settingdialog.h"
#include "ui_settingdialog.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::initSetting(const Config *cfg)
{
    if (cfg != NULL) {
        ui->comboBox_jsonPrint->setCurrentIndex(cfg->m_jsonPrintAll);
        ui->spinBox_headLength->setValue(cfg->m_headLength);
        ui->spinBox_tailLength->setValue(cfg->m_tailLength);
    }
}

void SettingDialog::on_pushButton_ok_clicked()
{
    Config* cfg = new Config(
        ui->comboBox_jsonPrint->currentIndex(),
        ui->spinBox_headLength->text().toInt(),
        ui->spinBox_tailLength->text().toInt()
    );
    emit sendConfig(cfg);

    this->close();
}

void SettingDialog::on_pushButton_cancel_clicked()
{
    this->close();
}
