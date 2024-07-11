// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "edge.h"
#include "node.h"

#include <QPainter>
#include <QtMath>

Edge::Edge(Node *sourceNode, Node *destNode, bool singleArrow, bool curvedArrow, QString label, qreal bandwidth_)
    : source(sourceNode), dest(destNode), singleArrow(singleArrow), curvedArrow(curvedArrow), label(label), bandwidth(bandwidth_)
{
    setAcceptedMouseButtons(Qt::NoButton);
    source->addEdge(this);
    dest->addEdge(this);
    adjust();
}

Node *Edge::sourceNode() const
{
    return source;
}

Node *Edge::destNode() const
{
    return dest;
}

void Edge::adjust()
{
    if (!source || !dest)
        return;

    QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
    qreal length = line.length();

    prepareGeometryChange();

    if (length > qreal(20.)) {
        QPointF edgeOffset((line.dx() * 10) / length, (line.dy() * 10) / length);
        sourcePoint = line.p1() + edgeOffset;
        destPoint = line.p2() - edgeOffset;
    } else {
        sourcePoint = destPoint = line.p1();
    }
}

QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 2.0;

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                      destPoint.y() - sourcePoint.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;

    // Draw the line itself
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    qreal excentricity;

    if (curvedArrow) {
        excentricity = 25.0;
    }
    else {
        excentricity = 0.0;
    }

    QPainterPath pathPainter;
    QPointF lineVec = destPoint - sourcePoint;
    QPointF orthogonalVector = QPointF(lineVec.y(), -lineVec.x()) / lineVec.manhattanLength();
    QPointF midPoint((sourcePoint.x() + destPoint.x()) / 2, (sourcePoint.y() + destPoint.y()) / 2);
    QPointF quadSupport = midPoint + orthogonalVector * excentricity;

    pathPainter.moveTo(sourcePoint);
    pathPainter.quadTo(quadSupport, destPoint); // draw a quadratic bezier from source to dest
    painter->drawPath(pathPainter);

    // Similarly modify the arrow drawing section to account for the possible curved line
    painter->setBrush(Qt::black);
    QLineF tangent(quadSupport, destPoint);
    double arrowAngle = std::atan2(-tangent.dy(), tangent.dx());

    QPointF destArrowP1 = destPoint + QPointF(sin(arrowAngle - M_PI / 3) * arrowSize,
                                              cos(arrowAngle - M_PI / 3) * arrowSize);
    QPointF destArrowP2 = destPoint + QPointF(sin(arrowAngle - M_PI + M_PI / 3) * arrowSize,
                                              cos(arrowAngle - M_PI + M_PI / 3) * arrowSize);
    painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);

    if (!singleArrow && !curvedArrow)
    {
        QPointF sourceArrowP1 = sourcePoint + QPointF(sin(arrowAngle + M_PI / 3) * arrowSize,
                                                      cos(arrowAngle + M_PI / 3) * arrowSize);
        QPointF sourceArrowP2 = sourcePoint + QPointF(sin(arrowAngle + M_PI - M_PI / 3) * arrowSize,
                                                      cos(arrowAngle + M_PI - M_PI / 3) * arrowSize);
        painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
    }

    // Draw the label

    // Calc width for label
    int fontsize = 10;
    qreal width = std::max(static_cast<double>(label.length()), 12.0) * fontsize * 0.75;
    qreal height = 30;

    // Set the color for the label
    QColor labelColor(0, 0, 0);

    // Set the font for the label
    QFont font = painter->font();
    font.setPointSize(fontsize);
    painter->setFont(font);

    // Rotate the painter's coordinate system
    painter->save();
    painter->translate(midPoint);
    qreal labelAngle = std::atan2(-line.dy(), line.dx());
    painter->rotate(-labelAngle * 180 / M_PI); // Convert from radians to degrees

    // Draw the label
    painter->setPen(labelColor);
    painter->drawText(QRectF(- width / 2, - height / 2 - excentricity, width, height), Qt::AlignCenter, label + "\n" + printHumanReadableBandwidth(bandwidth));

    // Restore the painter's coordinate system
    painter->restore();

}
// TODO:: Same function as in Node, implement more elegant solution
QString Edge::printHumanReadableBandwidth(qreal& internalBandwidth_) {
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