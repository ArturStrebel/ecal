// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include "monitoring.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QTimer>
#include <ecal/ecal.h>

#include <QMainWindow>

class MainWindow : public QMainWindow, private Ui::MainWindow {
  Q_OBJECT

public:
  MainWindow(Monitoring *monitor, QPushButton *pause_button,
             QWidget *parent = nullptr);

public slots:
  void updateActions();
  void updateProcessGraph();

  // TODO: Die Slots werden bis jetzt nicht verwendet.
  // Entweder entledigen oder die Funktionen verwenden um das Update
  // durchzuführen. Das Update des Process Graph führt im Moment zu einem
  // Aufklappen aller Knoten/Topics im Tree.
private slots:
  void insertChild();
  bool insertColumn();
  void insertRow();
  bool removeColumn();
  void removeRow();

private:
  Monitoring *monitor;
  QPushButton *pauseButton;
};