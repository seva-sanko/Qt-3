#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QDate>
#include <QString>
#include <QRegularExpression>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Contact {
    QString firstName;
    QString lastName;
    QString middleName;
    QString address;
    QDate birthDate;
    QString email;
    QStringList phoneNumbers;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addButton_clicked();
    void on_deleteButton_clicked();
    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_searchEdit_textChanged(const QString &text);
    void on_tableWidget_itemChanged(QTableWidgetItem *item);
    void on_tableWidget_headerClicked(int column);

private:
    Ui::MainWindow *ui;
    QList<Contact> contacts;
    bool isImporting = false;
    QString errorslist;
    QVector<Contact> filteredContacts;

    void addContactToTable(const Contact &contact);
    bool validateContact(const Contact &contact);
    void saveToFile(const QString &filePath);
    void loadFromFile(const QString &filePath);
    bool isValidEmail(const QString &email);
    //bool isValidPhone(const QString &phone);
    bool isValidName(const QString &name);
    void resetHighlighting();

};

#endif // MAINWINDOW_H
