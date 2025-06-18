#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileDialog>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск...");
    ui->verticalLayout->addWidget(searchEdit);
    ui->birthDateEdit->setMaximumDate(QDate::currentDate());

    ui->tableWidget->setColumnWidth(6, 120);

    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::on_tableWidget_itemChanged);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::on_searchEdit_textChanged);
    connect(ui->tableWidget->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::on_tableWidget_headerClicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_tableWidget_headerClicked(int column){
    ui->tableWidget->sortByColumn(column, Qt::AscendingOrder);
}

QStringList parsePhoneNumbers(const QString &input) {
    QStringList numbers = input.split(",", QString::SkipEmptyParts);
    QStringList validNumbers;

    QRegularExpression phoneRegex(R"(^\+?\s?\d{1,3}[\s-]?(\(\d+\))?[\s-]?\d+([\s-]?\d+)*$)");

    for (const QString &number : numbers) {
        QString trimmedNumber = number.trimmed();
        if (phoneRegex.match(trimmedNumber).hasMatch()) {
            QString sanitizedNumber = trimmedNumber.remove(QRegularExpression(R"([^\d+])"));
            validNumbers.append(sanitizedNumber);
        } else {
            qDebug() << "Неверный номер:" << trimmedNumber;
        }
    }
    return validNumbers;
}

void MainWindow::addContactToTable(const Contact &contact) {
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(contact.firstName));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(contact.lastName));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(contact.middleName));
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(contact.address));
    ui->tableWidget->setItem(row, 4, new QTableWidgetItem(contact.birthDate.toString("dd.MM.yyyy")));
    ui->tableWidget->setItem(row, 5, new QTableWidgetItem(contact.email));

    QTableWidgetItem *phoneItem = new QTableWidgetItem(contact.phoneNumbers.join(", "));
    phoneItem->setFlags(phoneItem->flags() | Qt::ItemIsEditable);
    ui->tableWidget->setItem(row, 6, phoneItem);

    phoneItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    phoneItem->setText(contact.phoneNumbers.join("\n"));

    ui->tableWidget->resizeRowToContents(row);

    if(!isImporting){
        QMessageBox::information(this, "Успех", "Контакт добавлен.");
    }
}

bool MainWindow::validateContact(const Contact &contact) {
    if (!isValidName(contact.firstName) || !isValidName(contact.lastName) || !isValidName(contact.middleName)) {
        QMessageBox::warning(this, "Ошибка", "Имя, фамилия или отчество некорректны!");
        return false;
    }

    if (!isValidEmail(contact.email)) {
        QMessageBox::warning(this, "Ошибка", "Некорректный email!");
        return false;
    }


    if (contact.phoneNumbers.isEmpty()) {
           QMessageBox::warning(this, "Ошибка", "Все номера телефонов некорректны!");
           return false;
    }


    return true;
}

bool MainWindow::isValidName(const QString &name) {
    QRegularExpression regex(R"(^[A-ZА-ЯЁ][A-Za-zА-Яа-яёЁ\- ]*[a-zа-яё]$)");
    return regex.match(name.trimmed()).hasMatch();
}

bool MainWindow::isValidEmail(const QString &email) {
    QRegularExpression regex(R"(^[a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex.match(email.trimmed()).hasMatch();
}

void MainWindow::on_addButton_clicked() {
    Contact contact;
    contact.firstName = ui->firstNameEdit->text().trimmed();
    contact.lastName = ui->lastNameEdit->text().trimmed();
    contact.middleName = ui->middleNameEdit->text().trimmed();
    contact.address = ui->addressEdit->text().trimmed();
    contact.birthDate = ui->birthDateEdit->date();
    contact.email = ui->emailEdit->text().trimmed();
    //contact.phoneNumbers = ui->phoneEdit->text().split(",", QString::SkipEmptyParts);

     QString phoneInput = ui->phoneEdit->text();
     contact.phoneNumbers = parsePhoneNumbers(phoneInput);

     if (contact.phoneNumbers.isEmpty()) {
         QMessageBox::warning(this, "Ошибка ввода", "Все номера телефонов некорректны!");
         return;
     }
     /*
     ui->firstNameEdit->clear();
     ui->lastNameEdit->clear();
     ui->middleNameEdit->clear();
     ui->addressEdit->clear();
     ui->birthDateEdit->setDate(QDate::currentDate());
     ui->emailEdit->clear();
     ui->phoneEdit->clear();
     */

     if (validateContact(contact)) {
         contacts.append(contact);
         addContactToTable(contact);
     }
}

void MainWindow::on_deleteButton_clicked() {
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        ui->tableWidget->removeRow(row);
        contacts.removeAt(row);
    }
}

void MainWindow::on_saveButton_clicked() {
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить файл", "", "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        saveToFile(filePath);
    }
}

void MainWindow::on_loadButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть файл", "", "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        loadFromFile(filePath);
    }
}

void MainWindow::saveToFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи!");
        return;
    }

    QTextStream out(&file);

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        if (!ui->tableWidget->isRowHidden(i)) {
            QString firstName = ui->tableWidget->item(i, 0)->text();
            QString lastName = ui->tableWidget->item(i, 1)->text();
            QString middleName = ui->tableWidget->item(i, 2)->text();
            QString address = ui->tableWidget->item(i, 3)->text();
            QString birthDate = ui->tableWidget->item(i, 4)->text();
            QString email = ui->tableWidget->item(i, 5)->text();
            QString phoneNumbersString = ui->tableWidget->item(i, 6)->text();

            QStringList phoneNumbers = phoneNumbersString.split(";");
            QStringList sanitizedPhoneNumbers;
            for (QString &number : phoneNumbers) {
                number = number.trimmed()
                               .remove(QRegularExpression(R"([^\d+\s-])"))
                               .replace(" ", "")
                               .replace("-", "");
                sanitizedPhoneNumbers.append(number);
            }

            QString sanitizedPhoneNumbersString = sanitizedPhoneNumbers.join(";");

            out << firstName << ","
                << lastName << ","
                << middleName << ","
                << address << ","
                << birthDate << ","
                << email << ","
                << sanitizedPhoneNumbersString << "\n";
        }
    }

    file.close();
}

void MainWindow::loadFromFile(const QString &filePath) {
    isImporting = true;

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        isImporting = false;
        return;
    }

    contacts.clear();
    ui->tableWidget->setRowCount(0);

    QTextStream in(&file);
    QString report;
    int addedCount = 0;
    int lineNumber = 0;

    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() < 7) {
            //errors.append(QString("%1. Ошибка: Неполное количество данных в строке.").arg(lineNumber));
            QMessageBox::warning(this, "Ошибка", "Неполное количество данных в строке.");
            continue;
        }

        Contact contact;
        contact.firstName = parts[0];
        contact.lastName = parts[1];
        contact.middleName = parts[2];
        contact.address = parts[3];
        contact.birthDate = QDate::fromString(parts[4], "dd.MM.yyyy");
        contact.email = parts[5];
        contact.phoneNumbers = parts[6].split(";");


        contacts.append(contact);
        addContactToTable(contact);
        addedCount++;
    }

    report = QString("Импорт завершён.\nДобавлено контактов: %1\n\n").arg(addedCount) + report + errorslist;
    QMessageBox::information(this, "Импорт завершён.", report.trimmed());
    errorslist = "";
    isImporting = false;
}

void MainWindow::on_searchEdit_textChanged(const QString &text) {
    filteredContacts.clear();
    int matchCount = 0;
    int totalCount = ui->tableWidget->rowCount() * ui->tableWidget->columnCount();

    if (text.isEmpty()) {
        resetHighlighting();
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        for (int j = 0; j < ui->tableWidget->columnCount(); ++j) {
            QTableWidgetItem *item = ui->tableWidget->item(i, j);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                ++matchCount;

                if (matchCount < totalCount) {
                    item->setTextColor(Qt::green);
                }
            } else {
                if (item) {
                    item->setTextColor(Qt::black);
                }
            }
        }
    }


    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        bool rowHasMatch = false;
        for (int j = 0; j < ui->tableWidget->columnCount(); ++j) {
            if (ui->tableWidget->item(i, j)->textColor() == Qt::green) {
                rowHasMatch = true;
                break;
            }
        }
        ui->tableWidget->setRowHidden(i, !rowHasMatch);
    }

    if (matchCount == totalCount) {
        resetHighlighting();
    }

    if (matchCount < totalCount) {
        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            bool matches = false;
            for (int j = 0; j < ui->tableWidget->columnCount(); ++j) {
                if (ui->tableWidget->item(i, j)->textColor() == Qt::green) {
                    matches = true;
                    break;
                }
            }

            if (matches) {
                Contact contact;
                contact.firstName = ui->tableWidget->item(i, 0)->text();
                contact.lastName = ui->tableWidget->item(i, 1)->text();
                contact.middleName = ui->tableWidget->item(i, 2)->text();
                contact.address = ui->tableWidget->item(i, 3)->text();
                contact.birthDate = QDate::fromString(ui->tableWidget->item(i, 4)->text(), "dd.MM.yyyy");
                contact.email = ui->tableWidget->item(i, 5)->text();
                contact.phoneNumbers = ui->tableWidget->item(i, 6)->text().split(";");

                filteredContacts.append(contact);
            }
        }
    }
}

void MainWindow::resetHighlighting() {
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        for (int j = 0; j < ui->tableWidget->columnCount(); ++j) {
            QTableWidgetItem *item = ui->tableWidget->item(i, j);
            if (item) {
                item->setTextColor(Qt::black);
            }
        }
    }
}

void MainWindow::on_tableWidget_itemChanged(QTableWidgetItem *item) {
    int row = item->row();
    int column = item->column();

    Contact &contact = contacts[row];

    if (column == 0) {
        QString firstName = item->text().trimmed();
        if (!isValidName(firstName)) {
            if(isImporting){
                errorslist += "Некорректное имя!\n";
            }
            else{
                QMessageBox::warning(this, "Ошибка", "Некорректное имя!");
            }
            item->setText(contact.firstName);
            return;
        }
        contact.firstName = firstName;
    } else if (column == 1) {
        QString lastName = item->text().trimmed();
        if (!isValidName(lastName)) {
            if(isImporting){
                errorslist += "Некорректная фамилия!\n";
            }
            else{
                QMessageBox::warning(this, "Ошибка", "Некорректная фамилия!");
            }
            item->setText(contact.lastName);
            return;
        }
        contact.lastName = lastName;
    } else if (column == 2) {
        QString middleName = item->text().trimmed();
        if (!isValidName(middleName)) {
            if(isImporting){
                errorslist += "Некорректное отчество!\n";
            }
            else{
                QMessageBox::warning(this, "Ошибка", "Некорректное отчество!");
            }
            item->setText(contact.middleName);
            return;
        }
        contact.middleName = middleName;
    } else if (column == 3) {
        contact.address = item->text().trimmed();
    } else if (column == 4) {  // Birth Date
        QDate birthDate = QDate::fromString(item->text(), "dd.MM.yyyy");
        if (!birthDate.isValid()) {
            if(isImporting){
                errorslist += "Некорректная дата рождения!\n";
            }
            else{
                QMessageBox::warning(this, "Ошибка", "Некорректная дата рождения!");
            }
            item->setText(contact.birthDate.toString("dd.MM.yyyy"));
            return;
        }
        contact.birthDate = birthDate;
    } else if (column == 5) {
        QString email = item->text().trimmed();
        if (!isValidEmail(email)) {
            if(isImporting){
                errorslist += "Некорректный email!\n";
            }
            else{
                QMessageBox::warning(this, "Ошибка", "Некорректный email!");
            }
            item->setText(contact.email);
            return;
        }
        contact.email = email;
    } else if (column == 6) {  // Phone Numbers
        QString phoneNumbersText = item->text().trimmed();
        QStringList phoneNumbers = parsePhoneNumbers(phoneNumbersText);
        if (phoneNumbers.isEmpty()) {
            if(isImporting){
                errorslist += "Некорректный номер телефона!\n";
            }
            else{
                //QMessageBox::warning(this, "Ошибка", "Некорректный номер телефона!");
            }
            item->setText(contact.phoneNumbers.join(", "));
            return;
        }
        contact.phoneNumbers = phoneNumbers;
    }
}
