#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include <QCalendarWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QSqlDatabase>
#include <QProgressDialog>

class StudentWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit StudentWindow(int studentId, const QString &fullName,
                           const QString &group, QWidget *parent = nullptr);

private slots:
    void checkCurrentClass();
    void markAttendance();
    void loadSchedule(const QDate &date);

private:
    void setupUi();
    void updateCurrentClassInfo();
    int studentId;
    QString studentName;
    QString studentGroup;
    QSqlDatabase db;
    QLabel *currentClassLabel;
    QPushButton *attendanceButton;
    QTableWidget *scheduleTable;
    QCalendarWidget *calendar;
    void saveAttendance(int scheduleId, const QDateTime &timestamp);
    QString getCurrentWifiSSID();  // Получает SSID текущей Wi-Fi сети
    bool checkAllowedWifiSSID(const QString &ssid);  // Проверяет, разрешена ли сеть
};

#endif // STUDENTWINDOW_H
