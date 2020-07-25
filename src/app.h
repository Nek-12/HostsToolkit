#pragma once
#include "./ui_mainwindow.h"
#include "engine.h"
#include <QMainWindow>
#include <fstream>
#include <QFileDialog>

#ifdef __linux__
#define HOSTS           "/etc/hosts"
#define TIME_MULTIPLIER 0
#elif ifdef _WIN32
#define TIME_MULTIPLIER 400
#define HOSTS           "C:/Windows/System32/drivers/etc/hosts"
#endif
#define VERSION            "3.0.0"
#define CONFIG_FNAME       "custom.txt"
//CONFIG has >CUSTOM, >FILES, >URLS in exactly this order!
#define FILEERRORMSG                                                           \
    "Couldn't process your file! Select another location or launch the app "   \
    "with admin privileges."
#define SPLIT_CHAR '>'


class App : public QMainWindow {
    Q_OBJECT
public:
    App& operator=(const App& ) = delete;
    App(const App& src) = delete;
    explicit App(QWidget* parent = nullptr);
    ~App() override;

public slots:
    void upd_stats(const Stats&); // Updates the stats text field
    void apply_clicked(); //if the user clickss "apply to system"
    void save_to_clicked(); //user clicks "save to..."
    void upd_pending_state();   //updates the availability of the save buttons
    void upd_progress_bar(int); //Sets the value
    void engine_failed();
    void rem_url();
    void rem_file();
    void rem_custom();
    void add_custom_entry_clicked();
//signals:


private:
    void add_url(const QString&);
    void add_file(const QString&);
    void add_custom(const QString&);
    void msg(const QString& msg); //Displays message in the statusbar
    void sys_load(); //Loads the system hosts file
    void load_file(const QString& fname); //loads any file (adding it to both ui and engine)
    void save_config();                   // Saves the data
    void load_config();                   // Loads user data
    void start_engine(const QString& path); //Starts the engine with the given path

    Engine e; //Background processing
    void closeEvent(QCloseEvent*) override; //Save files before exit
    bool                           sys_loaded = false; //Is the system hosts file loaded
    Ui::MainWindow*                ui;
};
