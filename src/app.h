#pragma once
#include "src/const.h"
#include "./ui_app.h"
#include "src/engine.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressDialog>
#include <fstream>
#include "src/const.h"
// NOLINTNEXTLINE
QT_BEGIN_NAMESPACE
namespace Ui {
class App;
} // namespace Ui
QT_END_NAMESPACE

// Main application class
// It was convenient for me to let the App class handle not only UI but some
// internal processes too.
// If the program is going to grow, the decoupling will be improved.
class App : public QMainWindow {
    Q_OBJECT
public:
    App &operator=(const App &) = delete; //singleton. kind of.
    App(const App &src)         = delete;
    explicit App(QWidget *parent = nullptr);
    ~App() override;
    void add_file(const QString &); //check and add the file path to memory
    void add_url(const QString &); //check and add the url through dialog box
    void add_custom(const QString &); //add a new custom entry

public slots:
    void upd_stats(const Stats &); // Updates the stats text field
    void apply_clicked();          // if the user clickss "apply to system"
    void save_to_clicked();        // user clicks "save to..."
    void upd_pending_state(); // updates the availability of the save buttons
    void engine_ready();      // when the work is done
    void engine_failed(const QString &msg); // display error message and abort
    void del_url_clicked(); //remove various entries
    void del_file_clicked();
    void del_custom_clicked();
    void add_custom_clicked(); // adds a new custom entry
    void add_file_clicked();   // adds a new file
    void add_url_clicked();    // adds a new url
    void about_clicked();      // displays about message
    void display_msg(const QString &);
    void display_warning(const QString &);
    // signals:

private:
    void load_ui_state();         // Loads the state of checkboxes, buttons etc
    void msg(const QString &msg); // Displays message in the statusbar
    void sys_load();              // Loads the system hosts file
    void save_config();           // Saves the data
    void load_config();           // Loads user data
    void
    start_engine(const QString &path); // Starts the engine with the given path

    Ui::MainWindow *ui;
    Engine          e;                       // Background processing
    void closeEvent(QCloseEvent *) override; // Save files before exit
    bool sys_loaded = false;                 // Is the system hosts file loaded
};
