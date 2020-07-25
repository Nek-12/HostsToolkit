#include "app.h"
#include "engine.h"
#include <qmessagebox.h>
#include <qnamespace.h>
// TODO: Add translations

App::App(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), e(this) {
    ui->setupUi(this);
    qDebug() << "Created engine, connecting...";
    // ENGINE
    connect(&e, &Engine::stats, this, &App::upd_stats);
    connect(&e, &Engine::ready, this, &App::engine_ready);
    connect(&e, &Engine::failed, this, &App::engine_failed);
    connect(&e, &Engine::message, this, &App::display_msg);
    connect(&e, &Engine::progress, this, &App::upd_progress_bar);
    connect(&e, &Engine::state_updated, this, &App::upd_pending_state);
    // UI
    connect(ui->CustomEntriesList, &QListWidget::itemActivated,
            ui->CustomEntriesList, &QListWidget::openPersistentEditor);
    connect(ui->CustomEntriesList, &QListWidget::currentItemChanged,
            ui->CustomEntriesList, &QListWidget::closePersistentEditor);
    connect(ui->ApplyFileButton, &QPushButton::clicked, this,
            &App::apply_clicked);
    connect(ui->LoadSystemHosts, &QPushButton::clicked, this, &App::sys_load);
    connect(ui->AboutButton, &QPushButton::clicked, this, &App::about_clicked);
    connect(ui->SaveToButton, &QPushButton::clicked, this,
            &App::save_to_clicked);
    connect(ui->AddCustomEntryButton, &QPushButton::clicked, this,
            &App::add_custom_clicked);
    connect(ui->CustomEntryField, &QLineEdit::returnPressed, this,
            &App::add_custom_clicked);
    connect(ui->AddUrlButton, &QPushButton::clicked, this,
            &App::add_url_clicked);
    connect(ui->BrowseFileButton, &QPushButton::clicked, this,
            &App::add_file_clicked);
    connect(ui->DeleteCustomButton, &QPushButton::clicked, this,
            &App::del_custom_clicked);
    connect(ui->DeleteFileButton, &QPushButton::clicked, this,
            &App::del_file_clicked);

    // MISC
    qDebug() << "Loading config...";
    load_config();
    //TODO: Don't permit adding system hosts file two times, add check
}
//      ---------MISC-------
void App::check_and_add_file(const QString& fname) {
    qDebug() << "load_file called";
    QFile f(fname);
    if (f.open(QIODevice::ReadOnly)) {
        add_file(fname);
        qDebug() << "Added the file " << fname;
    } else {
        msg(tr(FILEERRORMSG));
    }
}

void App::load_config() {
    QFile f(CONFIG_FNAME);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    int i = 0;
    while (!f.atEnd()) {
        QString s = f.readLine();
        // # Ignore comments
        if (s.isEmpty() || (!s.isEmpty() && s.startsWith('#')) ||
            std::all_of(s.toStdString().begin(), s.toStdString().end(),
                        isspace))
            continue;
        // First section: >CUSTOM
        if (s.startsWith(SPLIT_CHAR)) {
            ++i;
            continue;
        }
        switch (i) {
        case 1: add_custom(s); break;
        case 2: add_file(s); break;
        case 3: add_url(s); break;
        default:
            throw std::invalid_argument("Uncovered code path while loading files");
        }
        // TODO: Checkboxes states
    }
}
void App::start_engine(const QString& path) {
    if (!path.isEmpty()) {
        std::ofstream f(path.toStdString());
        if (f) {
            e.start_work(path.toStdString(), ui->RemCommentsBox->isChecked(),
                         ui->RemDupsBox->isChecked(),
                         ui->AddCreditsBox->isChecked(),
                         ui->AddStatsBox->isChecked());
            return;
        }
        msg(tr(FILEERRORMSG));
    }
}

void App::add_url(const QString& s) {
    e.add_url(s);
    new QListWidgetItem(s, ui->UrlsList);
}
void App::add_file(const QString& s) {
    e.add_file(s.toStdString());
    new QListWidgetItem(s, ui->FileList);
}
void App::add_custom(const QString& s) {
    e.add_custom(s.toStdString());
    new QListWidgetItem(s, ui->CustomEntriesList);
}

void App::engine_ready() {
    QMessageBox::information(this, "Success",
                             "Your file was created successfully");
}

void App::display_msg(const QString& msg) { ui->Stats->showMessage(msg); }

App::~App() { delete ui; }

void App::save_config() {
    std::ofstream f(CONFIG_FNAME);
    if (!f)
        throw std::runtime_error(
            "Couldn't open file for saving configuration.");
    f << "# HostsTools configuration file.\n"
      << CREDITS << "# You may edit this file, "
      << "but don't touch or change the order of the >HEADERs!\n";
    e.save_entries(f);
}

//      ----------UI--------
void App::engine_failed(const QString& msg) {
    QMessageBox::critical(this, "Saving failed",QString(
                          "Failed to download files and/or process them."
                          "Check URLs and files you added. \n%1").arg(msg));
}

void App::del_url_clicked() {
    auto* item = ui->UrlsList->currentItem();
    if (!item)
        return;
    auto row = ui->UrlsList->currentRow();
    e.rem_file(row);
    delete item;
}

void App::del_file_clicked() {
    auto* item = ui->FileList->currentItem();
    if (!item)
        return;
    if (item->text() == HOSTS) {
        sys_loaded = false;
        ui->LoadSystemHosts->setEnabled(true);
    }
    auto row = ui->FileList->currentRow();
    e.rem_file(row);
    delete item;
}
void App::del_custom_clicked() {
    e.rem_custom(ui->CustomEntriesList->currentRow());
    delete ui->CustomEntriesList->currentItem();
}

void App::add_custom_clicked() {
    auto entry = ui->CustomEntryField->text();
    if (!entry.isEmpty())
        add_custom(entry);
    ui->CustomEntryField->setText("");
}

void App::add_file_clicked() {
    auto path = QFileDialog::getOpenFileName(
        this, tr("Open a text or hosts file"), HOSTS);
    if (path == HOSTS) {
        sys_load();
        return;
    }
    if (!path.isEmpty())
        add_file(path);
}
//TODO: Add a progress dialog and the ability to cancel
void App::add_url_clicked() {
    QString text = QInputDialog::getText(
        this, "Enter the URL",
        "Enter the path to the file: ", QLineEdit::Normal);
    if (text.isEmpty())
        return;
    for (int i = 0; i < 5; ++i) {
        if (DownloadManager::check_url(QUrl::fromEncoded(text.toLocal8Bit()))) {
            add_url(text);
            return;
        }
    }
    QMessageBox::critical(this, "Invalid URL",
                          "The URL you entered is not available/valid. Aborted after 5 retrials");
}

void App::apply_clicked() {
    if (!e.is_pending() || !e.sources())
        return;
    start_engine(HOSTS);
}
void App::save_to_clicked() {
    if (!e.is_pending() || !e.sources())
        return;
    QString     path;
    QFileDialog d(this, tr("Select the destination file"), HOSTS);
    d.setFileMode(QFileDialog::AnyFile);
    d.selectFile("hosts");
    path = d.getSaveFileName();
    start_engine(path);
}

void App::upd_pending_state() {
    ui->ApplyFileButton->setEnabled(e.is_pending());
    ui->SaveToButton->setEnabled(e.is_pending());
}

void App::sys_load() {
    if (sys_loaded)
        return;
    ui->LoadSystemHosts->setEnabled(false);
    sys_loaded = true;
    check_and_add_file(HOSTS);
}

void App::closeEvent(QCloseEvent* event) {
    save_config();
    e.stop();
    e.deleteLater();
    QWidget::closeEvent(event);
}

void App::upd_stats(const Stats& st) {
    ui->FileStats->setText(
        tr("Total lines: %1 \n"
           "File Size (MB) %2 \n"
           "Comments: %3 \n"
           "Sources: %4 \n"
           "Seconds added to boot up process (Approximate): %5 \n"
           "Total lines removed: %6")
            .arg(st.lines)
            .arg(double(st.size) / 1000000)
            .arg(st.comments)
            .arg(st.sources)
            .arg(st.seconds_added)
            .arg(st.removed));
}

void App::msg(const QString& msg) { ui->Stats->showMessage(msg); }

void App::upd_progress_bar(int val) {
    ui->ProgressBar->setTextVisible(true);
    ui->ProgressBar->setValue(val);
}

void App::about_clicked() {
    QMessageBox::about(this, tr("About HostsTools"),
                       tr("HostsTools V%1\n"
                          "https://github.com/Nek-12/HostsTools\n"
                          "By Nek.12 \n"
                          "t.me/Nek_12\n")
                           .arg(VERSION));
}
