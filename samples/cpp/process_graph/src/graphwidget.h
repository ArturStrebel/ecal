// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include "filter.h"
#include "monitoring.h"
#include <QGraphicsView>
#include <QPushButton>
#include <QTimer>
#include <ecal/ecal.h>

class Node;
class Edge;

class GraphWidget : public QGraphicsView {
  Q_OBJECT

public:
  enum ViewType { HostView, ProcessView };
  GraphWidget(Monitoring *monitoringPtr, ProcessGraphFilter *filterPtr, QPushButton *pauseButton,
              GraphWidget::ViewType viewType, QWidget *parent, QString title);

  void itemMoved();
  int random(int from, int to);
  void addNodeToScene(Node *node, std::optional<double> xHint = std::nullopt,
                      std::optional<double> yHint = std::nullopt);
  void applyBlacklist();
  void updateCentralProcess(int newCentralProcess);

public slots:
  void shuffle();
  void zoomIn();
  void zoomOut();
  void updateProcessGraph();

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void timerEvent(QTimerEvent *event) override;
#if QT_CONFIG(wheelevent)
  void wheelEvent(QWheelEvent *event) override;
#endif
  void drawBackground(QPainter *painter, const QRectF &rect) override;

  void scaleView(qreal scaleFactor);

private:
  Monitoring *monitor;
  QPushButton *pauseButton;
  QTimer *timer;
  int timerId = 0;
  std::map<int, Node *> nodeMap;
  std::map<std::pair<int, int>, Edge *> edgeMap;
  QGraphicsScene *graphicsScene;
  ViewType viewType;
  QString title;
  ProcessGraphFilter *filter;
  int centralProcess = -1;
};