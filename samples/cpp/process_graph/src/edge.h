// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EDGE_H
#define EDGE_H

#include <QGraphicsItem>

class Node;

//! [0]
class Edge : public QGraphicsItem
{
public:
    Edge(Node *sourceNode, Node *destNode, bool singleArrow, bool curvedArrow,
         QString label, qreal bandwith_mbits);

    Node *sourceNode() const;
    Node *destNode() const;

    QString label;
    qreal bandwidth;

    void adjust();

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QString printHumanReadableBandwidth(qreal& internalBandwidth_);
private:
    Node *source, *dest;

    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize = 10;
    bool singleArrow = true;
    bool curvedArrow = false;
};
//! [0]

#endif // EDGE_H
