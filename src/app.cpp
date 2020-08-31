#include "src/app.h"
#include "src/engine.h"
#include <qmessagebox.h>
App::App(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), e(new Engine), progress_bar(this) {
    ui->setupUi(this);
    // ---------ENGINE------
    connect(e, &Engine::stats_ready, this, &App::upd_stats);
    connect(e, &Engine::success, this, &App::engine_ready);
    connect(e, &Engine::failed, this, &App::engine_failed);
    connect(e, &Engine::state_updated, this, &App::upd_pending_state);
    connect(e, &Engine::message, this, &App::msg);
    connect(e, &Engine::progress, this, &App::set_progress);
    connect(e, &Engine::report_speed,&progress_bar,&QProgressDialog::setWindowTitle);
    qDebug() << "Created engine";
    // ----------UI----------
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
    connect(ui->DeleteUrlButton, &QPushButton::clicked, this,
            &App::del_url_clicked);
    ///--PROGRESS DIALOG--
    progress_bar.setWindowModality(
        Qt::WindowModal);                  // block the app window on show()
    progress_bar.setAutoReset(false); // don't reset() on 100%
    progress_bar.reset();             // hide the window
    connect(&progress_bar, &QProgressDialog::canceled, e, &Engine::stop);
    qDebug() << "Created and hidden the progress window";
    //--MISC--
    qDebug() << "Loading config...";
    load_config();
}
//      ---------MISC---------

void App::load_config() {
    QFile f(CONFIG_FNAME);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    int i = 0;
    while (!f.atEnd()) {
        QString s = f.readLine();
        s         = s.remove('\n'); // remove all newlines and space
        s         = s.simplified(); // replace tabs and remove trailing spaces
        if (s.isEmpty() || (!s.isEmpty() && s.startsWith('#')))
            continue; // skip empty strings, system hosts, comments
        if (s == HOSTS) {
            sys_load(); //do not let the system hosts entry be duplicated if it was saved
            continue;
        }
        // Config file sections are handled by their order.
        // There may be better solution, but for now it is the best I could
        // think of without involvind 3rd-party libs
        if (s.startsWith(SPLIT_CHAR)) {
            ++i;
            continue;
        }
        switch (i) {
        case 1: add_custom(s); break;
        case 2: add_file(s); break;
        case 3: add_url(s); break;
        default: display_warning(tr(CONFIG_ERROR_MSG));
        }
    }
}

void App::start_engine(const QString& path) {
    progress_bar.show();
    if (!path.isEmpty()) { // if the user selected something (dialogbox could
                           // return "")
        if (path.isSimpleText())
            e->start_work(path.toStdString(), ui->RemCommentsBox->isChecked(),
                          ui->RemDupsBox->isChecked(),
                          ui->AddCreditsBox->isChecked(),
                          ui->AddStatsBox->isChecked());
        else
            display_warning("The path you entered is not valid.");
        //send the state and create a thread
    }
}

void App::add_url(const QString &s) {
    if (check_url(s)) { //if the url is valid (network is available)
        e->add_url(s); //send to Engine
        new QListWidgetItem(s, ui->UrlsList); //add text
    } else {
        display_warning(QString("Couldn't add URL: ").append(s));
    }
}

void App::add_file(const QString &s) {
    qDebug() << "load_file called";
    QFile f(s);
    if (f.open(QIODevice::ReadOnly)) { //at least check for read access
        e->add_file(s.toStdString());
        new QListWidgetItem(s, ui->FileList);
        qDebug() << "Added the file: " << s;
    } else {
        display_warning(tr(FILEERRORMSG));
    }
}

void App::add_custom(const QString &s) {
    e->add_custom(s.toStdString());
    new QListWidgetItem(s, ui->CustomEntriesList);
}

void App::engine_ready() {
    progress_bar.reset();
    QMessageBox::information(this, "Success",
                             "Your file was saved successfully");
}

App::~App() { delete ui; }

//      ----------UI--------

void App::engine_failed(const QString& msg) {
    progress_bar.reset();
    QMessageBox::critical(
        this, "Saving failed",
        QString("Failed to download files and/or process them. \n%1")
            .arg(msg));
    upd_pending_state();
    //engine will clean up for us
}

void App::del_url_clicked() {
    auto *item = ui->UrlsList->currentItem();
    if (!item) //if the user did not select anything but still clicks
        return;
    auto row = ui->UrlsList->currentRow();
    e->rem_url(row);
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
    e->rem_file(row);
    delete item;
}
void App::del_custom_clicked() {
    e->rem_custom(ui->CustomEntriesList->currentRow());
    delete ui->CustomEntriesList->currentItem();
}

void App::add_custom_clicked() {
    auto entry = ui->CustomEntryField->text();
    // Since hosts are case insensitive and treat tabs as spaces
    entry = entry.simplified().toLower();
    if (!entry.isEmpty())
        add_custom(entry);
    ui->CustomEntryField->setText(""); //erase text that was in the field
}

void App::add_file_clicked() {
    auto path = QFileDialog::getOpenFileName(
        this, tr("Open a text or hosts file"), HOSTS); //get the file path
    if (path == HOSTS) {
        sys_load();
        return;
    }
    if (!path.isEmpty()) //if the user did select something
        add_file(path);
}

void App::add_url_clicked() {
    QString text = QInputDialog::getText(
        this, "Enter the URL",
        "Enter the address of the file: ", QLineEdit::Normal);
    if (text.isEmpty())
        return;
    for (int i = 0; i < URL_RETRIALS_CNT; ++i) {
        if (check_url(QUrl::fromEncoded(text.toLocal8Bit()))) {
            add_url(text);
            return;
        }
    }
    QMessageBox::critical(
        this, "Invalid URL",
        QString("The URL you entered is not available/valid. Aborted after %1 retrials").arg(URL_RETRIALS_CNT));
}

void App::apply_clicked() {
    if (!e->is_pending() || !e->sources())
        return; //if there is nothing, just ignore
    start_engine(HOSTS);
}
void App::save_to_clicked() {
    if (!e->is_pending() || !e->sources())
        return;
    QString     path;
    QFileDialog d(this, tr("Select the destination file"), HOSTS);
    d.setFileMode(QFileDialog::AnyFile);
    d.selectFile("hosts");
    path = d.getSaveFileName();
    if (!path.isEmpty())
        start_engine(path);
}

void App::upd_pending_state() {
    //disable apply buttons if the user did not modify anything since last time->
    ui->ApplyFileButton->setEnabled(e->is_pending());
    ui->SaveToButton->setEnabled(e->is_pending());
}

void App::sys_load() {
    if (sys_loaded)
        return;
    //don't allow multiple calls
    ui->LoadSystemHosts->setEnabled(false);
    sys_loaded = true;
    add_file(HOSTS);
}

//on [X] pressed
void App::closeEvent(QCloseEvent *event) {
    if (!e->save_entries(CONFIG_FNAME))
        display_warning("Couldn't save your settings!");
    e->stop(); // cleanup
    e->deleteLater();
    QWidget::closeEvent(event);
    //QT will handle the memory for us
}

void App::upd_stats(const Stats &st) {
    ui->FileStats->setText(
        tr("Total lines: %1 \n"
           "File Size (KB) %2 \n"
           "Comments: %3 \n"
           "Sources: %4 \n"
           "Seconds added to boot up process (Approximate): %5 \n"
           "Total lines removed: %6")
            .arg(st.lines)
            .arg(double(st.size) / 1000) // convert B to kB
            .arg(st.comments)
            .arg(st.sources)
            .arg(st.seconds_added)
            .arg(st.removed));
    if (st.seconds_added > 300)
        display_warning(tr(
            "Your current hosts file contains more than %1 lines which "
            "means you will have around %2 seconds added to the boot process. "
            "Unless you have a very powerful machine, you will NOT have any access to the internet during these %2 seconds.")
            .arg(st.lines).arg(st.seconds_added));
}

void App::msg(const QString& s) { progress_bar.setLabelText(s); }

void App::about_clicked() {
    QMessageBox::about(this, tr("About HostsTools"),
                       tr("HostsTools V%1\n"
                          "https://github.com/Nek-12/HostsTools\n"
                          "By Nek.12 \n"
                          "t.me/Nek_12\n\n" LICENSE"\n")
                           .arg(VERSION));
}

void App::display_warning(const QString &msg) {
    QMessageBox::warning(this, "Warning!", msg);
}

void App::set_progress(int n) { progress_bar.setValue(n); }
