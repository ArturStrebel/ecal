// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "treemodel.h"

#include <QFile>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(Monitoring *monitor_, QPushButton *pause_button, QWidget *parent_)
    : QMainWindow(parent_), monitor(monitor_), pauseButton(pause_button) {
  setupUi(this);
  this->setMinimumSize(QSize(250, 400));

  // Initial set of model
  const QStringList headers({tr("Topic"), tr("Description")});

  std::vector<eCAL::ProcessGraph::STopicTreeItem> topicTreeItems = {};
  auto *model = new TreeModel(headers, topicTreeItems, this);
  view->setModel(model);
  for (int column = 0; column < model->columnCount(); ++column)
    view->header()->setSectionResizeMode(column, QHeaderView::ResizeToContents);
  view->expandAll();

  connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

  connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this,
          &MainWindow::updateActions);

  connect(actionsMenu, &QMenu::aboutToShow, this, &MainWindow::updateActions);
  connect(insertRowAction, &QAction::triggered, this, &MainWindow::insertRow);
  connect(insertColumnAction, &QAction::triggered, this, &MainWindow::insertColumn);
  connect(removeRowAction, &QAction::triggered, this, &MainWindow::removeRow);
  connect(removeColumnAction, &QAction::triggered, this, &MainWindow::removeColumn);
  connect(insertChildAction, &QAction::triggered, this, &MainWindow::insertChild);

  connect(monitor, &Monitoring::updateTopicTree, this, &MainWindow::updateProcessGraph);

  connect(pauseButton, &QPushButton::toggled, this, [this](bool checked) {
    if (checked == false) {
      connect(monitor, &Monitoring::updateTopicTree, this, &MainWindow::updateProcessGraph);
      pauseButton->setText("Pause");
      updateProcessGraph();
    } else {
      disconnect(monitor, &Monitoring::updateTopicTree, this, &MainWindow::updateProcessGraph);
      pauseButton->setText("Resume");
    }
  });

  updateActions();
}

void MainWindow::updateProcessGraph() {
  eCAL::ProcessGraph::SProcessGraph process_graph = monitor->getProcessGraph();

  const QStringList headers({tr("Topic"), tr("Description")});

  auto *model = new TreeModel(headers, process_graph.topicTreeItems, this);
  view->setModel(model);
  for (int column = 0; column < model->columnCount(); ++column)
    view->header()->setSectionResizeMode(column, QHeaderView::ResizeToContents);
  view->expandAll();
}

void MainWindow::insertChild() {
  const QModelIndex index = view->selectionModel()->currentIndex();
  QAbstractItemModel *model = view->model();

  if (model->columnCount(index) == 0) {
    if (!model->insertColumn(0, index))
      return;
  }

  if (!model->insertRow(0, index))
    return;

  for (int column = 0; column < model->columnCount(index); ++column) {
    const QModelIndex child = model->index(0, column, index);
    model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
    if (!model->headerData(column, Qt::Horizontal).isValid())
      model->setHeaderData(column, Qt::Horizontal, QVariant(tr("[No header]")), Qt::EditRole);
  }

  view->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                          QItemSelectionModel::ClearAndSelect);
  updateActions();
}

bool MainWindow::insertColumn() {
  QAbstractItemModel *model = view->model();
  int column = view->selectionModel()->currentIndex().column();

  // Insert a column in the parent item.
  bool changed = model->insertColumn(column + 1);
  if (changed)
    model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);

  updateActions();

  return changed;
}

void MainWindow::insertRow() {
  const QModelIndex index = view->selectionModel()->currentIndex();
  QAbstractItemModel *model = view->model();

  if (!model->insertRow(index.row() + 1, index.parent()))
    return;

  updateActions();

  for (int column = 0; column < model->columnCount(index.parent()); ++column) {
    const QModelIndex child = model->index(index.row() + 1, column, index.parent());
    model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
  }
}

bool MainWindow::removeColumn() {
  QAbstractItemModel *model = view->model();
  const int column = view->selectionModel()->currentIndex().column();

  // Insert columns in each child of the parent item.
  const bool changed = model->removeColumn(column);
  if (changed)
    updateActions();

  return changed;
}

void MainWindow::removeRow() {
  const QModelIndex index = view->selectionModel()->currentIndex();
  QAbstractItemModel *model = view->model();
  if (model->removeRow(index.row(), index.parent()))
    updateActions();
}

void MainWindow::updateActions() {
  const bool hasSelection = !view->selectionModel()->selection().isEmpty();
  removeRowAction->setEnabled(hasSelection);
  removeColumnAction->setEnabled(hasSelection);

  const bool hasCurrent = view->selectionModel()->currentIndex().isValid();
  insertRowAction->setEnabled(hasCurrent);
  insertColumnAction->setEnabled(hasCurrent);

  if (hasCurrent) {
    view->closePersistentEditor(view->selectionModel()->currentIndex());

    const int row = view->selectionModel()->currentIndex().row();
    const int column = view->selectionModel()->currentIndex().column();
    if (view->selectionModel()->currentIndex().parent().isValid())
      statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
    else
      statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
  }
}
