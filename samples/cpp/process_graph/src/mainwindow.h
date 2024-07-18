// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include "ui_mainwindow.h"
#include <ecal/ecal.h>
#include "monitoring.h"
#include <QTimer>

#include <QMainWindow>

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(Monitoring* monitor, QWidget *parent = nullptr);

public slots:
    void updateActions();
    void updateProcessGraph();

private slots:
    void insertChild();
    bool insertColumn();
    void insertRow();
    bool removeColumn();
    void removeRow();

private:
    Monitoring* monitor;
};