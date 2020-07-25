#pragma once
#include "./ui_mainwindow.h"
#include "engine.h"
#include <QMainWindow>
#include <qglobal.h>
#include <qmainwindow.h>
#include <thread>
#include <fstream>

#ifdef __linux__
#define HOSTS           "/etc/hosts"
#define TIME_MULTIPLIER 0
#elif ifdef _WIN32
#define TIME_MULTIPLIER 400
#define HOSTS           "C:/Windows/System32/drivers/etc/hosts"
#endif
#define VERSION            "3.0.0"
#define CONFIG_FNAME       "custom.txt"
#define FILEERRORMSG                                                           \
    "Couldn't process your file! Select another location or launch the app "   \
    "with admin privileges."


class App : public QMainWindow {
    Q_OBJECT
public:
    App& operator=(const App& ) = delete;
    App(const App& src) = delete;
    explicit App(QWidget* parent = nullptr);
    ~App() override;
    int exec();

public slots:

signals:


private:
    void                           update_pending_state(bool);
    Engine e;
    void closeEvent(QCloseEvent *bar) override;
    bool                           sys_loaded = false;
    Ui::MainWindow*                ui;
};
