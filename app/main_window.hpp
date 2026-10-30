#pragma once

#include <QMainWindow>

class QWidget;

namespace openlab::app {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void showAboutDialog();

private:
    QWidget* createWorkspace();
};

} // namespace openlab::app

