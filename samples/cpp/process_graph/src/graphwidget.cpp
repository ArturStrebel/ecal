// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphwidget.h"
#include "edge.h"
#include "node.h"
#include <ecal/ecal.h>
#include <QTimer>

#include <math.h>

#include <QKeyEvent>
#include <QRandomGenerator>

GraphWidget::GraphWidget(Monitoring* monitor, ProcessGraphFilter* filter_, QPushButton* pause_button, GraphWidget::ViewType view_type, QWidget *parent, QString title)
    : QGraphicsView(parent), title(title), view_type(view_type), monitor(monitor), filter(filter_), pauseButton(pause_button)
{
    // Setup the Scene/UI
    graphicsScene = new QGraphicsScene(this);
    graphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    graphicsScene->setSceneRect(-300, -300, 550, 350);
    setScene(graphicsScene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scale(qreal(0.95), qreal(0.95));
    setMinimumSize(400, 400);
    setWindowTitle(tr("Elastic Nodes"));
 
    // Recurrent update.
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
    connect(pauseButton, &QPushButton::toggled, this, [this] (bool checked) 
    {
        if (checked == false) {
            connect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
            pauseButton->setText("Pause");
        } else {
            disconnect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
            pauseButton->setText("Resume");
        }
    });
    timer->start(500);
}

void GraphWidget::updateProcessGraph() {
    eCAL::ProcessGraph::SProcessGraph process_graph = monitor->getProcessGraph();
    if (view_type == GraphWidget::ViewType::HostView) {

        // Add new Edges
        for (auto edge : process_graph.hostEdges) {
            bool edgeExists = edge_map.find(edge.edgeID) != edge_map.end();
            if (!edgeExists) {
                // Add new node if incoming Host does not exist
                bool incomingNodeExists = node_map.find(edge.edgeID.second) != node_map.end();
                if (!incomingNodeExists) {
                    Node* newNode = new Node(Node::Host, QString::fromStdString(edge.incomingHostName), edge.edgeID.second);
                    node_map.insert(std::make_pair(edge.edgeID.second, newNode));

                    // Add new Node to Scene
                    graphicsScene->addItem(newNode);
                    newNode->setGraph(this);
                    newNode->setPos(GraphWidget::random(-50, 50), GraphWidget::random(-50, 50));
                }

                // Add new node if outgoing Host does not exist
                bool outgoingNodeExists = node_map.find(edge.edgeID.first) != node_map.end();
                if (!outgoingNodeExists) {
                    Node* newNode = new Node(Node::Host, QString::fromStdString(edge.outgoingHostName), edge.edgeID.first);
                    node_map.insert(std::make_pair(edge.edgeID.first, newNode));

                    // Add new Node to Scene
                    graphicsScene->addItem(newNode);
                    newNode->setGraph(this);
                    newNode->setPos(GraphWidget::random(-50, 50), GraphWidget::random(-50, 50));
                }

                // Finally add the edge
                if (edge.edgeID.first == edge.edgeID.second) {
                    node_map[edge.edgeID.first]->setInternalBandwidth(edge.bandwidth);
                    edge_map.insert(std::make_pair(edge.edgeID, nullptr));
                } else {
                    Edge* newEdge = new Edge(node_map[edge.edgeID.first], node_map[edge.edgeID.second], false, "", edge.bandwidth);
                    auto reverseEdge = edge_map.find(std::make_pair(edge.edgeID.second, edge.edgeID.first));
                    if (reverseEdge != edge_map.end()) // if reverse edge exists, change edges to curved
                    {
                        reverseEdge->second->setCurvedArrow(true);
                        newEdge->setCurvedArrow(true);
                    }
                    edge_map.insert(std::make_pair(edge.edgeID, newEdge));
                    graphicsScene->addItem(newEdge);
                }
            } else {
                if (edge.edgeID.first == edge.edgeID.second) {
                    node_map[edge.edgeID.first]->setInternalBandwidth(edge.bandwidth);
                } else {
                    edge_map[edge.edgeID]->bandwidth = edge.bandwidth; 
                }
            }
        }

        // Delete Edges that do not exist anymore
        QList<std::pair<int,int>> edgesToDrop;
        for (const auto& pair: edge_map) {
            std::pair<int,int> edgeToCheck = pair.first;
            bool edgeDeleted = true;
            for (auto edge : process_graph.hostEdges) {
                if (edge.edgeID == edgeToCheck) {
                    edgeDeleted = false;
                    break;
                }
            }

            if (edgeDeleted) {
                Edge* edge = pair.second;
                graphicsScene->removeItem(edge);
                delete edge;
                edgesToDrop.append(edgeToCheck);
            }
        }
        for (auto key : edgesToDrop) {
            edge_map.erase(key);
        }

        // Drop Nodes without Edges
        QList<int> hostsToDrop;
        for (const auto& pair: node_map) {
            int nodeId = pair.first;
            Node* node = pair.second;
            bool nodeDeleted = true;
            for (auto it : process_graph.hostEdges) 
                if(it.edgeID.first == nodeId || it.edgeID.second == nodeId )
                    nodeDeleted = false;
            if (nodeDeleted == true) {
                graphicsScene->removeItem(node);
                delete node;
                hostsToDrop.append(nodeId);
            }
        }
        for (auto key : hostsToDrop) {
            node_map.erase(key);
        }

    } else if (view_type == GraphWidget::ViewType::ProcessView) {

        for (auto edge : process_graph.processEdges) {        
            bool edgeExists = edge_map.find(edge.edgeID) != edge_map.end();
            if (!edgeExists && !filter->isInFilterList(edge)) {
                // Add new node if incoming Host does not exist
                bool publisherNodeExists = node_map.find(edge.edgeID.first) != node_map.end();
                if (!publisherNodeExists) {
                    Node* newNode = new Node(Node::Publisher, QString::fromStdString(edge.publisherName), edge.edgeID.first);
                    node_map.insert(std::make_pair(edge.edgeID.first, newNode));

                    // Add new Node to Scene
                    graphicsScene->addItem(newNode);
                    newNode->setGraph(this);
                    newNode->setPos(GraphWidget::random(-50, 50), GraphWidget::random(-50, 50));
                }

                // Add new node if outgoing Host does not exist
                bool subscriberNodeExists = node_map.find(edge.edgeID.second) != node_map.end();
                if (!subscriberNodeExists) {
                    Node* newNode = new Node(Node::Subscriber, QString::fromStdString(edge.subscriberName), edge.edgeID.second);
                    node_map.insert(std::make_pair(edge.edgeID.second, newNode));

                    // Add new Node to Scene
                    graphicsScene->addItem(newNode);
                    newNode->setGraph(this);
                    newNode->setPos(GraphWidget::random(-50, 50), GraphWidget::random(-50, 50));
                }

                // Finally add the edge
                Edge* newEdge = new Edge(node_map[edge.edgeID.first], node_map[edge.edgeID.second], false, QString::fromStdString(edge.topicName), edge.bandwidth);
                edge_map.insert(std::make_pair(edge.edgeID, newEdge));
                graphicsScene->addItem(newEdge);
            } else {
                if (!filter->isInFilterList(edge))
                {
                    edge_map[edge.edgeID]->bandwidth = edge.bandwidth; 
                    edge_map[edge.edgeID]->label = QString::fromStdString(edge.topicName); 
                }

            }
        
        }
        
        // Delete Edges that do not exist anymore
        QList<std::pair<int,int>> edgesToDrop;
        for (const auto& pair: edge_map) {
            std::pair<int,int> edgeToCheck = pair.first;
            bool edgeDeleted = true;
            if (!filter->isInFilterList(pair.second)) {
                for (auto edge : process_graph.processEdges) {
                    if (edge.edgeID == edgeToCheck) {
                        edgeDeleted = false;
                        break;
                    }
                }
            }

            if (edgeDeleted) {
                Edge* edge = pair.second;
                graphicsScene->removeItem(edge);
                delete edge;
                edgesToDrop.append(edgeToCheck);
            }
        }
        for (auto key : edgesToDrop) {
            edge_map.erase(key);
        }

        // Drop Nodes without Edges
        QList<int> hostsToDrop;
        for (const auto& pair: node_map) {
            int nodeId = pair.first;
            Node* node = pair.second;
            if (node->edges().size() == 0) {
                graphicsScene->removeItem(node);
                delete node;
                hostsToDrop.append(nodeId);
            }
        }
        for (auto key : hostsToDrop) {
            node_map.erase(key);
        }
    }

    this->update();
    this->viewport()->update();
}

int GraphWidget::random(int from, int to) {
    return rand() % (to - from + 1) + from;
}

void GraphWidget::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
}

void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
        shuffle();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void GraphWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QList<Node *> nodes;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
            nodes << node;
    }

    for (Node *node : std::as_const(nodes))
        node->calculateForces();

    bool itemsMoved = false;
    for (Node *node : std::as_const(nodes)) {
        if (node->advancePosition())
            itemsMoved = true;
    }

    if (itemsMoved) {
        this->update();
        this->viewport()->update();
    } else {
        killTimer(timerId);
        timerId = 0;
    }
}


#if QT_CONFIG(wheelevent)
void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scaleView(pow(2., -event->angleDelta().y() / 240.0));
}
#endif

void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    // Shadow
    QRectF sceneRect = this->sceneRect();
    QRectF rightShadow(sceneRect.right(), sceneRect.top() + 5, 5, sceneRect.height());
    QRectF bottomShadow(sceneRect.left() + 5, sceneRect.bottom(), sceneRect.width(), 5);
    if (rightShadow.intersects(rect) || rightShadow.contains(rect))
        painter->fillRect(rightShadow, Qt::darkGray);
    if (bottomShadow.intersects(rect) || bottomShadow.contains(rect))
        painter->fillRect(bottomShadow, Qt::darkGray);

    // Fill
    QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect.intersected(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);

    // Text
    QRectF textRect(sceneRect.left() + 4, sceneRect.top() + 4,
                    sceneRect.width() - 4, sceneRect.height() - 4);

    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(14);
    painter->setFont(font);
    painter->setPen(Qt::lightGray);
    painter->drawText(textRect.translated(2, 2), title);
    painter->setPen(Qt::black);
    painter->drawText(textRect, title);
}

void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}


void GraphWidget::shuffle()
{
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (qgraphicsitem_cast<Node *>(item))
            item->setPos(-150 + QRandomGenerator::global()->bounded(300), -150 + QRandomGenerator::global()->bounded(300));
    }
}

void GraphWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

void GraphWidget::zoomOut()
{
    scaleView(1 / qreal(1.2));
}
