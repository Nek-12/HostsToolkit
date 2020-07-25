#include "src/app.h"

App::App(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->ApplyFileButton, &QPushButton::clicked, this, &App::apply);
    connect(ui->LoadSystemHosts, &QPushButton::clicked, this, &App::sys_load);
    connect(this, &App::updated, this, &App::update_stats);
    connect(this, &App::progress, this, &App::upd_progress_bar);
    connect(ui->SaveToButton, &QPushButton::clicked, this, &App::save_to);
    connect(ui->AddCustomEntryButton, &QPushButton::clicked, this,
            &App::append_entry);
    connect(ui->CustomEntryField, &QLineEdit::returnPressed, this,
            &App::append_entry);
    connect(ui->BrowseFileButton, &QPushButton::clicked, this, &App::open_file);
    connect(ui->CustomEntriesList, &QListWidget::itemActivated,
            ui->CustomEntriesList, &QListWidget::openPersistentEditor);
    connect(ui->CustomEntriesList, &QListWidget::currentItemChanged,
            ui->CustomEntriesList, &QListWidget::closePersistentEditor);
    connect(ui->DeleteListItemButton, &QPushButton::clicked, this,
            &App::del_selected_list_entry);
    connect(ui->AboutButton, &QPushButton::clicked, this, &App::display_about);
    connect(ui->DeleteFileButton, &QPushButton::clicked, this,
            &App::delete_selected_file);
    connect(ui->AddUrlButton, &QPushButton::clicked, this, &App::add_url);
    load_custom();
}

void App::apply_clicked() {
    
}

void App::update_pending_state(bool state) {
    pending = state;
    ui->ApplyFileButton->setEnabled(false);
    ui->SaveToButton->setEnabled(false);
}

App::~App() { delete ui; }

void App::closeEvent(QCloseEvent *bar) {
    std::ofstream f(CONFIG_FNAME);
    if (f) {
        for (auto *l : customlines)
            f << l->text().toStdString() << '\n';
    }
    QWidget::closeEvent(bar);
}

void App::msg(const QString &msg) { ui->Stats->showMessage(msg); }
