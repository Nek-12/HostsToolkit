#pragma once
#include <vector>
#include <set>
#include <map>
#include <string>
#include <fstream>
#include <cassert>
#include <sstream>
#include <QMainWindow>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#ifdef __linux__
#define HOSTS "/etc/hosts"
#endif
#ifdef _WIN32
#define TIME_MULTIPLIER 80000/180
#define HOSTS "C:/Windows/System32/drivers/etc/hosts"
#endif
#define VERSION "1.0.0"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    std::string prepare_file();
    void load_file(const std::string& path);


public slots:
    void sys_load();
    void update_stats();
    void apply();
    void upd_progress_bar(int);
    void save_to();
    void append_entry();
    void open_file();
    void del_selected_list_entry();
    void display_about();
    void delete_selected_file();
signals:
    void updated();
    void progress(int);


private:
    bool process_line(std::string& );
    std::vector<std::string> files;
    std::vector<QListWidgetItem* > filepaths;
    std::vector<QListWidgetItem* > customlines;
    bool sys_loaded = false;
    bool pending = false;
    Ui::MainWindow *ui;
    qulonglong total_lines = 1;

};
