#include "tutorwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

TutorWindow::TutorWindow(const QString &fullName, const QString &depart, QWidget *parent)
    : QMainWindow(parent), tutorName(fullName), tutorDepartment(depart)
{
    db = QSqlDatabase::database(); // Получаем соединение
    setupUi();
    setWindowTitle("Личный кабинет преподавателя");
    resize(1000, 700); // размер окна
}

void TutorWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 1. Заголовок и информация о преподавателе
    QLabel *titleLabel = new QLabel("Личный кабинет преподавателя", centralWidget);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *infoLabel = new QLabel(
        QString("ФИО: %1\nКафедра: %2").arg(tutorName).arg(tutorDepartment),
        centralWidget);
    infoLabel->setStyleSheet("font-size: 16px; margin-bottom: 20px;");
    infoLabel->setAlignment(Qt::AlignCenter);

    // 2. Календарь
    calendar = new QCalendarWidget(centralWidget);
    calendar->setMaximumDate(QDate::currentDate());
    connect(calendar, &QCalendarWidget::clicked, this, &TutorWindow::loadSchedule);

    // 3. Группа с расписанием
    QGroupBox *scheduleGroup = new QGroupBox("Расписание занятий", centralWidget);
    QVBoxLayout *scheduleLayout = new QVBoxLayout(scheduleGroup);

    scheduleTable = new QTableWidget(centralWidget);
    scheduleTable->setColumnCount(4);
    scheduleTable->setHorizontalHeaderLabels({"Время", "Предмет", "Группа", "Аудитория"});
    scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    scheduleTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(scheduleTable, &QTableWidget::cellClicked, [this](int row) {
        int scheduleId = scheduleTable->item(row, 0)->data(Qt::UserRole).toInt();
        loadAttendance(scheduleId);
    });

    scheduleLayout->addWidget(scheduleTable);

    // 4. Группа с посещаемостью
    QGroupBox *attendanceGroup = new QGroupBox("Посещаемость студентов", centralWidget);
    QVBoxLayout *attendanceLayout = new QVBoxLayout(attendanceGroup);

    attendanceTable = new QTableWidget(centralWidget);
    attendanceTable->setColumnCount(3);
    attendanceTable->setHorizontalHeaderLabels({"Студент", "Группа", "Время отметки"});
    attendanceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    attendanceLayout->addWidget(attendanceTable);

    // 5. Кнопка выхода
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

    connect(logoutButton, &QPushButton::clicked, this, &TutorWindow::close);

    // Компоновка всех элементов
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(calendar);
    mainLayout->addWidget(scheduleGroup);
    mainLayout->addWidget(attendanceGroup);
    mainLayout->addWidget(logoutButton, 0, Qt::AlignRight);

    setCentralWidget(centralWidget);

    // Загружаем расписание на текущую дату
    loadSchedule(QDate::currentDate());
}

void TutorWindow::loadSchedule(const QDate &date)
{
    QString dateStr = date.toString("yyyy-MM-dd");
    qDebug() << "Loading schedule for date:" << dateStr;
    qDebug() << "Tutor department:" << tutorDepartment;

    // Упрощенный запрос без связи с таблицей tutors
    QSqlQuery query;
    query.prepare(
        "SELECT id, start_time, end_time, subject_name, group_name, room "
        "FROM schedule "
        "WHERE date = :date "
        "ORDER BY start_time");

    query.bindValue(":date", dateStr);

    if (!query.exec()) {
        QString error = "Не удалось загрузить расписание:\n" + query.lastError().text();
        qDebug() << error;
        QMessageBox::warning(this, "Ошибка", error);
        return;
    }

    scheduleTable->setRowCount(0);

    while (query.next()) {
        int row = scheduleTable->rowCount();
        scheduleTable->insertRow(row);

        int scheduleId = query.value(0).toInt();
        QString time = query.value(1).toString() + " - " + query.value(2).toString();
        QString subject = query.value(3).toString();
        QString group = query.value(4).toString();
        QString room = query.value(5).toString();

        QTableWidgetItem *idItem = new QTableWidgetItem(time);
        idItem->setData(Qt::UserRole, scheduleId);

        scheduleTable->setItem(row, 0, idItem);
        scheduleTable->setItem(row, 1, new QTableWidgetItem(subject));
        scheduleTable->setItem(row, 2, new QTableWidgetItem(group));
        scheduleTable->setItem(row, 3, new QTableWidgetItem(room));
    }

    if (scheduleTable->rowCount() > 0) {
        int firstScheduleId = scheduleTable->item(0, 0)->data(Qt::UserRole).toInt();
        loadAttendance(firstScheduleId);
    } else {
        attendanceTable->setRowCount(0);
        qDebug() << "No classes found for" << dateStr;
    }
}

void TutorWindow::loadAttendance(int scheduleId)
{
    QSqlQuery query;
    query.prepare(
        "SELECT s.full_name, s.group_name, a.timestamp "
        "FROM attendance a "
        "JOIN students s ON a.student_id = s.id "
        "WHERE a.schedule_id = :schedule_id "
        "ORDER BY a.timestamp");
    query.bindValue(":schedule_id", scheduleId);

    if (!query.exec())
    {
        QMessageBox::warning(this, "Ошибка",
        "Не удалось загрузить посещаемость: " + query.lastError().text());
        return;
    }

    attendanceTable->setRowCount(0);

    while (query.next()) {
        int row = attendanceTable->rowCount();
        attendanceTable->insertRow(row);

        QString studentName = query.value(0).toString();
        QString group = query.value(1).toString();
        QString timestamp = query.value(2).toDateTime().toString("dd.MM.yyyy HH:mm");

        attendanceTable->setItem(row, 0, new QTableWidgetItem(studentName));
        attendanceTable->setItem(row, 1, new QTableWidgetItem(group));
        attendanceTable->setItem(row, 2, new QTableWidgetItem(timestamp));
    }
}
