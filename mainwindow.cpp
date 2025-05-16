#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Вход");
    initDatabase();

    // Стилизация кнопок
    ui->AsStudent->setStyleSheet(
        "QPushButton {"
        "   border-radius: 10px;"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3e8e41; }"
        );

    ui->AsTutor->setStyleSheet(
        "QPushButton {"
        "   border-radius: 10px;"
        "   background-color: #2196F3;"
        "   color: white;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #0b7dda; }"
        "QPushButton:pressed { background-color: #0a68b4; }"
        );

    // Создаем stacked widget для переключения между видами
    QStackedWidget *stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // Создаем основной вид
    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Добавляем заголовок
    QLabel *titleLabel = new QLabel("Войти как:", mainWidget);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin-bottom: 7px;");
    mainLayout->addWidget(titleLabel);

    // Добавляем кнопки в основной вид
    mainLayout->addWidget(ui->AsStudent);
    mainLayout->addWidget(ui->AsTutor);
    mainLayout->setSpacing(20);

    // Настройка размеров и шрифта кнопок
    ui->AsStudent->setFixedSize(200, 50);
    ui->AsTutor->setFixedSize(200, 50);
    QFont font = ui->AsStudent->font();
    font.setPointSize(14);
    ui->AsStudent->setFont(font);
    ui->AsTutor->setFont(font);

    // Растягиваемые пространства
    mainLayout->addStretch();
    mainLayout->insertStretch(0);

    QWidget *studentWidget = new QWidget();
    QVBoxLayout *studentLayout = new QVBoxLayout(studentWidget);
    studentLayout->setContentsMargins(0, 0, 0, 50); // Отступ снизу 50px

    // Заголовок
    QLabel *studentTitle = new QLabel("Войти как студент", studentWidget);
    studentTitle->setStyleSheet("font-size: 20px; font-weight: bold; margin-bottom: 30px;");
    studentTitle->setAlignment(Qt::AlignCenter);
    studentLayout->addWidget(studentTitle, 0, Qt::AlignHCenter);

    // Контейнер для формы (центрирование)
    QWidget *formContainer = new QWidget(studentWidget);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setAlignment(Qt::AlignCenter);
    formContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //  Поля формы
    // Логин
    QLabel *loginLabel = new QLabel("Логин:", formContainer);
    loginLabel->setStyleSheet("font-size:14px;"
                              " font-weight: bold;");
    QLineEdit *loginEdit = new QLineEdit(formContainer);
    loginEdit->setPlaceholderText("Введите ваш логин");
    loginEdit->setStyleSheet("font-size:12px;"
                             " font-weight: bold;");
    loginEdit->setFixedSize(250, 35);
    loginEdit->setStyleSheet("QLineEdit { padding: 5px; border-radius: 5px; }");

    // Пароль
    QLabel *passwordLabel = new QLabel("Пароль:", formContainer);
    passwordLabel->setStyleSheet("font-size:14px;"
                                 " font-weight: bold;");
    QLineEdit *passwordEdit = new QLineEdit(formContainer);
    passwordEdit->setPlaceholderText("Введите ваш пароль");
    passwordEdit->setStyleSheet("font-size:12px;"
                                " font-weight: bold;");
    passwordEdit->setFixedSize(250, 35);
    passwordEdit->setStyleSheet("QLineEdit { padding: 5px; border-radius: 5px; }");

    // Кнопка входа
    QPushButton *loginButton = new QPushButton("Войти", formContainer);
    loginButton->setFixedSize(170, 50);
    loginButton->setStyleSheet(
        "QPushButton {"
        "   border-radius: 5px;"
        "   background-color: #4CAF50;"  // Зеленый цвет как у кнопки Студент
        "   color: white;"
        "font-size:20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3e8e41; }"
        );

    connect(loginButton, &QPushButton::clicked, this, [this, loginEdit, passwordEdit]() {
        QString login = loginEdit->text().trimmed();
        QString password = passwordEdit->text();

        if (login.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Заполните все поля");
            return;
        }

        if (checkStudentCredentials(login, password)) {
            QSqlQuery query;
            query.prepare("SELECT full_name, group_name FROM students WHERE login = :login");
            query.bindValue(":login", login);

            if (!query.exec()) {
                QMessageBox::warning(this, "Ошибка",
                                     "Ошибка базы данных:\n" + query.lastError().text());
                return;
            }

            if (query.next())
            {
                QString fullName = query.value("full_name").toString();
                QString group = query.value("group_name").toString();

                QSqlQuery idQuery;
                idQuery.prepare("SELECT id FROM students WHERE login = :login");
                idQuery.bindValue(":login", login);

                if (idQuery.exec() && idQuery.next())
                {
                    int studentId = idQuery.value(0).toInt();
                    StudentWindow *studentWindow = new StudentWindow(studentId, fullName, group);
                    studentWindow->show();
                    this->hide();

                    connect(studentWindow, &StudentWindow::destroyed, this, [this, loginEdit, passwordEdit]()
                            {
                                this->show();
                                loginEdit->clear();
                                passwordEdit->clear();
                            });
                }
            }
            else
            {
                QMessageBox::warning(this, "Ошибка",
                                     "Данные студента не найдены в базе");
            }
        }
        else
        {
            QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль");
        }
    });

    // Компоновка формы
    formLayout->addWidget(loginLabel, 0, Qt::AlignHCenter);
    formLayout->addWidget(loginEdit, 0, Qt::AlignHCenter);
    formLayout->addSpacing(15);
    formLayout->addWidget(passwordLabel, 0, Qt::AlignHCenter);
    formLayout->addWidget(passwordEdit, 0, Qt::AlignHCenter);
    formLayout->addSpacing(20);
    formLayout->addWidget(loginButton, 0, Qt::AlignHCenter);

    // Добавляем форму в основной layout
    studentLayout->addWidget(formContainer);
    studentLayout->addStretch();

    // Кнопка "Назад" (внизу экрана)
    QWidget *backButtonContainer = new QWidget(studentWidget);
    QHBoxLayout *backButtonLayout = new QHBoxLayout(backButtonContainer);
    backButtonLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *backFromStudent = new QPushButton("Назад", backButtonContainer);
    backFromStudent->setFixedSize(150, 50);
    backFromStudent->setFont(font);
    backFromStudent->setStyleSheet(
        "QPushButton {"
        "   border-radius: 10px;"
        "   background-color: #f44336;"
        "   color: white;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:pressed { background-color: #b71c1c; }"
        );

    backButtonLayout->addWidget(backFromStudent, 0, Qt::AlignHCenter);
    studentLayout->addWidget(backButtonContainer);

    studentLayout->addStretch();
    studentLayout->insertStretch(0);

    // Экран преподавателя
    QWidget *tutorWidget = new QWidget();
    QVBoxLayout *tutorLayout = new QVBoxLayout(tutorWidget);
    tutorLayout->setContentsMargins(0, 0, 0, 50); // Отступ снизу 50px

    // Заголовок
    QLabel *tutorTitle = new QLabel("Войти как преподаватель", tutorWidget);
    tutorTitle->setStyleSheet("font-size: 20px; font-weight: bold; margin-bottom: 30px;");
    tutorTitle->setAlignment(Qt::AlignCenter);
    tutorLayout->addWidget(tutorTitle, 0, Qt::AlignHCenter);

    // Контейнер для формы преподавателя
    QWidget *tutorFormContainer = new QWidget(tutorWidget);
    QVBoxLayout *tutorFormLayout = new QVBoxLayout(tutorFormContainer);
    tutorFormLayout->setAlignment(Qt::AlignCenter);
    tutorFormContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Поле для логина преподавателя
    QLabel *tutorLoginLabel = new QLabel("Логин:", tutorFormContainer);
    tutorLoginLabel->setStyleSheet("font-size:14px;"
                                   " font-weight: bold;");
    QLineEdit *tutorLoginEdit = new QLineEdit(tutorFormContainer);
    tutorLoginEdit->setPlaceholderText("Введите ваш логин");
    tutorLoginEdit->setFixedSize(250, 35);
    tutorLoginEdit->setStyleSheet("font-size:12px;"
                                  " font-weight: bold;");

    // Поле для пароля преподавателя
    QLabel *tutorPasswordLabel = new QLabel("Пароль:", tutorFormContainer);
    tutorPasswordLabel->setStyleSheet("font-size:14px;"
                                      " font-weight: bold;");
    QLineEdit *tutorPasswordEdit = new QLineEdit(tutorFormContainer);
    tutorPasswordEdit->setPlaceholderText("Введите ваш пароль");
    tutorPasswordEdit->setEchoMode(QLineEdit::Password);
    tutorPasswordEdit->setFixedSize(250, 35);
    tutorPasswordEdit->setStyleSheet("font-size:12px;"
                                     " font-weight: bold;");

    // Кнопка входа для преподавателя
    QPushButton *tutorLoginButton = new QPushButton("Войти", tutorFormContainer);
    tutorLoginButton->setFixedSize(170, 50);
    tutorLoginButton->setStyleSheet(
        "QPushButton {"
        "   border-radius: 5px;"
        "   background-color: #2196F3;"  // Синий цвет вместо зеленого
        "   color: white;"
        "font-size:20px;"
        " font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #0b7dda; }"
        );

    connect(tutorLoginButton, &QPushButton::clicked, this, [this, tutorLoginEdit, tutorPasswordEdit]() {
        QString login = tutorLoginEdit->text().trimmed();
        QString password = tutorPasswordEdit->text();

        if (login.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Заполните все поля");
            return;
        }

        if (checkTutorCredentials(login, password)) {
            QSqlQuery query;
            query.prepare("SELECT name, Department FROM tutors WHERE login_tutor = :login");
            query.bindValue(":login", login);

            if (!query.exec()) {
                QMessageBox::warning(this, "Ошибка",
                                     "Ошибка базы данных:\n" + query.lastError().text());
                return;
            }

            if (query.next()) {
                QString fullName = query.value("name").toString();
                QString depart = query.value("Department").toString();

                // Создаем окно студента с полным именем и группой
                TutorWindow *tutorWindow = new TutorWindow(fullName, depart);
                tutorWindow->show();
                this->hide();

                connect(tutorWindow, &TutorWindow::destroyed, this, [this, tutorLoginEdit, tutorPasswordEdit]() {
                    this->show();
                    tutorLoginEdit->clear();
                    tutorPasswordEdit->clear();
                });
            } else {
                QMessageBox::warning(this, "Ошибка",
                                     "Данные преподавателя не найдены в базе");
            }
        } else {
            QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль");
        }
    });


    // Добавляем элементы в форму преподавателя
    tutorFormLayout->addWidget(tutorLoginLabel, 0, Qt::AlignHCenter);
    tutorFormLayout->addWidget(tutorLoginEdit, 0, Qt::AlignHCenter);
    tutorFormLayout->addSpacing(15);
    tutorFormLayout->addWidget(tutorPasswordLabel, 0, Qt::AlignHCenter);
    tutorFormLayout->addWidget(tutorPasswordEdit, 0, Qt::AlignHCenter);
    tutorFormLayout->addSpacing(20);
    tutorFormLayout->addWidget(tutorLoginButton, 0, Qt::AlignHCenter);

    // Добавляем форму в основной layout
    tutorLayout->addWidget(tutorFormContainer);
    tutorLayout->addStretch();

    // Кнопка "Назад" для преподавателя
    QWidget *backButtonContainerTutor = new QWidget(tutorWidget);
    QHBoxLayout *backButtonLayoutTutor = new QHBoxLayout(backButtonContainerTutor);
    backButtonLayoutTutor->setContentsMargins(0, 0, 0, 0);

    QPushButton *backFromTutor = new QPushButton("Назад", backButtonContainerTutor);
    backFromTutor->setFixedSize(150, 50);
    backFromTutor->setFont(font);
    backFromTutor->setStyleSheet(
        "QPushButton {"
        "   border-radius: 10px;"
        "   background-color: #f44336;"
        "   color: white;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:pressed { background-color: #b71c1c; }"
        );

    backButtonLayoutTutor->addWidget(backFromTutor, 0, Qt::AlignHCenter);
    tutorLayout->addWidget(backButtonContainerTutor);

    tutorLayout->addStretch();
    tutorLayout->insertStretch(0);

    // Добавляем все виды в stacked widget
    stackedWidget->addWidget(mainWidget);    // Индекс 0 - главный экран
    stackedWidget->addWidget(studentWidget); // Индекс 1 - студент
    stackedWidget->addWidget(tutorWidget);   // Индекс 2 - преподаватель

    // Подключаем кнопки
    connect(ui->AsStudent, &QPushButton::clicked, [stackedWidget](){
        stackedWidget->setCurrentIndex(1); // Переключаем на вид студента
    });

    connect(ui->AsTutor, &QPushButton::clicked, [stackedWidget](){
        stackedWidget->setCurrentIndex(2); // Переключаем на вид преподавателя
    });

    connect(backFromStudent, &QPushButton::clicked, [stackedWidget](){
        stackedWidget->setCurrentIndex(0); // Возвращаемся к основному виду
    });

    connect(backFromTutor, &QPushButton::clicked, [stackedWidget](){
        stackedWidget->setCurrentIndex(0); // Возвращаемся к основному виду
    });

}

void MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("students.db"); // Используем существующий файл

    if (!db.open())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть базу данных:\n" + db.lastError().text());
        return;
    }

    // Просто проверяем, что таблица существует
    if ((!db.tables().contains("students")) or (!db.tables().contains("tutors")))
    {
        QMessageBox::critical(this, "Ошибка", "Некорректная структура базы данных");
        qApp->exit(1);
    }
}

bool MainWindow::checkStudentCredentials(const QString &login, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT password_hash FROM students WHERE login = :login");
    query.bindValue(":login", login);

    if (!query.exec() || !query.next())
    {
        return false; // Пользователь не найден
    }

    // Получаем хеш из базы данных
    QString storedHash = query.value(0).toString();

    // Хешируем введенный пароль для сравнения
    QString inputHash = QString(QCryptographicHash::hash(
                                    password.toUtf8(), QCryptographicHash::Sha256).toHex());

    return storedHash == inputHash;
}

bool MainWindow::checkTutorCredentials(const QString &login, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT password_tutor FROM tutors WHERE login_tutor = :login");
    query.bindValue(":login", login);

    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qDebug() << "Преподаватель с логином" << login << "не найден";
        return false;
    }

    // Получаем хеш пароля из базы данных
    QString storedHash = query.value(0).toString();

    // Хешируем введенный пароль для сравнения (SHA-256)
    QString inputHash = QString(QCryptographicHash::hash(
                                    password.toUtf8(),
                                    QCryptographicHash::Sha256
                                    ).toHex());

    // Сравниваем хеши
    bool isMatch = (storedHash.compare(inputHash, Qt::CaseInsensitive) == 0);
    if (!isMatch)
    {
        qDebug() << "Неверный пароль для логина" << login;
    }
    return isMatch;
}

MainWindow::~MainWindow()
{
    delete ui;
    db.close(); // Закрываем соединение с БД
}
