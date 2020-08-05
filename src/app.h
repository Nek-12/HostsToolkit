#pragma once
#include "./ui_app.h"
#include "src/engine.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressDialog>
#include <fstream>
#ifdef __linux__
#define HOSTS "/etc/hosts"
#define TIME_MULTIPLIER 0
#elif ifdef _WIN32
#define TIME_MULTIPLIER 400
#define HOSTS "C:/Windows/System32/drivers/etc/hosts"
#endif
#define VERSION "3.1.0"
#define CONFIG_FNAME "settings.txt"
// CONFIG has >CUSTOM, >FILES, >URLS in exactly this order!
#define FILEERRORMSG                                                           \
    "Couldn't process your file! Select another location or launch the app "   \
    "with admin privileges."
#define CONFIG_ERROR_MSG "The program couldn't read " CONFIG_FNAME " Your custom settings file may be corrupted."

QT_BEGIN_NAMESPACE
namespace Ui
{
class App;
} // namespace Ui
QT_END_NAMESPACE

class App : public QMainWindow
{
    Q_OBJECT
  public:
    App &operator=(const App &) = delete;
    App(const App &src) = delete;
    explicit App(QWidget *parent = nullptr);
    ~App() override;
    void add_file(const QString &); // just adds the file to both lists
    void add_url(const QString &);
    void add_custom(const QString &);

public slots:
    void upd_stats(const Stats &); // Updates the stats text field
    void apply_clicked();          // if the user clickss "apply to system"
    void save_to_clicked();        // user clicks "save to..."
    void upd_pending_state();      // updates the availability of the save buttons
    void upd_progress_bar(int);    // Sets the value
    void on_engine_ready(); //when the work is done
    void on_engine_failed(const QString &msg); //display error message and abort
    void del_url_clicked();
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
    void msg(const QString &msg);           // Displays message in the statusbar
    void sys_load();                        // Loads the system hosts file
    void save_config();                     // Saves the data
    void load_config();                     // Loads user data
    void start_engine(const QString &path); // Starts the engine with the given path

    Ui::MainWindow *ui;
    Engine e;                                // Background processing
    void closeEvent(QCloseEvent *) override; // Save files before exit
    bool sys_loaded = false;                 // Is the system hosts file loaded
};
