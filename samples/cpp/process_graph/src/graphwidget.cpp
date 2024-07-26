// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphwidget.h"
#include "edge.h"
#include "node.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <ecal/ecal.h>
#include <math.h>

GraphWidget::GraphWidget(Monitoring *monitor_, ProcessGraphFilter *filter_,
                         QPushButton *pauseButton_, GraphWidget::ViewType viewType_,
                         QWidget *parent_, QString title_)
    : QGraphicsView(parent_), title(title_), viewType(viewType_), monitor(monitor_),
      filter(filter_), pauseButton(pauseButton_) {
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

  // Recurrent update.
  timer = new QTimer(this);
  timer->start(500);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
  connect(pauseButton, &QPushButton::toggled, this, [this](bool checked) {
    if (checked == false) {
      connect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
      pauseButton->setText("Pause");
    } else {
      disconnect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
      pauseButton->setText("Resume");
    }
  });
}

void GraphWidget::applyBlackList(eCAL::ProcessGraph::SProcessGraph &processGraph) {
  for (auto it = processGraph.processEdges.begin(); it != processGraph.processEdges.end();) {
    if (filter->isInBlackList(*it))
      processGraph.processEdges.erase(it++);
    else
      ++it;
  }
}

void GraphWidget::updateCentralProcess(QString newCentralProcess) {
  if (centralProcess == newCentralProcess)
    return;

  for (auto it = nodeMap.begin(); it != nodeMap.end(); it++) {
    if (it->second->getName() == newCentralProcess) {
      it->second->setPosition(QPointF(0, 0));
      graphicsScene->update(sceneRect());
      it->second->setFlag(QGraphicsItem::ItemIsMovable, false);
    }
    if (it->second->getName() == centralProcess)
      it->second->setFlag(QGraphicsItem::ItemIsMovable, true);
  }
  centralProcess = newCentralProcess;
  this->update();
  this->viewport()->update();
}

void GraphWidget::updateProcessGraph() {
  eCAL::ProcessGraph::SProcessGraph processGraph = monitor->getProcessGraph();
  applyBlackList(processGraph);

  updateCentralProcess(QString::fromStdString(filter->getCentralProcess()));
  if (viewType == GraphWidget::ViewType::HostView) {
    // Add new Edges
    for (auto edge : processGraph.hostEdges) {
      if (!(edgeMap.find(edge.edgeID) != edgeMap.end())) {
        // Add new node if incoming Host does not exist
        if (!(nodeMap.find(edge.edgeID.second) != nodeMap.end())) {
          Node *newNode = new Node(Node::Host, QString::fromStdString(edge.incomingHostName),
                                   edge.edgeID.second);
          nodeMap.insert(std::make_pair(edge.edgeID.second, newNode));
          addNodeToScene(newNode);
        }
        // Add new node if outgoing Host does not exist
        if (!(nodeMap.find(edge.edgeID.first) != nodeMap.end())) {
          Node *newNode = new Node(Node::Host, QString::fromStdString(edge.outgoingHostName),
                                   edge.edgeID.first);
          nodeMap.insert(std::make_pair(edge.edgeID.first, newNode));
          addNodeToScene(newNode);
        }

        // Finally add the edge
        if (edge.edgeID.first == edge.edgeID.second) {
          nodeMap[edge.edgeID.first]->setInternalBandwidth(edge.bandwidth);
          edgeMap.insert(std::make_pair(edge.edgeID, nullptr));
        } else {
          Edge *newEdge =
              new Edge(nodeMap[edge.edgeID.first], nodeMap[edge.edgeID.second], "", edge.bandwidth);
          auto reverseEdge = edgeMap.find(std::make_pair(edge.edgeID.second, edge.edgeID.first));
          if (reverseEdge != edgeMap.end()) // if reverse edge exists, change edges to curved
          {
            reverseEdge->second->setCurvedArrow(true);
            newEdge->setCurvedArrow(true);
          }
          edgeMap.insert(std::make_pair(edge.edgeID, newEdge));
          graphicsScene->addItem(newEdge);
        }
      } else {
        if (edge.edgeID.first == edge.edgeID.second) {
          nodeMap[edge.edgeID.first]->setInternalBandwidth(edge.bandwidth);
        } else {
          edgeMap[edge.edgeID]->bandwidth = edge.bandwidth;
          edgeMap[edge.edgeID]->isAlive = true;
        }
        nodeMap[edge.edgeID.first]->isAlive = true;
        nodeMap[edge.edgeID.second]->isAlive = true;
      }
    }

    // Delete Edges that do not exist anymore
    for (auto edge = edgeMap.begin(); edge != edgeMap.end();) {
      if (edge->second == nullptr) {
        ++edge;
        continue; // Skip over "internal edges" TODO: edgeMap shouldnt need nullptr edges
                  // checkout commit bc72e076c2bda8d3cb5d090087d3bf12a1ed34f9 and fix behaviour
                  // there
      }

      if (edge->second->isAlive) {
        edge->second->isAlive = false;
        ++edge;
      } else {
        graphicsScene->removeItem(edge->second);
        edgeMap.erase(edge++);
      }
    }

    for (auto node = nodeMap.begin(); node != nodeMap.end();) {
      if (node->second->isAlive) {
        node->second->isAlive = false;
        ++node;
      } else {
        graphicsScene->removeItem(node->second);
        nodeMap.erase(node++);
      }
    }
  } else if (viewType == GraphWidget::ViewType::ProcessView) {
    for (auto edge : processGraph.processEdges) {
      if (!(edgeMap.find(edge.edgeID) != edgeMap.end())) {
        // Add new node if incoming Host does not exist
        if (!(nodeMap.find(edge.edgeID.first) != nodeMap.end())) {
          Node *newNode = new Node(Node::Publisher, QString::fromStdString(edge.publisherName),
                                   edge.edgeID.first);
          nodeMap.insert(std::make_pair(edge.edgeID.first, newNode));
          addNodeToScene(newNode);
        }

        // Add new node if outgoing Host does not exist
        if (!(nodeMap.find(edge.edgeID.second) != nodeMap.end())) {
          Node *newNode = new Node(Node::Subscriber, QString::fromStdString(edge.subscriberName),
                                   edge.edgeID.second);
          nodeMap.insert(std::make_pair(edge.edgeID.second, newNode));
          addNodeToScene(newNode);
        }

        // Finally add the edge
        Edge *newEdge = new Edge(nodeMap[edge.edgeID.first], nodeMap[edge.edgeID.second],
                                 QString::fromStdString(edge.topicName), edge.bandwidth);
        edgeMap.insert(std::make_pair(edge.edgeID, newEdge));
        graphicsScene->addItem(newEdge);
      } else {
        edgeMap[edge.edgeID]->bandwidth = edge.bandwidth;
        edgeMap[edge.edgeID]->label = QString::fromStdString(edge.topicName);

        nodeMap[edge.edgeID.first]->isAlive = true;
        nodeMap[edge.edgeID.second]->isAlive = true;
        edgeMap[edge.edgeID]->isAlive = true;
      }
    }

    // Delete Edges that do not exist anymore
    for (auto edge = edgeMap.begin(); edge != edgeMap.end();) {
      if (edge->second->isAlive) {
        edge->second->isAlive = false;
        ++edge;
      } else {
        graphicsScene->removeItem(edge->second);
        edgeMap.erase(edge++);
      }
    }

    for (auto node = nodeMap.begin(); node != nodeMap.end();) {
      if (node->second->isAlive) {
        node->second->isAlive = false;
        ++node;
      } else {
        graphicsScene->removeItem(node->second);
        nodeMap.erase(node++);
      }
    }
  }
  this->update();
  this->viewport()->update();
}

void GraphWidget::addNodeToScene(Node *node) {
  graphicsScene->addItem(node);
  node->setGraph(this);
  QPointF pos;

  switch (node->nodeType) {
  case Node::NodeType::Publisher:
    pos = QPointF(-50, GraphWidget::random(-50, 50));
    break;
  case Node::NodeType::Subscriber:
    pos = QPointF(50, GraphWidget::random(-50, 50));
    break;
  case Node::NodeType::Process:
    pos = QPointF(0, 0);
    break;
  default:
    pos = QPointF(GraphWidget::random(-50, 50), GraphWidget::random(-50, 50));
    break;
  }
  node->setPos(pos);
}

int GraphWidget::random(int from, int to) { return rand() % (to - from + 1) + from; }

void GraphWidget::itemMoved() {
  if (!timerId)
    timerId = startTimer(1000 / 25);
}

void GraphWidget::keyPressEvent(QKeyEvent *event) {
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

void GraphWidget::timerEvent(QTimerEvent *event) {
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
void GraphWidget::wheelEvent(QWheelEvent *event) {
  scaleView(pow(2., -event->angleDelta().y() / 240.0));
}
#endif

void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect) {
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
  gradient.setColorAt(0, Qt::black);
  gradient.setColorAt(1, Qt::darkGray);
  painter->fillRect(rect.intersected(sceneRect), gradient);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(sceneRect);

  // Text
  QRectF textRect(sceneRect.left() + 4, sceneRect.top() + 4, sceneRect.width() - 4,
                  sceneRect.height() - 4);

  QFont font = painter->font();
  font.setBold(true);
  font.setPointSize(14);
  painter->setFont(font);
  painter->setPen(Qt::darkGray);
  painter->drawText(textRect.translated(2, 2), title);
  painter->setPen(Qt::white);
  painter->drawText(textRect, title);
}

void GraphWidget::scaleView(qreal scaleFactor) {
  qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
  if (factor < 0.07 || factor > 100)
    return;

  scale(scaleFactor, scaleFactor);
}

void GraphWidget::shuffle() {
  const QList<QGraphicsItem *> items = scene()->items();
  for (QGraphicsItem *item : items) {
    if (qgraphicsitem_cast<Node *>(item))
      item->setPos(-150 + QRandomGenerator::global()->bounded(300),
                   -150 + QRandomGenerator::global()->bounded(300));
  }
}

void GraphWidget::zoomIn() { scaleView(qreal(1.2)); }

void GraphWidget::zoomOut() { scaleView(1 / qreal(1.2)); }
