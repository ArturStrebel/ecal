// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <QGraphicsView>
#include <ecal/ecal.h>
#include "monitoring.h"
#include "filter.h"
#include <QTimer>
#include <QPushButton>

class Node;
class Edge;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    enum ViewType
    {
        HostView,
        ProcessView
    };
    GraphWidget(Monitoring *monitor, ProcessGraphFilter *filter_, QPushButton *pause_button, GraphWidget::ViewType view_type, QWidget *parent, QString title);

    void itemMoved();
    int random(int from, int to);
    void addNodeToScene(Node *node);
    void applyFilter(eCAL::ProcessGraph::SProcessGraph &process_graph);
    void updateCentralProcess(QString newCentralProcess);

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
    std::map<int, Node *> node_map;
    std::map<std::pair<int, int>, Edge *> edge_map;
    QGraphicsScene *graphicsScene;
    ViewType view_type;
    QString title;
    ProcessGraphFilter *filter;
    QString centralProcess = "";
};
