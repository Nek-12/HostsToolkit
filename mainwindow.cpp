#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->ApplyFileButton, &QPushButton::clicked, this, &MainWindow::apply);
    connect(ui->LoadSystemHosts, &QPushButton::clicked, this, &MainWindow::sys_load);
    connect(this, &MainWindow::updated, this, &MainWindow::update_stats);
    connect(this,&MainWindow::progress,this,&MainWindow::upd_progress_bar);
    connect(ui->SaveToButton, &QPushButton::clicked, this, &MainWindow::save_to);
    connect(ui->AddCustomEntryButton,&QPushButton::clicked,this, &MainWindow::append_entry);
    connect(ui->CustomEntryField,&QLineEdit::returnPressed,this, &MainWindow::append_entry);
    connect(ui->BrowseFileButton, &QPushButton::clicked,this, &MainWindow::open_file);
    connect(ui->CustomEntriesList, &QListWidget::itemActivated, ui->CustomEntriesList,&QListWidget::openPersistentEditor);
    connect(ui->CustomEntriesList, &QListWidget::currentItemChanged, ui->CustomEntriesList, &QListWidget::closePersistentEditor);
    connect(ui->DeleteListItemButton, &QPushButton::clicked, this, &MainWindow::del_selected_list_entry);
    connect(ui->AboutButton,&QPushButton::clicked, this,&MainWindow::display_about);
    connect(ui->DeleteFileButton, &QPushButton::clicked, this, &MainWindow::delete_selected_file);
    load_custom();
}

std::string MainWindow::prepare_file() {
    std::stringstream ss, ret;
    qulonglong commented_lines = 0, total, removed;
    if (ui->AddCreditsBox->isChecked())
        ret << "#This file was generated with HostsToolkit: https://github.com/Nek-12/HostsToolkit \n";
    ret << "\n# ------------- C U S T O M ------------- \n";
    for (const auto& l : customlines)
        ret << l->text().toStdString() << '\n'; //Add custom
    for (const auto& f : files) {
        ss << f << '\n'; //Concatenate the files
    }
    if (ui->RemDupsBox->isChecked()) {
        ret << "\n# ------------- DEDUPLICATED AND SORTED ------------- \n";
        std::set<std::string> strset;
        while (ss) { // Process lines
            double i = double(strset.size())/total_lines;
            emit progress(int(i*100));
            qDebug() << i;
            std::string line;
            std::getline(ss, line);
            commented_lines += process_line(line); //remove comments if needed and increment the counter
            if (line.empty()) continue;
            strset.insert(line); //set does not store duplicates
        }
        ss.clear();
        total = strset.size();
        for (auto& s: strset)
            ret << s << '\n'; //write every line back
    }
    else { //don't remove duplicates
        std::vector<std::string> strvec;
        ret << "\n# ------------- MERGED TOGETHER ------------- \n";
        while (ss) { // Process lines
            double i = strvec.size()/double(total_lines);
            emit progress(int(i/total_lines*100));
            qDebug() << i;
            std::string line;
            std::getline(ss, line);
            commented_lines += process_line(line); //remove comments if needed
            if (line.empty()) continue;
            strvec.push_back(line);
        }
        ss.clear();
        total = strvec.size();
        for (auto& s: strvec) {
            ret << s << '\n'; //write every line back
        }
    }
    removed = total_lines - total;
    auto msg = tr("Generated file: \t Comments removed: %1 \t "
                  "Total lines from files: %2 \t "
                  "Total lines removed: %3 ")
            .arg(commented_lines)
            .arg(total)
            .arg(removed);
    std::string str;
    if (ui->AddCreditsBox->isChecked())
        str.append('#'+msg.toStdString()+'\n');
    ui->Stats->showMessage(msg);
    emit progress(100);
    return str+ret.str();
}

void MainWindow::apply() {
    if (pending) {
        std::ofstream f(HOSTS);
        if (f) {
            f << prepare_file();
            pending = false;
            ui->ApplyFileButton->setEnabled(pending);
            ui->SaveToButton->setEnabled(pending);
            return;
        }
        else {
            ui->Stats->showMessage(tr(FILEERRORMSG));
            return;
        }
    }
}

void MainWindow::sys_load() {
    if (sys_loaded) return;
    ui->LoadSystemHosts->setEnabled(false);
    sys_loaded = true;
    load_file(HOSTS);
}

void MainWindow::update_stats() {
    pending = true;
    ui->ApplyFileButton->setEnabled(pending);
    ui->SaveToButton->setEnabled(pending);
    qulonglong lines = customlines.size(), comments = 0, filenum = files.size(), seconds_to_load = 0;
    char symbol;
    std::string opt;
    size_t parts = files.size();
    double i = 0;
    for (auto f : files) {
        emit progress(int(++i/parts*100));
        symbol = '#';
        auto pred = [&symbol](char ch) { return ch == symbol; };
        comments += std::count_if(f.begin(), f.end(), pred);
        symbol = '\n';
        lines += std::count_if(f.begin(), f.end(), pred);
    }
    emit progress(100);
#ifdef TIME_MULTIPLIER
    seconds_to_load = qulonglong(lines/TIME_MULTIPLIER);
#endif
    total_lines = lines; // remember the value
    qDebug() << lines;
    ui->Stats->showMessage(tr("Files to merge: %1 \t"
                                   "Lines: %2 \t"
                                   "Comments: %3 \t"
                                   "Seconds added to boot up process (Approximate): %4")
                                   .arg(filenum)
                                   .arg(lines)
                                   .arg(comments)
                                   .arg(seconds_to_load));

}
bool MainWindow::process_line(std::string& line) {
    if (ui->RemCommentsBox->isChecked()) { //remove comments
        auto it = line.find('#');
        if (it != std::string::npos) {
            line.erase(it);
            if (std::all_of(line.begin(),line.end(),isspace)) //Check if only whitespace remains
                line.clear();
            return true;
        }
    }
    return false;
}
void MainWindow::upd_progress_bar(int val) {
    ui->ProgressBar->setTextVisible(true);
    ui->ProgressBar->setValue(val);
}
void MainWindow::save_to() {
    if (!pending || (files.empty() && customlines.empty()) ) return;
    QString path;
    QFileDialog d(this,tr("Select the destination file"),HOSTS);
    d.setFileMode(QFileDialog::AnyFile);
    d.selectFile("hosts");
    path = d.getSaveFileName();
    if (path != "") {
        std::ofstream f(path.toStdString());
        if (f) {
            f << prepare_file();
            pending = false;
            return;
        }
        else
            ui->Stats->showMessage(tr(FILEERRORMSG));
    }
}

void MainWindow::append_entry() {
    auto entry = ui->CustomEntryField->text();
    auto pitem = new QListWidgetItem(entry, ui->CustomEntriesList);
    customlines.push_back(pitem);
    ui->Stats->showMessage(tr("Added custom line: ") + entry);
    ui->CustomEntryField->setText("");
    emit updated();
}
void MainWindow::open_file() {
    auto path = QFileDialog::getOpenFileName(this,tr("Open a text or hosts file"),HOSTS);
    if (path == HOSTS) {
        sys_load();
        return;
    }
    load_file(path.toStdString());
}
void MainWindow::load_file(const std::string& path) {
    qDebug() << "load_file called";
    std::ifstream f(path);
    if (f) {
        std::stringstream ss;
        filepaths.push_back(new QListWidgetItem(QString::fromStdString(path),ui->FileList));
        ss << f.rdbuf();
        files.push_back(ss.str());
        qDebug() << "Added the file " << QString::fromStdString(path);
        emit updated();
    }
    else {
        ui->Stats->showMessage(tr(FILEERRORMSG));
    }
}

void MainWindow::display_about() {
    QMessageBox::about(this,tr("About HostsToolkit"),tr(
            "HostsTools V%1\n"
            "https://github.com/Nek-12/HostsToolkit\n"
            "By Nek.12 \n"
            "t.me/Nek_12\n"
            ).arg(VERSION));
}

void MainWindow::delete_selected_file() {
    auto item = ui->FileList->currentItem();
    if (!item) return;
    if (item->text() == HOSTS) {
        sys_loaded = false;
        ui->LoadSystemHosts->setEnabled(true);
    }
    auto row = ui->FileList->currentRow();
    files.erase(files.begin()+row);
    filepaths.erase(filepaths.begin()+row);
    delete item;
}

void MainWindow::del_selected_list_entry() {
    delete ui->CustomEntriesList->currentItem();
}

void MainWindow::load_custom() {
    std::ifstream f(CONFIG_FNAME);
    while (f) {
        std::string s;
        std::getline(f,s);
        if (!s.empty()) {
            customlines.emplace_back(new QListWidgetItem(QString::fromStdString(s),ui->CustomEntriesList));
        }
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* bar) {
    std::ofstream f(CONFIG_FNAME);
    if (f) {
        for (auto l : customlines)
            f << l->text().toStdString() << '\n';
    }
    QWidget::closeEvent(bar);
}
