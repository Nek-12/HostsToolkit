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
            getline(ss, line);
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
            getline(ss, line);
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
    auto msg = QString("Generated file \t Comments removed: %1 \t "
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
            ui->ApplyFileButton->setChecked(!pending);
            ui->SaveToButton->setChecked(!pending);
            return;
        }
        else {
            ui->Stats->showMessage(
                    "Couldn't write your file! Select another location or launch the app with admin privileges");
            return;
        }
    }
}

void MainWindow::sys_load() {
    ui->LoadSystemHosts->setChecked(sys_loaded);
    if (sys_loaded) return;
    ui->LoadSystemHosts->setChecked(true);
    sys_loaded = true;
    auto font = ui->LoadSystemHosts->font();
    font.setStrikeOut(true);
    ui->LoadSystemHosts->setFont(font);
    load_file(HOSTS);
}

void MainWindow::update_stats() {
    pending = true;
    ui->ApplyFileButton->setChecked(!pending);
    ui->SaveToButton->setChecked(!pending);
    qulonglong lines = customlines.size(), comments = 0, filenum = files.size(), seconds_to_load;
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
        // TODO: Fix spamming ##### breaking the number of comments
        lines += std::count_if(f.begin(), f.end(), pred);
    }
    emit progress(100);
#ifdef TIME_MULTIPLIER
    seconds_to_load = lines/TIME_MULTIPLIER;
#endif
    total_lines = lines; // remember the value
    qDebug() << lines;
    ui->Stats->showMessage(QString("Files to merge: %1 \t"
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
    ui->ApplyFileButton->setChecked(!pending);
    ui->SaveToButton->setChecked(!pending);
    if (!pending || (files.empty() && customlines.empty()) ) return;
    QString path;
    QFileDialog d(this,"Select the destination file",HOSTS);
    d.setFileMode(QFileDialog::AnyFile);
    d.selectFile("hosts");
    path = d.getSaveFileName();
    qDebug() << path;
    if (path != "") {
        std::ofstream f(path.toStdString());
        if (f) {
            f << prepare_file();
            pending = false;
            return;
        }
        else
            ui->Stats->showMessage(
                    "Couldn't write your file! Select another location or launch the app with admin privileges");
    }
}

void MainWindow::append_entry() {
    auto entry = ui->CustomEntryField->text();
    auto pitem = new QListWidgetItem(entry, ui->CustomEntriesList);
    customlines.push_back(pitem);
    ui->Stats->showMessage("Added custom line: " + entry);
    ui->CustomEntryField->setText("");
    emit updated();
}
void MainWindow::open_file() {
    auto path = QFileDialog::getOpenFileName(this,"Open a text or hosts file",HOSTS);
    if (path == HOSTS) {
        sys_load();
        return;
    }
    load_file(path.toStdString());
}
void MainWindow::load_file(const std::string& path) {
    std::ifstream f(path);
    if (f) {
        std::stringstream ss;
        auto pitem = new QListWidgetItem(path.c_str(),ui->FileList);
        filepaths.push_back(pitem);
        ss << f.rdbuf();
        files.push_back(ss.str());
        qDebug() << "Added the file " << path.c_str(); //TODO: Why isn't string working?

        emit updated();
    }
    else {
        ui->Stats->showMessage(
                "Couldn't write your file! Select another location or launch the app with admin privileges");
    }
}

void MainWindow::display_about() {
    QMessageBox::about(this,"About HostsTools",QString(
            "HostsTools V%1\n"
            "https://github.com/Nek-12/HostsToolkit"
            "By Nek-12 \n"
            "t.me/Nek_12\n"
            ).arg(VERSION));
}


void MainWindow::del_selected_list_entry() {
    auto pitem = ui->CustomEntriesList->currentItem();
    delete pitem;
}

MainWindow::~MainWindow() {
    delete ui;
}
