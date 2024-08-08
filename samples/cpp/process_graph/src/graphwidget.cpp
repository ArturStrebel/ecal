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
    if (checked) {
      disconnect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
      pauseButton->setText("Resume");
    } else {
      connect(timer, SIGNAL(timeout()), this, SLOT(updateProcessGraph()));
      pauseButton->setText("Pause");
    }
  });
}

void GraphWidget::applyBlacklist() {
  for (auto it : nodeMap) {
    if (filter->isInBlacklist(it.first))
      it.second->setVisible(false);
    else
      it.second->setVisible(true);
  }

  for (auto it : edgeMap) {
    if (it.second->sourceNode()->isVisible() && it.second->destNode()->isVisible())
      it.second->setVisible(true);
    else
      it.second->setVisible(false);
  }

  graphicsScene->update(sceneRect());
  this->update();
  this->viewport()->update();
}

void GraphWidget::updateCentralProcess(int newCentralProcess) {
  if (centralProcess == newCentralProcess)
    return;

  for (auto it : nodeMap)
    filter->addToBlacklist(std::to_string(it.second->getId()));

  filter->removeFromBlacklist(std::to_string(newCentralProcess));
  nodeMap[newCentralProcess]->setPosition(sceneRect().center());
  nodeMap[newCentralProcess]->setFlag(QGraphicsItem::ItemIsMovable, false);

  if (centralProcess != -1) // dont update old process at the very first time
    nodeMap[centralProcess]->setFlag(QGraphicsItem::ItemIsMovable, true);

  for (auto it : edgeMap) {
    if (it.second->sourceNode()->getId() == newCentralProcess) {
      filter->removeFromBlacklist(std::to_string(it.second->destNode()->getId()));
    }
    if (it.second->destNode()->getId() == newCentralProcess) {
      filter->removeFromBlacklist(std::to_string(it.second->sourceNode()->getId()));
    }
  }
  centralProcess = newCentralProcess;
  graphicsScene->update(sceneRect());
  this->update();
  this->viewport()->update();
}

void GraphWidget::tryInsertNode(int nodeId, Node::NodeType nodeType, std::string nodeName) {
  if (nodeMap.find(nodeId) != nodeMap.end())
    return;
  Node *newNode = new Node(nodeType, QString::fromStdString(nodeName), nodeId);
  nodeMap.insert(std::make_pair(nodeId, newNode));
  addNodeToScene(newNode);
}

void GraphWidget::insertEdge(std::pair<int, int> edgeID, std::string edgeName,
                             double edgeBandwidth) {
  if (edgeID.first == edgeID.second) {
    nodeMap[edgeID.first]->setInternalBandwidth(edgeBandwidth);
    edgeMap.insert(std::make_pair(edgeID, nullptr));
  } else {
    Edge *newEdge = new Edge(nodeMap[edgeID.first], nodeMap[edgeID.second],
                             QString::fromStdString(edgeName), edgeBandwidth);
    auto reverseEdge = edgeMap.find(std::make_pair(edgeID.second, edgeID.first));
    if (reverseEdge != edgeMap.end()) { // if reverse edge exists, change edges to curved
      reverseEdge->second->setCurvedArrow(true);
      newEdge->setCurvedArrow(true);
    }
    edgeMap.insert(std::make_pair(edgeID, newEdge));
    graphicsScene->addItem(newEdge);
  }
}

void GraphWidget::updateEdge(std::pair<int, int> edgeID, double edgeBandwidth) {
  if (edgeID.first == edgeID.second) {
    nodeMap[edgeID.first]->setInternalBandwidth(edgeBandwidth);
  } else {
    edgeMap[edgeID]->bandwidth = edgeBandwidth;
    edgeMap[edgeID]->isAlive = true;
  }
  nodeMap[edgeID.first]->isAlive = true;
  nodeMap[edgeID.second]->isAlive = true;
}

void GraphWidget::deleteInactiveElements() {
  for (auto edge = edgeMap.begin(); edge != edgeMap.end();) {
    if (edge->second == nullptr) {
      ++edge;
      continue;
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
}

void GraphWidget::updateProcessGraph() {
  eCAL::ProcessGraph::SProcessGraph processGraph = monitor->getProcessGraph();
  if (viewType == GraphWidget::ViewType::HostView) {
    for (auto edge : processGraph.hostEdges) {
      if (edgeMap.find(edge.edgeID) == edgeMap.end()) {
        tryInsertNode(edge.edgeID.first, Node::Host, edge.outgoingHostName);
        tryInsertNode(edge.edgeID.second, Node::Host, edge.incomingHostName);
        insertEdge(edge.edgeID, "", edge.bandwidth);
      } else
        updateEdge(edge.edgeID, edge.bandwidth);
    }
    deleteInactiveElements();
  }
  if (viewType == GraphWidget::ViewType::ProcessView) {
    updateCentralProcess(filter->getCentralProcess());
    applyBlacklist();
    for (auto edge : processGraph.processEdges) {
      if (edgeMap.find(edge.edgeID) == edgeMap.end()) {
        tryInsertNode(edge.edgeID.first, Node::Publisher, edge.publisherName);
        tryInsertNode(edge.edgeID.second, Node::Subscriber, edge.subscriberName);
        insertEdge(edge.edgeID, edge.topicName, edge.bandwidth);
      } else
        updateEdge(edge.edgeID, edge.bandwidth);
    }
    deleteInactiveElements();
  }
  this->update();
  this->viewport()->update();
}

void GraphWidget::addNodeToScene(Node *node, std::optional<qreal> xHint,
                                 std::optional<qreal> yHint) {
  graphicsScene->addItem(node);
  node->setGraph(this);
  const qreal sceneWidth = this->sceneRect().width();
  const qreal sceneHeight = this->sceneRect().height();
  qreal xpos;
  qreal ypos;

  if (!xHint.has_value()) {
    switch (node->nodeType) {
    case Node::NodeType::Publisher:
      xHint = std::round(1.0 / 4.0);
      break;
    case Node::NodeType::Subscriber:
      xHint = std::round(3.0 / 4.0);
      break;
    default:
      xHint = 0;
      break;
    }
  }
  xpos = std::round((2 * xHint.value() - 1) * sceneWidth) + GraphWidget::random(-10, 10);

  if (yHint.has_value())
    ypos = std::round((2 * yHint.value() - 1) * sceneHeight) + GraphWidget::random(-10, 10);
  else
    ypos = GraphWidget::random(-sceneHeight / 2, sceneHeight / 2);

  node->setPos(QPointF(xpos, ypos));
}

int GraphWidget::random(int from, int to) {
  return rand() % (to - from + 1) + from;
}

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

void GraphWidget::zoomIn() {
  scaleView(qreal(1.2));
}

void GraphWidget::zoomOut() {
  scaleView(1 / qreal(1.2));
}
