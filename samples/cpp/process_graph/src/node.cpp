// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "edge.h"
#include "node.h"
#include "graphwidget.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

Node::Node(NodeType nodeType, QString name, std::optional<qreal> internalBandwidth_)
    : nodeType(nodeType), name(name), internalBandwidth(internalBandwidth_)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

    QString text_label = name;
    if (internalBandwidth.has_value()) {
        text_label += "\n  🗘" + printHumanReadableBandwidth(internalBandwidth.value());
    }

    // Erstellen des label-Widgets und Einstellen des Textes
    label = new QGraphicsTextItem(this);
    label->setPlainText(text_label);
    label->setPos(5, -30); // Position relativ zum Knoten
}

void Node::setInternalBandwidth(qreal internalBandwidth_) {
    QString text_label = name;
    text_label += "\n  🗘" + QString::number(internalBandwidth_) + " MBit/s";
    label->setPlainText(text_label);
}

void Node::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<Edge *> Node::edges() const
{
    return edgeList;
}

void Node::removeEdge(Edge *edge)
{
    edgeList.removeOne(edge);
}

void Node::calculateForces()
{
    if (!scene() || scene()->mouseGrabberItem() == this) {
        newPos = pos();
        return;
    }

    // Sum up all forces pushing this item away
    qreal xvel = 0;
    qreal yvel = 0;
    qreal charge = 1000.0;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        Node *node = qgraphicsitem_cast<Node *>(item);
        if (!node)
            continue;

        QPointF vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();

        double l = 2.0 * (dx * dx + dy * dy);
        if (l > 0) {
            xvel += (dx * charge) / l;
            yvel += (dy * charge) / l;
        }
    }

    // Now subtract all forces pulling items together
    double weight = (edgeList.size() + 1) * 10;
    for (const Edge *edge : std::as_const(edgeList)) {
        QPointF vec;
        if (edge->sourceNode() == this)
            vec = mapToItem(edge->destNode(), 0, 0);
        else
            vec = mapToItem(edge->sourceNode(), 0, 0);
        xvel -= vec.x() / weight;
        yvel -= vec.y() / weight;
    }

    // Substract forces pulling towards wall in order to sort subscribers right and publishers left.
    QRectF sceneRect = scene()->sceneRect();
    QPointF vec;
    switch (nodeType)
    {
        case Node::NodeType::Subscriber:
            vec = pos() - QPointF(sceneRect.right(), 0);
            xvel -= vec.x() / weight;
            yvel -= vec.y() / weight;
            break;
        case Node::NodeType::Publisher:
            vec = pos() - QPointF(sceneRect.left(), 0);
            xvel -= vec.x() / weight;
            yvel -= vec.y() / weight;
            break;
        default:
            break;
    }

    if (qAbs(xvel) < 0.1 && qAbs(yvel) < 0.1)
        xvel = yvel = 0;

    newPos = pos() + QPointF(xvel, yvel);
    newPos.setX(qMin(qMax(newPos.x(), sceneRect.left() + 10), sceneRect.right() - 10));
    newPos.setY(qMin(qMax(newPos.y(), sceneRect.top() + 10), sceneRect.bottom() - 10));
}

bool Node::advancePosition()
{
    if (newPos == pos() || nodeType == Node::NodeType::Process)
        return false;

    setPos(newPos);
    return true;
}

void Node::setGraph(GraphWidget *newGraphWidget)
{
    graph = newGraphWidget;
}

QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    return QRectF( -10 - adjust, -10 - adjust, 23 + adjust, 23 + adjust);
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);
    return path;
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);

    QColor *light;
    QColor *dark;

    switch (nodeType)
    {
        case Node::Publisher:
            light = new QColor(Qt::gray);
            dark = new QColor(Qt::darkGray);
            break;
        case Node::Process:
            light = new QColor(Qt::blue);
            dark = new QColor(Qt::darkBlue);
            break;
        case Node::Subscriber:
            light = new QColor(Qt::yellow);
            dark = new QColor(Qt::darkYellow);
            break;
        case Node::Host:
            light = new QColor(Qt::green);
            dark = new QColor(Qt::darkGreen);
            break;
        default:
            light = new QColor(Qt::blue);
            dark = new QColor(Qt::darkBlue);
            break;
    }

    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, *light);
        gradient.setColorAt(0, *dark);
    } else {
        gradient.setColorAt(0, *light);
        gradient.setColorAt(1, *dark);
    }
    painter->setBrush(gradient);

    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-10, -10, 20, 20);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        for (Edge *edge : std::as_const(edgeList))
            edge->adjust();
        graph->itemMoved();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

// TODO:: Same function as in Node, implement more elegant solution
QString Node::printHumanReadableBandwidth(qreal& internalBandwidth_) {
    int bandwidthDimension = 0;
    while( internalBandwidth_ > 1024 && bandwidthDimension < 4) {
        internalBandwidth_ /= 1024;
        bandwidthDimension++;
    }      

    switch(bandwidthDimension) {
        case 1: return QString::number(internalBandwidth_) + " Kbit/s";
        case 2: return QString::number(internalBandwidth_) + " Mbit/s";
        case 3: return QString::number(internalBandwidth_) + " Gbit/s";
        default: return QString::number(internalBandwidth_) + " Bit/s";
    }
}