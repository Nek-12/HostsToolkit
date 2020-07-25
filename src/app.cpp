#include "src/app.h"
// TODO: Add translations


App::App(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    // ENGINE
    connect(&e, &Engine::stats, this, &App::upd_stats);
    connect(&e, &Engine::ready, this, &App::on_engine_ready);
    connect(&e, &Engine::failed, this, &App::engine_failed);
    connect(&e, &Engine::message, ui->Stats, &QStatusBar::showMessage);
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
    connect(ui->AboutButton, &QPushButton::clicked, this, &App::display_about);
    connect(ui->SaveToButton, &QPushButton::clicked, this,
            &App::save_to_clicked);
    connect(ui->AddCustomEntryButton, &QPushButton::clicked, this,
            &App::add_custom_clicked);
    connect(ui->CustomEntryField, &QLineEdit::returnPressed, this,
            &App::add_custom_clicked);
    connect(ui->BrowseFileButton, &QPushButton::clicked, this,
            &App::add_file_clicked);
    connect(ui->DeleteCustomButton, &QPushButton::clicked, this,
            &App::del_custom_clicked);
    connect(ui->DeleteFileButton, &QPushButton::clicked, this,
            &App::del_file_clicked);
    // MISC
    load_config();
}
//      ---------MISC-------
void App::check_and_add_file(const QString& fname) {
    qDebug() << "load_file called";
    QFile f(fname);
    if (f.open(QIODevice::ReadWrite)) {
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
    while (!f.atEnd()) {
        QString s = f.readLine();
        // # Ignore comments
        if (s.isEmpty() || (!s.isEmpty() && s.startsWith('#')))
            continue;
        // First section: >CUSTOM
        while (!s.startsWith(SPLIT_CHAR))
            add_custom(s);
        // 2nd section: >FILES
        while (!s.startsWith(SPLIT_CHAR))
            add_file(s);
        // 3rd section: >URLS
        while (!s.startsWith(SPLIT_CHAR))
            add_url(s);
        // TODO: Checkboxes
    }
}
void App::start_engine(const QString& path) {
    if (!path.isEmpty()) {
        std::ofstream f(path.toStdString());
        if (f) {
            e.start_work(path, ui->RemCommentsBox->isChecked(),
                         ui->RemDupsBox->isChecked(),
                         ui->AddCreditsBox->isChecked(),
                         ui->AddStatsBox.isChecked());
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

void App::on_engine_ready() {
    QMessageBox::information(this,"Success","Your file was created successfully");
}

App::~App() { delete ui; }

//      ----------UI--------

void App::del_url_clicked() {
    auto* item = ui->UrlsList->currentItem();
    if (!item)
        return;
    auto row = ui->UrlsList->currentRow();
    e.rem_file(row);
    delete item;
}

void App::del_file_clicked() {
    auto *item = ui->FileList->currentItem();
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
    add_file(path);
}

void App::add_url_clicked() {
    QString text = QInputDialog::getText(
        this, "Enter the URL",
        "Enter the path to the file: ", QLineEdit::Normal);
    auto* dialog = new QProgressDialog(this);
    dialog->setRange(0, 0);
    dialog->setWindowTitle("Waiting for the availability of remote file...");
    dialog->setLabelText("If the process is taking too long, cancel and check your "
                         "internet connection");
    while (true) {
        if (DownloadManager::check_url(QUrl::fromEncoded(text.toLocal8Bit()))) {
            add_url(text);
            break;
        }
        if (dialog->wasCanceled()) {
            QMessageBox::critical(this, "Invalid URL", "The URL you entered is not available/valid.");
            break;
        }
    }
    dialog->close();
    dialog->deleteLater();
}

void App::apply_clicked() {
    if (e.is_pending() || !e.sources())
        return;
    start_engine(HOSTS);
}
void App::save_to_clicked() {
    if (e.is_pending() || !e.sources())
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
    QMessageBox::about(this, tr("About HostsToolkit"),
                       tr("HostsTools V%1\n"
                          "https://github.com/Nek-12/HostsToolkit\n"
                          "By Nek.12 \n"
                          "t.me/Nek_12\n")
                           .arg(VERSION));
}
