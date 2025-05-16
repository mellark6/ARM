#include "studentwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QGroupBox>
#include <QSqlQuery>
#include <QMessageBox>
#include <QHeaderView>
#include <QObject>
#include <QDebug>
#include <QSqlError>
#include <QProgressDialog>
#include <QtNetwork>



StudentWindow::StudentWindow(int studentId, const QString &fullName,
                             const QString &group, QWidget *parent)
    : QMainWindow(parent), studentId(studentId), studentName(fullName),
    studentGroup(group)
{
    db = QSqlDatabase::database(); // Получаем существующее соединение
    setupUi();
    setWindowTitle("Личный кабинет студента");
    resize(1000, 700);

    // Проверяем текущее занятие при запуске
    QTimer::singleShot(0, this, &StudentWindow::checkCurrentClass);
}

void StudentWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 1. Информация о студенте
    QLabel *infoLabel = new QLabel(
        QString("Студент: %1\nГруппа: %2").arg(studentName).arg(studentGroup));
    infoLabel->setStyleSheet("font-size: 16px; font-weight: bold;");

    // 2. Блок текущего занятия
    QGroupBox *currentClassBox = new QGroupBox("Текущее занятие");
    QVBoxLayout *currentClassLayout = new QVBoxLayout(currentClassBox);

    currentClassLabel = new QLabel("Проверяем расписание...");
    currentClassLabel->setStyleSheet("font-size: 14px;");

    attendanceButton = new QPushButton("Отметиться на занятии");
    attendanceButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   padding: 10px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:disabled { background-color: #cccccc; }"
        );
    attendanceButton->setEnabled(false);

    connect(attendanceButton, &QPushButton::clicked,
            this, &StudentWindow::markAttendance);

    currentClassLayout->addWidget(currentClassLabel);
    currentClassLayout->addWidget(attendanceButton, 0, Qt::AlignCenter);

    // 3. Календарь и расписание
    calendar = new QCalendarWidget();
    calendar->setMaximumDate(QDate::currentDate());
    connect(calendar, &QCalendarWidget::clicked,
            this, &StudentWindow::loadSchedule);

    scheduleTable = new QTableWidget();
    scheduleTable->setColumnCount(5);
    scheduleTable->setHorizontalHeaderLabels(
        {"Время", "Предмет", "Аудитория", "Статус", ""});
    scheduleTable->horizontalHeader()->setStretchLastSection(true);

    // 4. Кнопка выхода
    QPushButton *logoutButton = new QPushButton("Выйти", centralWidget);
    logoutButton->setFixedSize(150, 40);
    logoutButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #d32f2f; }"
        );
    connect(logoutButton, &QPushButton::clicked, this, &StudentWindow::close);

    // Компоновка
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(currentClassBox);
    mainLayout->addWidget(calendar);
    mainLayout->addWidget(scheduleTable);
    mainLayout->addWidget(logoutButton, 0, Qt::AlignRight);

    setCentralWidget(centralWidget);

    // Загружаем расписание на сегодня
    calendar->setSelectedDate(QDate::currentDate());
    loadSchedule(QDate::currentDate());
}

void StudentWindow::checkCurrentClass()
{
    QDateTime now = QDateTime::currentDateTime();
    QString currentDate = now.date().toString("yyyy-MM-dd"); // Формат даты в вашей БД
    QString currentTime = now.time().toString("HH:mm");
    QString currentDay = now.toString("dddd");

    qDebug() << "Checking class for:";
    qDebug() << "Date:" << currentDate;
    qDebug() << "Time:" << currentTime;
    qDebug() << "Day:" << currentDay;
    qDebug() << "Group:" << studentGroup;

    QSqlQuery query;
    query.prepare(
        "SELECT id, subject_name, room, start_time, end_time "
        "FROM schedule "
        "WHERE group_name = :group "
        "AND date = :date "
        "AND start_time <= :time AND end_time >= :time");
    query.bindValue(":group", studentGroup);
    query.bindValue(":date", currentDate);
    query.bindValue(":time", currentTime);

    if (!query.exec())
    {
        qDebug() << "Query error:" << query.lastError().text();
        currentClassLabel->setText("Ошибка проверки расписания\n" + query.lastError().text());
        return;
    }

    if (query.next())
    {
        int scheduleId = query.value(0).toInt();
        QString subject = query.value(1).toString();
        QString room = query.value(2).toString();
        QString timeRange = query.value(3).toString() + "-" + query.value(4).toString();

        currentClassLabel->setText(
            QString("Сейчас: %1\nАудитория: %2\nВремя: %3")
                .arg(subject).arg(room).arg(timeRange));

        // Проверка отметки
        QSqlQuery checkAttendance;
        checkAttendance.prepare(
            "SELECT timestamp FROM attendance "
            "WHERE student_id = :student_id AND schedule_id = :schedule_id");
        checkAttendance.bindValue(":student_id", studentId);
        checkAttendance.bindValue(":schedule_id", scheduleId);

        if (checkAttendance.exec() && checkAttendance.next())
        {
            QDateTime timestamp = checkAttendance.value(0).toDateTime();
            attendanceButton->setText("Отмечен " + timestamp.toString("HH:mm"));
            attendanceButton->setEnabled(false);
        }
        else
        {
            attendanceButton->setText("Отметиться на занятии");
            attendanceButton->setEnabled(true);
        }
    }
    else
    {
        qDebug() << "No current class found";
        currentClassLabel->setText("В данный момент занятий нет");
        attendanceButton->setEnabled(false);
    }
}

void StudentWindow::saveAttendance(int scheduleId, const QDateTime &timestamp)
{
    QSqlQuery query;
    query.prepare(
        "INSERT INTO attendance (student_id, schedule_id, timestamp) "
        "VALUES (:student_id, :schedule_id, :timestamp)");
    query.bindValue(":student_id", studentId);
    query.bindValue(":schedule_id", scheduleId);
    query.bindValue(":timestamp", timestamp);

    if (query.exec())
    {
        QMessageBox::information(this, "Успех", "Вы успешно отметились");
        attendanceButton->setEnabled(false);
        attendanceButton->setText("Отметка зарегистрирована");
    }
    else
    {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось сохранить отметку: " + query.lastError().text());
    }
}

// Получает SSID текущей wi fi сети
QString StudentWindow::getCurrentWifiSSID()
{
    QProcess process;
    process.setProgram("cmd");
    process.setArguments({"/C", "netsh wlan show interfaces | findstr SSID"});
    process.start();
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    qDebug() << "Wi-Fi output:" << output;  // Для отладки

    if (output.isEmpty())
    {
        qDebug() << "Wi-Fi не найден или netsh не работает";
        return "";
    }

    // Парсинг SSID
    QRegularExpression regex("SSID\\s*:\\s*(.+)");
    QRegularExpressionMatch match = regex.match(output);

    if (match.hasMatch())
    {
        QString ssid = match.captured(1).trimmed();
        qDebug() << "Найден SSID:" << ssid;
        return ssid;
    }

    return "";
}

// Проверяет, разрешена ли сеть
bool StudentWindow::checkAllowedWifiSSID(const QString &ssid)
{
    // Пример: список разрешенных сетей (лучше хранить в БД)
    QStringList allowedNetworks = {"HUAWEI-4PyQ","ITAS"};

    return allowedNetworks.contains(ssid, Qt::CaseInsensitive);
}

void StudentWindow::markAttendance()
{
    // Получаем текущее время и дату
    QDateTime now = QDateTime::currentDateTime();
    QString currentDate = now.date().toString("yyyy-MM-dd");
    QString currentTime = now.time().toString("HH:mm");

    // Находим текущее занятие
    QSqlQuery query;
    query.prepare(
        "SELECT id, room FROM schedule "
        "WHERE group_name = :group "
        "AND date = :date "
        "AND start_time <= :time AND end_time >= :time");
    query.bindValue(":group", studentGroup);
    query.bindValue(":date", currentDate);
    query.bindValue(":time", currentTime);

    if (!query.exec() || !query.next())
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось определить текущее занятие");
        return;
    }

    int scheduleId = query.value(0).toInt();
    QString room = query.value(1).toString();

    // SSID текущей сети
    QString currentSSID = getCurrentWifiSSID();

    if (currentSSID.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось определить Wi-Fi сеть");
        return;
    }

    // Проверяем, что подключены к университетской сети
    bool isInCampus = checkAllowedWifiSSID(currentSSID);

    if (isInCampus)
    {
        saveAttendance(scheduleId, now);
    }
    else
    {
        QMessageBox::warning(this, "Ошибка",
                             "Вы не подключены к университетской Wi-Fi сети.\n"
                             "Текущая сеть: " + currentSSID);
    }
}

void StudentWindow::loadSchedule(const QDate &date)
{
    QString dayOfWeek = date.toString("dddd");

    QSqlQuery query;
    query.prepare(
        "SELECT start_time, end_time, subject_name, room, id "
        "FROM schedule "
        "WHERE day_of_week = :day AND group_name = :group "
        "ORDER BY start_time");
    query.bindValue(":day", dayOfWeek);
    query.bindValue(":group", studentGroup);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось загрузить расписание" );
        return;
    }

    scheduleTable->setRowCount(0);

    while (query.next())
    {
        int row = scheduleTable->rowCount();
        scheduleTable->insertRow(row);

        QString time = query.value(0).toString() + " - " + query.value(1).toString();
        QString subject = query.value(2).toString();
        QString room = query.value(3).toString();
        int scheduleId = query.value(4).toInt();

        scheduleTable->setItem(row, 0, new QTableWidgetItem(time));
        scheduleTable->setItem(row, 1, new QTableWidgetItem(subject));
        scheduleTable->setItem(row, 2, new QTableWidgetItem(room));

        // Проверяем статус посещения для конкретной даты
        QSqlQuery attendanceQuery;
        attendanceQuery.prepare(
            "SELECT timestamp FROM attendance "
            "WHERE student_id = :student_id AND schedule_id = :schedule_id "
            "AND DATE(timestamp) = :date");
        attendanceQuery.bindValue(":student_id", studentId);
        attendanceQuery.bindValue(":schedule_id", scheduleId);
        attendanceQuery.bindValue(":date", date);

        if (attendanceQuery.exec() && attendanceQuery.next()) {
            QDateTime timestamp = attendanceQuery.value(0).toDateTime();
            scheduleTable->setItem(row, 3,
                                   new QTableWidgetItem("Присутствовал " + timestamp.toString("dd.MM.yyyy HH:mm")));
        }
        else
        {
            scheduleTable->setItem(row, 3, new QTableWidgetItem("Не отмечен"));
        }
    }
    scheduleTable->resizeColumnsToContents();
}
