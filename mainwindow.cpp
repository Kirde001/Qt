#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->addTaskButton,&QPushButton::clicked,this,&MainWindow::on_addTaskButton_clicked);
    connect(ui->detailsTaskButton,&QPushButton::clicked,this,&MainWindow::on_detailsTaskButton_clicked);
    connect(ui->deleteTaskButton,&QPushButton::clicked,this,&MainWindow::on_deleteTaskButton_clicked);
    loadTasks();
}

MainWindow::~MainWindow()
{
    saveTasks();
    delete ui;
}

void MainWindow::on_addTaskButton_clicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить задачу");
    QVBoxLayout layout(&dialog);

    QLineEdit titleEdit;
    titleEdit.setPlaceholderText("Название задачи");
    layout.addWidget(&titleEdit);

    QTextEdit descriptionEdit;
    descriptionEdit.setPlaceholderText("Описание задачи");
    layout.addWidget(&descriptionEdit);

    QDateEdit dateEdit;
    dateEdit.setCalendarPopup(true);
    layout.addWidget(&dateEdit);

    QPushButton addButton("Добавить");
    layout.addWidget(&addButton);

    connect(&addButton, &QPushButton::clicked, [&]() {
       QString title = titleEdit.text().trimmed();
       QString description = descriptionEdit.toPlainText().trimmed();
       QDate date = dateEdit.date();
       QListWidgetItem *newItem = new QListWidgetItem(title);
       if (title.isEmpty()) {
           QMessageBox::warning(this, "Ошибка", "Название задачи не может быть пустым.");
           return;
       }
       newItem->setData(Qt::UserRole, description);
       newItem->setData(Qt::UserRole + 1, date);
       ui->tasksListWidget->addItem(newItem);
       dialog.accept();
    });

    dialog.exec();
}


void MainWindow::on_detailsTaskButton_clicked()
{
    QListWidgetItem *item = ui->tasksListWidget->currentItem();
    if (item) {
       QString taskDetails = item->text();
       QString description = item->data(Qt::UserRole).toString();
       QDate taskDate = item->data(Qt::UserRole + 1).toDate();
       QString details = QString("<b>Задача:</b> %1\n<b>Описание:</b> %2\n<b>Дата выполнения:</b> %3")
        .arg(taskDetails)
        .arg(description)
        .arg(taskDate.toString("dd.MM.yyyy"));
       QMessageBox::information(this, "Детали задачи", details);
    } else {
       QMessageBox::warning(this, "Просмотр задачи", "Выберите задачу для просмотра.");
    }
}

void MainWindow::on_deleteTaskButton_clicked()
{
    QListWidgetItem *item = ui->tasksListWidget->currentItem();
    if (item) {
       QMessageBox msgBox(this);
       msgBox.setWindowTitle("Удаление задачи");
       msgBox.setText("Вы уверены, что хотите удалить задачу?");
       QPushButton* yesButton = msgBox.addButton(QMessageBox::Yes);
       QPushButton* noButton = msgBox.addButton(QMessageBox::No);
       yesButton->setText("Да");
       noButton->setText("Нет");
       msgBox.exec();
       if (msgBox.clickedButton() == yesButton) {
        delete item;
       }
    } else {
       QMessageBox::warning(this, "Удаление задачи", "Выберите задачу для удаления.");
    }
}

void MainWindow::saveTasks()
{
    QSettings settings("PersonalOrganizer");
    settings.beginGroup("Tasks");
    settings.remove("");
    for (int i = 0; i < ui->tasksListWidget->count(); ++i) {
        QListWidgetItem *item = ui->tasksListWidget->item(i);
        QString title = item->text();
        QString description = item->data(Qt::UserRole).toString();
        QDate date = item->data(Qt::UserRole + 1).toDate();
        settings.setValue(QString("task%1/title").arg(i), title);
        settings.setValue(QString("task%1/description").arg(i), description);
        settings.setValue(QString("task%1/date").arg(i), date);
    }
    settings.endGroup();
}

void MainWindow::loadTasks()
{
    QSettings settings("PersonalOrganizer");
    settings.beginGroup("Tasks");
    QStringList keys = settings.childKeys();
    for (const QString &key : keys) {
        if (key.startsWith("task")) {
            QString title = settings.value(key + "/title").toString();
            QString description = settings.value(key + "/description").toString();
            QDate date = settings.value(key + "/date").toDate();
            QListWidgetItem *newItem = new QListWidgetItem(title);
            newItem->setData(Qt::UserRole, description);
            newItem->setData(Qt::UserRole + 1, date);
            ui->tasksListWidget->addItem(newItem);
        }
    }
    settings.endGroup();
}

/*
Эта функция отвечает за сохранение списка задач из пользовательского интерфейса в 
постоянное хранилище (чтобы их можно было загрузить при следующем запуске приложения). Она использует класс QSettings из Qt для этой цели.
Функция saveTasks последовательно проходит по всем задачам, отображенным в списке tasksListWidget, извлекает из каждого элемента списка его заголовок, описание и дату (которые хранятся как текст и связанные данные), 
а затем сохраняет эту информацию в системное хранилище настроек с помощью QSettings, используя структурированные ключи (например, Tasks/task0/title, Tasks/task0/description и т.д.) для каждой задачи. Перед сохранением старые данные задач удаляются.

void MainWindow::saveTasks()
{
    // 1. Создание объекта QSettings
    // Создается объект settings для работы с настройками приложения.
    // "PersonalOrganizer" - это имя приложения или организации, которое помогает
    // QSettings определить, где хранить настройки (например, в реестре Windows
    // или в конфигурационном файле Linux/macOS).
    QSettings settings("PersonalOrganizer");

    // 2. Начало группы настроек
    // Начинается группа "Tasks". Это как создать папку "Tasks" внутри хранилища
    // настроек. Все последующие записи будут помещены внутрь этой группы,
    // что помогает организовать данные.
    settings.beginGroup("Tasks");

    // 3. Очистка предыдущих задач
    // settings.remove(""); внутри группы удаляет все ранее сохраненные ключи
    // в этой группе ("Tasks"). Это гарантирует, что мы сохраняем только
    // текущий список задач и не оставляем "мусора" от предыдущих сессий.
    // По сути, старый список задач стирается перед записью нового.
    settings.remove("");

    // 4. Цикл по элементам списка в интерфейсе
    // Запускается цикл, который проходит по каждому элементу (задаче),
    // отображаемому в виджете списка ui->tasksListWidget.
    // ui->tasksListWidget->count() возвращает количество элементов в списке.
    for (int i = 0; i < ui->tasksListWidget->count(); ++i) {
        // 5. Получение текущего элемента списка
        // Получаем указатель (item) на текущий элемент списка (QListWidgetItem)
        // с индексом i.
        QListWidgetItem *item = ui->tasksListWidget->item(i);

        // 6. Извлечение данных задачи из элемента списка:
        // 6.1. Заголовок (title)
        // item->text() возвращает видимый текст элемента списка - это заголовок задачи.
        QString title = item->text();
        // 6.2. Описание (description)
        // item->data(Qt::UserRole) извлекает пользовательские данные, связанные
        // с элементом списка под стандартной ролью Qt::UserRole.
        // .toString() преобразует эти данные в строку. Здесь хранится описание задачи.
        QString description = item->data(Qt::UserRole).toString();
        // 6.3. Дата (date)
        // item->data(Qt::UserRole + 1) извлекает другие пользовательские данные,
        // хранящиеся под следующей ролью (Qt::UserRole + 1).
        // .toDate() преобразует их в объект QDate (дата). Здесь хранится дата задачи.
        QDate date = item->data(Qt::UserRole + 1).toDate();

        // 7. Сохранение данных задачи в QSettings
        // Используя объект settings, сохраняем извлеченные данные.
        // Ключи создаются динамически для каждой задачи (task0, task1, ...):
        // QString("task%1/title").arg(i) создает ключ типа "Tasks/task0/title", "Tasks/task1/title", ...
        // (Префикс "Tasks/" добавляется автоматически из-за beginGroup).
        settings.setValue(QString("task%1/title").arg(i), title);
        settings.setValue(QString("task%1/description").arg(i), description);
        settings.setValue(QString("task%1/date").arg(i), date);
    }

    // 8. Завершение группы
    // Закрываем группу "Tasks". Последующие вызовы setValue будут записываться
    // вне этой группы (если не будет нового beginGroup).
    settings.endGroup();
}
=====================
Эта функция является "обратной" к saveTasks. Её задача — загрузить ранее сохраненные задачи из постоянного хранилища (QSettings) и отобразить их в виджете списка tasksListWidget при запуске приложения.
Функция loadTasks предназначена для загрузки сохраненных задач из QSettings. Она открывает группу "Tasks", пытается перебрать сохраненные ключи 
и для каждого (предположительно уникального) задания прочитать его заголовок, описание и дату. Затем она создает соответствующий элемент QListWidgetItem, устанавливает его текст и связанные данные (описание и дату) и добавляет этот элемент в виджет tasksListWidget.
void MainWindow::loadTasks()
{
    // 1. Создание объекта QSettings
    // Как и в saveTasks, создаем объект для доступа к настройкам
    // приложения "PersonalOrganizer".
    QSettings settings("PersonalOrganizer");

    // 2. Начало группы настроек
    // Открываем группу "Tasks", в которой были сохранены данные задач.
    settings.beginGroup("Tasks");

    // 3. Получение списка ключей
    // settings.childKeys() возвращает список всех ключей, находящихся
    // непосредственно внутри текущей группы ("Tasks").
    // На основе того, как работает saveTasks (сохраняя ключи вида "task0/title",
    // "task0/description", "task1/title" и т.д.), этот список будет содержать
    // все эти составные ключи: ["task0/title", "task0/description", "task0/date", "task1/title", ...].
    QStringList keys = settings.childKeys();

    // --- Важное замечание о потенциальной ошибке в коде ---
    // Логика внутри следующего цикла, скорее всего, содержит ошибку.
    // Она перебирает ключи вроде "task0/title" и затем пытается прочитать
    // значение по ключу key + "/title", что даст "task0/title/title",
    // в то время как данные хранятся просто по ключу "task0/title".
    // Код ниже не будет работать правильно с


Пояснение терминов
Виджет (Widget):
Это основной строительный блок графического пользовательского интерфейса (GUI) в Qt. Виджет - это видимый элемент, с которым пользователь может взаимодействовать или который он может видеть. 
Примеры: кнопка (QPushButton), текстовое поле (QLineEdit), метка (QLabel), список (QListWidget), само окно (QMainWindow, QDialog).
Виджеты могут содержать другие виджеты, образуя иерархию (например, окно содержит панель, а панель содержит кнопки).
Они получают события от системы (клики мыши, нажатия клавиш) и умеют отрисовывать себя на экране.
Базовым классом для всех виджетов является QWidget.

Форма (Form):
В контексте Qt Creator, "форма" - это визуальное представление окна или его части, которое вы создаете в режиме Дизайнера (Qt Designer). Обычно это файл с расширением .ui.
На форме вы размещаете и компонуете виджеты, чтобы создать внешний вид вашего приложения.

Qt Creator:
Это Интегрированная Среда Разработки (IDE - Integrated Development Environment), специально созданная для разработки приложений с использованием фреймворка Qt.
Она включает в себя редактор кода, отладчик, визуальный дизайнер интерфейсов (Qt Designer, где вы работаете с формами и виджетами), инструменты управления проектом и многое другое.

Сигнал (Signal):
Это сообщение, которое объект Qt (часто виджет) "испускает" (emits), когда происходит какое-то событие. Например, кнопка испускает сигнал clicked(), когда на нее нажимают; поле ввода испускает сигнал textChanged(const QString &), когда его текст изменяется.
Объект, испускающий сигнал, не знает и не заботится о том, кто (и есть ли кто-то вообще) этот сигнал примет. Он просто объявляет о произошедшем событии.
Сигналы объявляются в заголовочном файле (.h) внутри класса в секции signals:. Класс должен наследоваться от QObject и содержать макрос Q_OBJECT.

Слот (Slot):
Это специальная функция-член класса в Qt (класс должен наследоваться от QObject и иметь макрос Q_OBJECT), которая может быть подключена к сигналу.
Когда объект испускает сигнал, все слоты, подключенные к этому сигналу, автоматически вызываются.
Слоты - это обычные функции C++, но они должны быть объявлены в заголовочном файле (.h) в одной из секций: slots:, public slots:, protected slots: или private slots:.
Слоты служат обработчиками сигналов. В коде, который мы разбирали ранее (on_addTaskButton_clicked(), on_deleteTaskButton_clicked()), это как раз были слоты.

Сессия приложения (Session):
Это один запуск вашего приложения: с момента его старта до момента его закрытия.
"Сохранение данных между сессиями" означает, что данные, введенные или измененные во время одного запуска приложения, будут доступны и при следующем его запуске. Данные делаются постоянными (persistent).




На Linux системах QSettings по умолчанию использует текстовые конфигурационные файлы (обычно в формате, похожем на INI). Точное местоположение зависит от нескольких факторов:

Область видимости (Scope):

Пользовательская (QSettings::UserScope - используется по умолчанию): Настройки сохраняются отдельно для каждого пользователя.
Системная (QSettings::SystemScope): Настройки общие для всех пользователей системы (обычно требует прав администратора для записи).
Имя организации и приложения: Эти имена используются для создания пути к файлу. Вы задаете их при создании объекта QSettings или глобально через QCoreApplication::setOrganizationName() и QCoreApplication::setApplicationName().

В вашем коде вы используете:
QSettings settings("PersonalOrganizer");

Это конструктор, который принимает имя приложения ("PersonalOrganizer") и использует имя организации по умолчанию (если оно было задано ранее через QCoreApplication::setOrganizationName) или оставляет его пустым. Также по умолчанию используется QSettings::UserScope.

Стандартные местоположения для пользовательских настроек (UserScope) на Linux:

Файлы настроек обычно хранятся в домашней директории пользователя ($HOME), следуя спецификации XDG Base Directory:

Основной вариант (предпочтительный):

$HOME/.config/ИмяОрганизации/ИмяПриложения.conf
Если имя организации не задано или пустое:

$HOME/.config/ИмяПриложения.conf
Для вашего случая ("PersonalOrganizer"):

Наиболее вероятно: Если вы не задавали имя организации где-либо еще в коде (QCoreApplication::setOrganizationName(...)), файл будет находиться по пути:

$HOME/.config/PersonalOrganizer.conf
(где $HOME - это ваша домашняя директория, например /home/ваш_логин)
Если имя организации было задано (например, MyCompany), то путь будет:

$HOME/.config/MyCompany/PersonalOrganizer.conf
Другие возможные (менее стандартные или устаревшие) расположения:

Иногда настройки могут оказаться в $HOME/.local/share/ИмяОрганизации/ИмяПриложения.conf.
В очень старых приложениях или при нестандартной конфигурации могли использоваться пути вида $HOME/.ИмяПриложения/ИмяПриложения.conf или $HOME/.ИмяПриложения.conf.



void MainWindow::saveTasks()
{
    QSettings settings("PersonalOrganizer");
    settings.beginGroup("Tasks");
    settings.clear(); // Удаляем все старые задачи перед сохранением новых
    for (int i = 0; i < ui->tasksListWidget->count(); ++i) {
        QListWidgetItem *item = ui->tasksListWidget->item(i);
        QString title = item->text();
        QString description = item->data(Qt::User Role).toString();
        QDate date = item->data(Qt::User Role + 1).toDate();
        settings.setValue(QString("task%1/title").arg(i), title);
        settings.setValue(QString("task%1/description").arg(i), description);
        settings.setValue(QString("task%1/date").arg(i), date);
    }
    settings.endGroup();
}

void MainWindow::loadTasks()
{
    QSettings settings("PersonalOrganizer");
    settings.beginGroup("Tasks");
    QStringList keys = settings.childKeys();
    for (const QString &key : keys) {
        if (key.startsWith("task")) {
            QString title = settings.value(key).toString(); // Получаем title
            QString description = settings.value(key.replace("/title", "/description")).toString(); // Получаем description
            QDate date = settings.value(key.replace("/title", "/date")).toDate(); // Получаем date
            QListWidgetItem *newItem = new QListWidgetItem(title);
            newItem->setData(Qt::User Role, description);
            newItem->setData(Qt::User Role + 1, date);
            ui->tasksListWidget->addItem(newItem);
        }
    }
    settings.endGroup();
}

















void MainWindow::saveTasks()
{
    QSettings settings("PersonalOrganizer");
    settings.beginGroup("Tasks");
    settings.clear(); // Удаляем все старые задачи перед сохранением новых
    for (int i = 0; i < ui->tasksListWidget->count(); ++i) {
        QListWidgetItem *item = ui->tasksListWidget->item(i);
        QString title = item->text();
        QString description = item->data(Qt::User Role).toString();
        QDate date = item->data(Qt::User Role + 1).toDate();
        settings.setValue(QString("task%1/title").arg(i), title);
        settings.setValue(QString("task%1/description").arg(i), description);
        settings.setValue(QString("task%1/date").arg(i), date);
    }
    settings.endGroup();
}

void MainWindow::loadTasks()
{
    QSettings settings("PersonalOrganizer");
    settings.beginGroup("Tasks");
    QStringList keys = settings.childKeys();
    for (const QString &key : keys) {
        if (key.startsWith("task")) {
            QString title = settings.value(key).toString(); // Получаем title
            QString description = settings.value(key.replace("/title", "/description")).toString(); // Получаем description
            QDate date = settings.value(key.replace("/title", "/date")).toDate(); // Получаем date
            
            // Проверка на наличие значений
            if (!title.isEmpty() && !description.isEmpty() && date.isValid()) {
                QListWidgetItem *newItem = new QListWidgetItem(title);
                newItem->setData(Qt::User Role, description);
                newItem->setData(Qt::User Role + 1, date);
                ui->tasksListWidget->addItem(newItem);
            }
        }
    }
    settings.endGroup();
}



*/
