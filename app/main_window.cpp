#include "main_window.hpp"

#include "openlab/core/version.hpp"

#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

namespace openlab::app {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("OpenLab Simulator"));
    resize(1180, 760);
    setMinimumSize(840, 560);
    setCentralWidget(createWorkspace());

    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&Fichier"));
    QAction* quitAction = fileMenu->addAction(QStringLiteral("&Quitter"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    auto* helpMenu = menuBar()->addMenu(QStringLiteral("&Aide"));
    QAction* aboutAction = helpMenu->addAction(QStringLiteral("À &propos"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    auto* libraryDock = new QDockWidget(QStringLiteral("Bibliothèque"), this);
    libraryDock->setObjectName(QStringLiteral("componentLibraryDock"));
    libraryDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    auto* libraryContent = new QLabel(
        QStringLiteral("La bibliothèque de composants sera ajoutée avec le Core.\n\n"
                       "Aucun composant n’est simulé ou affiché comme disponible à ce stade."),
        libraryDock);
    libraryContent->setWordWrap(true);
    libraryContent->setMargin(12);
    libraryDock->setWidget(libraryContent);
    addDockWidget(Qt::LeftDockWidgetArea, libraryDock);

    statusBar()->showMessage(QStringLiteral("Application démarrée — Core %1").arg(QString::fromUtf8(openlab::core::version().data(), static_cast<qsizetype>(openlab::core::version().size()))));
}

QWidget* MainWindow::createWorkspace() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(48, 48, 48, 48);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Bienvenue dans OpenLab Simulator"), page);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* description = new QLabel(
        QStringLiteral("Le shell natif est opérationnel. Le projet ne contient pas encore de système scientifique : "
                       "les éditeurs, moteurs et instruments seront ajoutés après validation de leurs contrats."),
        page);
    description->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addStretch();
    return page;
}

void MainWindow::showAboutDialog() {
    QMessageBox::about(this,
                       QStringLiteral("À propos d’OpenLab Simulator"),
                       QStringLiteral("OpenLab Simulator %1\n\n"
                                      "Laboratoire numérique modulaire en construction.\n"
                                      "Aucun moteur de simulation n’est encore inclus.")
                           .arg(QString::fromUtf8(openlab::core::version().data(), static_cast<qsizetype>(openlab::core::version().size()))));
}

} // namespace openlab::app

