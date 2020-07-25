void App::sys_load() {
    if (sys_loaded)
        return;
    ui->LoadSystemHosts->setEnabled(false);
    sys_loaded = true;
    load_file(HOSTS);
}

void App::update_stats() {
    pending = true;
    ui->ApplyFileButton->setEnabled(pending);
    ui->SaveToButton->setEnabled(pending);

    ui->FileStats->setText(tr("Files to merge: %1 \n"
                              "Lines: %2 \n"
                              "Comments: %3 \n"
                              "Seconds added to boot up process (Approximate): %4 \n"
                              "Files to download (not listed here): %5")
                               .arg(parts)
                               .arg(lines)
                               .arg(comments)
                               .arg(seconds_to_load)
                               .arg(urls.size()));
}


void App::upd_progress_bar(int val) {
    ui->ProgressBar->setTextVisible(true);
    ui->ProgressBar->setValue(val);
}

void App::save_to() {
    if (e.pending
        return;//TODO: Test for entries, not just pending.
    QString     path;
    QFileDialog d(this, tr("Select the destination file"), HOSTS);
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
        msg(tr(FILEERRORMSG));
    }
}

void App::append_entry() {
    auto  entry = ui->CustomEntryField->text();
    auto *pitem = new QListWidgetItem(entry, ui->CustomEntriesList);
    customlines.push_back(pitem);
    msg(tr("Added custom line: ") + entry);
    ui->CustomEntryField->setText("");
    emit updated();
}
void App::open_file() {
    auto path = QFileDialog::getOpenFileName(this, tr("Open a text or hosts file"), HOSTS);
    if (path == HOSTS) {
        sys_load();
        return;
    }
    load_file(path.toStdString());
}

void App::add_url() {
    QString text = QInputDialog::getText(this, "Enter the URL", "Enter the path to the file: ", QLineEdit::Normal);
    msg("Checking the remote file...");
    if (dl_mgr.check_url(QUrl::fromEncoded(text.toLocal8Bit()))) {
        auto *purl = new QListWidgetItem(text, ui->UrlsList);
        urls.push_back(purl);
        pending = true;
        emit updated();
        return;
    }
    msg("The URL you entered is not available/valid. You can try again?.. ");
}

void App::load_file(const std::string &path) {
    qDebug() << "load_file called";
    std::ifstream f(path);
    if (f) {
        std::stringstream ss;
        filepaths.push_back(new QListWidgetItem(QString::fromStdString(path), ui->FileList));
        ss << f.rdbuf();
        files.push_back(ss.str());
        qDebug() << "Added the file " << QString::fromStdString(path);
        emit updated();
    } else {
        msg(tr(FILEERRORMSG));
    }
}

void App::display_about() {
    QMessageBox::about(this, tr("About HostsToolkit"),
                       tr("HostsTools V%1\n"
                          "https://github.com/Nek-12/HostsToolkit\n"
                          "By Nek.12 \n"
                          "t.me/Nek_12\n")
                           .arg(VERSION));
}

void App::delete_selected_file() {
    auto *item = ui->FileList->currentItem();
    if (!item)
        return;
    if (item->text() == HOSTS) {
        sys_loaded = false;
        ui->LoadSystemHosts->setEnabled(true);
    }
    auto row = ui->FileList->currentRow();
    files.erase(files.begin() + row);
    filepaths.erase(filepaths.begin() + row);
    delete item;
}

void App::del_selected_list_entry() {
    delete ui->CustomEntriesList->currentItem();
}

void App::load_custom() {
    std::ifstream f(CONFIG_FNAME);
    while (f) {
        std::string s;
        std::getline(f, s);
        if (!s.empty()) {
            customlines.emplace_back(new QListWidgetItem(QString::fromStdString(s), ui->CustomEntriesList));
        }
    }
}
