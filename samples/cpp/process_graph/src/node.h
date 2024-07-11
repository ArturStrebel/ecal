// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef NODE_H
#define NODE_H

#include <QGraphicsItem>
#include <QList>

class Edge;
class GraphWidget;

//! [0]
class Node : public QGraphicsItem
{
public:
    enum NodeType {
        Publisher,
        Process,
        Host,
        Subscriber
    };
    NodeType nodeType;

    Node(NodeType nodeType, QString name, std::optional<qreal> internalBandwidth_mbits = std::nullopt);

    QString printHumanReadableBandwidth(qreal& internalBandwidth_);
    void setInternalBandwidth(qreal internalBandwidth);
    void addEdge(Edge *edge);
    QList<Edge *> edges() const;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    void calculateForces();
    bool advancePosition();
    void setGraph(GraphWidget *newGraphWidget);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QList<Edge *> edgeList;
    QPointF newPos;
    GraphWidget *graph;
    QGraphicsTextItem *label;
    std::optional<qreal> internalBandwidth = std::nullopt;
    QString name;
};
//! [0]

#endif // NODE_H
