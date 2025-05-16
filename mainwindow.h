#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <studentwindow.h>
#include <tutorwindow.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    QWidget *mainWidget;
    QWidget *studentWidget;
    QSqlDatabase db;
    void initDatabase();
    bool checkStudentCredentials(const QString &login, const QString &password);
    bool checkTutorCredentials(const QString &login, const QString &password);
};
#endif // MAINWINDOW_H

