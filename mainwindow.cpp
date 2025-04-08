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
    settings.clear();
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
    int taskCount = 0;
    while (settings.contains(QString("task%1/title").arg(taskCount))) {
        QString title = settings.value(QString("task%1/title").arg(taskCount)).toString();
        QString description = settings.value(QString("task%1/description").arg(taskCount)).toString();
        QDate date = settings.value(QString("task%1/date").arg(taskCount)).toDate();

        if (!title.isEmpty() && !description.isEmpty() && date.isValid()) {
            QListWidgetItem *newItem = new QListWidgetItem(title);
            newItem->setData(Qt::UserRole, description);
            newItem->setData(Qt::UserRole + 1, date);
            ui->tasksListWidget->addItem(newItem);
        }
        taskCount++;
    }
    settings.endGroup();
}
