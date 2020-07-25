#include "src/app.h"

App::App(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    // ENGINE
    connect(&e, &Engine::stats, this, &App::upd_stats);
    connect(&e, &Engine::ready, this, &App::upd_pending_state);
    connect(&e, &Engine::failed, this, &App::engine_failed);
    connect(&e, &Engine::message, ui->Stats, &QStatusBar::showMessage);
    connect(&e, &Engine::progress, this, &App::upd_progress_bar);
    // UI
    
    // MISC
    load_config();
}
//      ---------MISC-------
void App::load_file(const QString& fname) {
    qDebug() << "load_file called";
    QFile f(fname);
    if (f.open(QIODevice::ReadWrite)) {
        e.add_file(fname.toStdString());
        new QListWidgetItem(fname, ui->FileList);
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
        while (!s.startsWith(SPLIT_CHAR)) {
            add_custom(s);
        }
        // 2nd section: >FILES
        while (!s.startsWith(SPLIT_CHAR)) {
            add_file(s);
        }
        // 3rd section: >URLS
        while (!s.startsWith(SPLIT_CHAR)) {
            add_url(s);
        }
    }
}
void App::start_engine(const QString& path) {
    if (!path.isEmpty()) {
        std::ofstream f(path.toStdString());
        if (f) {
            e.start_work(path, ui->RemCommentsBox->isChecked(),
                         ui->RemDupsBox->isChecked(),
                         ui->AddCreditsBox->isChecked(),
                         ui->AddStatsBox.isChecked()) return;
            upd_pending_state();
        }
        msg(tr(FILEERRORMSG));
    }
}

void App::add_url(const QString& s) {
    e.add_url(s);
    new QListWidgetItem(s, ui->UrlsList);
    upd_pending_state();
}
void App::add_file(const QString& s) {
    e.add_file(s.toStdString());
    new QListWidgetItem(s, ui->FileList);
    upd_pending_state();
}
void App::add_custom(const QString& s) {
    e.add_custom(s.toStdString());
    new QListWidgetItem(s, ui->CustomEntriesList);
    upd_pending_state();
}



App::~App() { delete ui; }

//      ----------UI--------

void App::add_custom_entry_clicked() {
    auto  entry = ui->CustomEntryField->text();
    auto* pitem = new QListWidgetItem(entry, ui->CustomEntriesList);
    customlines.push_back(pitem);
    msg(tr("Added custom line: ") + entry);
    ui->CustomEntryField->setText("");
    emit updated();
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
    load_file(HOSTS);
}

void App::closeEvent(QCloseEvent* event) {
    save_config();
    e.stop();
    e.deleteLater();
    QWidget::closeEvent(event);
}

void App::upd_stats(const Stats& st) {
    upd_pending_state();
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
