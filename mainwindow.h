#pragma once
#include <QMainWindow>
#include <entry.h>

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
signals:
    void updated();
    void progress(int);


private:
    bool process_line(std::string& );
    std::vector<std::string> files;
    std::vector<std::string> filepaths;
    std::vector<std::string> customlines;
    bool sys_loaded = false;
    bool pending = false;
    Ui::MainWindow *ui;
    qulonglong total_lines = 1;

};
