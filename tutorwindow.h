#ifndef TUTORWINDOW_H
#define TUTORWINDOW_H

#include <QMainWindow>
#include <QCalendarWidget>
#include <QTableWidget>
#include <QSqlDatabase>

class TutorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit TutorWindow(const QString &fullName, const QString &depart, QWidget *parent = nullptr);

private slots:
    void loadSchedule(const QDate &date);
    void loadAttendance(int scheduleId);

private:
    void setupUi();
    QString tutorName;
    QString tutorDepartment;
    QSqlDatabase db;
    //UI элементы
    QCalendarWidget *calendar;
    QTableWidget *scheduleTable;
    QTableWidget *attendanceTable;
};

#endif // TUTORWINDOW_H
