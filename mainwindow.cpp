#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include "main.h"

void MainWindow::process_files() {

}

void MainWindow::sys_load() {
    Data& data = Data::get();
    if (data.sys_loaded == True) return;
    data.sys_loaded = True;
    std::ifstream f(HOSTS);
    if (!f) throw std::runtime_error("Couldn't open the hosts file. The app may lack privileges.");
    std::stringstream ss;
    ss << f.rdbuf();
    data.files.push_back(ss.str());
    data.update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->ProcessFilesButton, &QPushButton::clicked, this, &MainWindow::process_files);
    connect(ui->LoadSystemHosts, &QPushButton::clicked, this, &MainWindow::sys_load);
}


