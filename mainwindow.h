#pragma once
#include "main.h"

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
    void load_custom();


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
    void closeEvent(QCloseEvent *bar) override;
    bool process_line_ip(std::string& );
    std::vector<std::string> files;
    std::vector<QListWidgetItem* > filepaths;
    std::vector<QListWidgetItem *> customlines;
    std::vector<QListWidgetItem *> urls;
    bool sys_loaded = false;
    bool pending = false;
    Ui::MainWindow *ui;
    qulonglong total_lines = 1;

};
