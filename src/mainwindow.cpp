




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
