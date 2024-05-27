#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_metro_ui.h"
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QFormLayout>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QLineEdit>

class metro_ui : public QMainWindow
{
    Q_OBJECT

public:
    metro_ui(QWidget *parent = nullptr);
    ~metro_ui();

private:
    Ui::metro_uiClass ui;
    void submitForm();
    void checkFields();

private:
    QLabel *label;
    QPushButton *button;
    QLineEdit *departureBox;
    QLineEdit *arrivalBox;
    QDateTimeEdit *dateTimeEdit;
    QSpinBox *passengerCount;
    QLineEdit *passengerInfo;
    QComboBox *ticketType;
    QFormLayout *formLayout;

};

#endif // MAINWINDOW_H
